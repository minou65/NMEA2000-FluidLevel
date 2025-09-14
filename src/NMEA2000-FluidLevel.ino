// #define DEBUG_NMEA_MSG // Uncomment to see, what device will send to bus. Use e.g. OpenSkipper or Actisense NMEA Reader  
// #define DEBUG_NMEA_MSG_ASCII // If you want to use simple ascii monitor like Arduino Serial Monitor, uncomment this line
// #define DEBUG_NMEA_Actisense // Uncomment this, so you can test code without CAN bus chips on Arduino Mega
// #define DEBUG_MSG

// use the following Pins

#define ESP32_CAN_TX_PIN GPIO_NUM_5  // Set CAN TX port to D5 
#define ESP32_CAN_RX_PIN GPIO_NUM_4  // Set CAN RX port to D4

#include <esp_task_wdt.h>
#include <esp_mac.h>
#include <Wire.h>
#include <SparkFun_VL53L1X.h>
#include <N2kMessages.h>
#include <NMEA2000_CAN.h>
#include <Preferences.h>

#include "common.h"
#include "webhandling.h"
#include "version.h"
#include "neotimer.h"

bool debugMode = false;
uint8_t targetDistanceInMm = 100; // Used for calibration

// Manufacturer's Software version code
char Version[] = VERSION_STR;

// Configuration for the Watchdog Timer
esp_task_wdt_config_t wdt_config = {
    .timeout_ms = 5000, // Timeout in milliseconds
    .trigger_panic = true // Trigger panic if the Watchdog Timer expires
};

uint8_t gN2KSource = 22;
tN2kFluidType gFluidType = N2kft_GrayWater;

bool gSaveParams = false;

tN2kSyncScheduler FluidLevelScheduler(false, 2500, 500);
tN2kSyncScheduler MeasurementScheduler(true, 1000, 0);

RingBuf<uint16_t, 30> gAverageTankFilled;

uint16_t gTankCapacity = 150; // l
uint16_t gTankHeight = 1000; // mm
uint8_t gTankFilledPercent = 0; // %
float gSensorCalibrationFactor = 1.0;
uint8_t gDeadzoneUpper = 0; //mm
uint8_t gDeadzoneLower = 0; //mm

String gStatusSensor = "";


SFEVL53L1X VL53L1X;
Neotimer temperatureUpdate = Neotimer(30 * 60 * 1000); // Update temperature compensation every 30 minutes
Neotimer debouncer = Neotimer(100); // 100ms debounce time

// List here messages your device will transmit.
const unsigned long TransmitMessages[] PROGMEM = {
    127505L,
    0
};

void OnN2kOpen() {
    // Start schedulers now.
    FluidLevelScheduler.UpdateNextTime();
    MeasurementScheduler.UpdateNextTime();
}

void setup() {
    uint8_t chipid[6];
    uint32_t id = 0;
    int i = 0;

    // Generate unique number from chip id
    esp_efuse_mac_get_default(chipid);
    for (i = 0; i < 6; i++) id += (chipid[i] << (7 * i));

    Serial.begin(115200);
    while (!Serial) {
        delay(1);
    }

    Serial.printf("Firmware version:%s\n", Version);

    // init wifi
    wifiInit();

    Wire.begin();

    if (VL53L1X.begin() == 0) { //Begin returns 0 on a good init 
         Serial.println("VL53L1X online!");
         VL53L1X.setTimingBudgetInMs(800);         // Maximale Genauigkeit
         VL53L1X.setDistanceModeLong();            // Long Mode für mehr Präzision
		 VL53L1X.setIntermeasurementPeriod(1000); // Messung alle 1000ms, musste >= TimingBudget
		 loadCalibration(VL53L1X);                 // Load calibration data from NVS
		 VL53L1X.startTemperatureUpdate();
         VL53L1X.startRanging();

		 temperatureUpdate.start(); // Start timer for temperature compensation

		 gStatusSensor = "Ok";
     } else {
        Serial.println(F("Failed to detect and initialize VL53L1X!"));
		gStatusSensor = "NOK";
    }

	debouncer.start();

    // Reserve enough buffer for sending all messages. This does not work on small memory devices like Uno or Mega
    NMEA2000.SetN2kCANMsgBufSize(8);
    NMEA2000.SetN2kCANReceiveFrameBufSize(150);
    NMEA2000.SetN2kCANSendFrameBufSize(150);

    // Set Product information
    NMEA2000.SetProductInformation(
        "1", // Manufacturer's Model serial code
        100, // Manufacturer's product code
        "VL53L0X level sensor",  // Manufacturer's Model ID
        Version,  // Manufacturer's Software version code
        Version, // Manufacturer's Model version
        1, // Load Equivalency
        0xffff, // NMEA version
        0xff  // Certification level
    );

    // Set device information
    NMEA2000.SetDeviceInformation(
        id, // Unique number. Use e.g. Serial number.
        150, // Device function=Fluid. See codes on https://web.archive.org/web/20190531120557/https://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
        75, // Device class=Sensor Communication Interface. See codes on https://web.archive.org/web/20190531120557/https://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
        2040 // Just choosen free from code list on https://web.archive.org/web/20190529161431/http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf
    );

    NMEA2000.SetConfigurationInformation(
        String(id).c_str(), // Unique number
        iotWebConf.getThingName(),
        ""
	);

    NMEA2000.EnableForward(false); // Disable all msg forwarding to USB (=Serial)

    // If you also want to see all traffic on the bus use N2km_ListenAndNode instead of N2km_NodeOnly below
    NMEA2000.SetMode(tNMEA2000::N2km_NodeOnly, gN2KSource);

    // Here we tell library, which PGNs we transmit
    NMEA2000.ExtendTransmitMessages(TransmitMessages);

    NMEA2000.SetOnOpen(OnN2kOpen);
    NMEA2000.Open();

    Serial.println("NMEA2000 started");

    // Initialize the Watchdog Timer
    esp_task_wdt_init(&wdt_config);
    esp_task_wdt_add(NULL); //add current thread to WDT watch

    pinMode(GPIO_NUM_0, INPUT_PULLUP);

	Serial.println("Setup complete");
}

void loadCalibration(SFEVL53L1X& sensor) {
    Preferences prefs_;
    prefs_.begin("vl53cal", true);
    int offset_ = prefs_.getInt("offset", 0);
    int xTalk_ = prefs_.getInt("xtalk", 0);
	int roiX_ = prefs_.getInt("roix", 16);
	int roiY_ = prefs_.getInt("roiy", 16);
	int roic_ = prefs_.getInt("roic", 199);
    prefs_.end();

	if (offset_ < -500 || offset_ > 500) offset_ = 0;
	if (xTalk_ < 0 || xTalk_ > 1000) xTalk_ = 0;
	if (roiX_ < 4 || roiX_ > 16) roiX_ = 16;
	if (roiY_ < 4 || roiY_ > 16) roiY_ = 16;
	if (roic_ < 0 || roic_ > 255) roic_ = 199;

    sensor.setOffset(offset_);
    sensor.setXTalk(xTalk_);
	sensor.setROI(roiX_, roiY_, roic_);

    Serial.print("Loaded offset: ");
    Serial.print(offset_);
    Serial.println(" mm");

    Serial.print("Loaded cross-talk: ");
    Serial.print(xTalk_);
    Serial.println(" kcps");

	Serial.printf("Loaded ROI: %dx%d, center %d\n", roiX_, roiY_, roic_);
}

void calibrateSensor(int referenceDistance = 140, bool automatic = false) {

    const int referenceDistance_ = referenceDistance; // mm
    const int triggerThreshold_ = 100;  // mm
    const int triggerCount_ = 20;       // ~1 second at 50ms timing budget
    Preferences prefs_;

    if (gStatusSensor == "NOK") {
        Serial.println("Failed to detect and calibrate VL53L1X!");
        if (automatic) {
            WebSerial.println("Failed to detect and calibrate VL53L1X!");
        }
        return;
    }

    Serial.println();
    Serial.println("*****************************************************************************************************");
    Serial.println("                                    Offset calibration");
    Serial.println("Place a light grey (17% gray) target at a distance of 140mm in front of the VL53L1X sensor.");
    Serial.println("The calibration will start 5 seconds after a distance below 10 cm was detected for 1 second.");
    Serial.println("Use the resulting offset distance as parameter for the setOffset() function called after begin().");
    if (!automatic) {
        Serial.println("If you are ready, press the button connected to GPIO0 (GND when pressed) to start the calibration.");
        Serial.println("If you want to cancel the calibration, reboot the device.");
    }
    else {
        Serial.println("Automatic calibration mode: starting immediately.");
    }
    Serial.println("*****************************************************************************************************");
    Serial.println();

    if (!automatic) {
        // Wait until the button is released (HIGH)
        while (digitalRead(GPIO_NUM_0) == LOW) {
            esp_task_wdt_reset();
            delay(10);
        }
        // Wait for a new button press (LOW), debounced
        bool lastState_ = HIGH;
        unsigned long lastChange_ = millis();
        while (true) {
            bool state_ = digitalRead(GPIO_NUM_0);
            if (state_ != lastState_) {
                lastChange_ = millis();
                lastState_ = state_;
            }
            if (state_ == LOW && (millis() - lastChange_) > 50) { // 50ms debounce time
                break;
            }
            esp_task_wdt_reset();
            delay(5);
        }

        int tLowDistanceCount_ = 0;
        while (tLowDistanceCount_ < triggerCount_) {
            while (!VL53L1X.checkForDataReady()) delay(1);
            int dist_ = VL53L1X.getDistance();
            if (dist_ < triggerThreshold_) {
                tLowDistanceCount_++;
            }
            else {
                tLowDistanceCount_ = 0;
            }
            VL53L1X.clearInterrupt();
        }

        Serial.println("Target detected. Starting calibration in 5 seconds...");
        delay(5000);
    }

    // Offset Calibration
    Serial.println("Starting offset calibration...");
    VL53L1X.calibrateOffset(referenceDistance_);
    int offset_ = VL53L1X.getOffset();
    Serial.print("Offset value determined: ");
    Serial.print(offset_);
    Serial.println(" mm");

    prefs_.begin("vl53cal", false);
    prefs_.putInt("offset", offset_);
    prefs_.end();
    Serial.println("Offset value saved to flash.");

    // Cross-Talk Calibration
    Serial.println("Starting cross-talk calibration...");
    VL53L1X.calibrateXTalk(referenceDistance_);
    int xTalk_ = VL53L1X.getXTalk();
    Serial.print("Cross-talk value determined: ");
    Serial.print(xTalk_);
    Serial.println(" kcps");

    prefs_.begin("vl53cal", false);
    prefs_.putInt("xtalk", xTalk_);
    prefs_.end();
    Serial.println("Cross-talk value saved to flash.");

    if (automatic) {
        WebSerial.println("Calibration complete.");
        WebSerial.printf("Offset: %d mm\n", offset_);
        WebSerial.printf("Cross-talk: %d kcps\n", xTalk_);
    }

    Serial.println("Calibration complete.");

    gParamsChanged = true;
}

uint16_t GetAverageDistance() {
    uint16_t SumTankFilled_ = 0;
    uint16_t Average_ = 0;

    for (uint8_t j_ = 0; j_ < gAverageTankFilled.size(); j_++) {
        SumTankFilled_ += gAverageTankFilled[j_];
    }

    if (gAverageTankFilled.size() > 0) {
        Average_ = SumTankFilled_ / gAverageTankFilled.size();
    }

    return Average_;
}

String getCurrentTime() {
    time_t now_ = time(nullptr);
    struct tm* timeInfo_ = localtime(&now_);

    char buffer_[9]; // Puffer für "H:m:s"
    strftime(buffer_, sizeof(buffer_), "%H:%M:%S", timeInfo_);

    return String(buffer_);
}

void GetDistance() {

    if (MeasurementScheduler.IsTime()) {
        MeasurementScheduler.UpdateNextTime();

        if (gStatusSensor == "NOK") {
            WebSerial.printf("%s : Failed to detect and initialize VL53L1X!\n", getCurrentTime());
            return;
        }

        if (!VL53L1X.checkForDataReady()) {
            WebSerial.printf("%s : Sensor not ready\n", getCurrentTime());
            return;
		}

		uint32_t distance_ = VL53L1X.getDistance();

        if (distance_ < gDeadzoneUpper) {
            distance_ = 0;
        }

        if (distance_ > gTankHeight - gDeadzoneLower) {
            distance_ = gTankHeight;
        }

        gAverageTankFilled.pushOverwrite(gTankHeight - distance_);

        uint16_t Tankfilled_ = GetAverageDistance();
        if (Tankfilled_ != 0) {
            gTankFilledPercent = 100 * Tankfilled_ / gTankHeight; // %
        }
        else {
            gTankFilledPercent = 0;
        }

        WebSerial.printf("%s : Distance: %dmm. Calibration factor: %f\n", getCurrentTime(), distance_, gSensorCalibrationFactor);

        DEBUG_PRINTF("Height: %dmm\n", gTankHeight);
        DEBUG_PRINTF("Distance: %dmm\n", distance_);
        DEBUG_PRINTF("Filled: %dmm (%d%%)\n", gTankHeight - distance_, gTankFilledPercent);
        DEBUG_PRINTF("calibration factor: %f\n", gSensorCalibrationFactor);
        DEBUG_PRINTLN("");
    }
}

void SendN2kFluidLevel(void) {
    tN2kMsg N2kMsg_;

    if (FluidLevelScheduler.IsTime()) {
        FluidLevelScheduler.UpdateNextTime();

        SetN2kFluidLevel(N2kMsg_, Config.Instance(), gFluidType, gTankFilledPercent, gTankCapacity);
        NMEA2000.SendMsg(N2kMsg_);
    }
}



void loop() {
    esp_task_wdt_reset();

    SendN2kFluidLevel();
    NMEA2000.ParseMessages();
    wifiLoop();

    if (NMEA2000.GetN2kSource() != gN2KSource) {
        gN2KSource = NMEA2000.GetN2kSource();
        gSaveParams = true;
    }

    if (gParamsChanged) {
        gParamsChanged = false;
		NMEA2000.SetInstallationDescription1(iotWebConf.getThingName());
		loadCalibration(VL53L1X);
	}

    GetDistance();

    if(temperatureUpdate.repeat()) {
        VL53L1X.startTemperatureUpdate();
        WebSerial.printf("%s : Temperature compensation updated\n", getCurrentTime());
	}

    // Dummy to empty input buffer to avoid board to stuck with e.g. NMEA Reader
    if (Serial.available()) {
        Serial.read();
    }

    static bool lastDebouncedState_ = false;
    debouncer.debounce(digitalRead(GPIO_NUM_0) == LOW);
    bool currentDebouncedState_ = debouncer.getDebouncedState();

    if (currentDebouncedState_ && !lastDebouncedState_) {
        calibrateSensor();
    }
    lastDebouncedState_ = currentDebouncedState_;
}