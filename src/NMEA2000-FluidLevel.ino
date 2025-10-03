// #define DEBUG_NMEA_MSG // Uncomment to see, what device will send to bus. Use e.g. OpenSkipper or Actisense NMEA Reader  
// #define DEBUG_NMEA_MSG_ASCII // If you want to use simple ascii monitor like Arduino Serial Monitor, uncomment this line
// #define DEBUG_NMEA_Actisense // Uncomment this, so you can test code without CAN bus chips on Arduino Mega
// #define DEBUG_MSG

// use the following Pins

#define ESP32_CAN_TX_PIN GPIO_NUM_5  // Set CAN TX port to D5 
#define ESP32_CAN_RX_PIN GPIO_NUM_4  // Set CAN RX port to D4

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
uint8_t targetDistanceInMm = 140; // Used for calibration

// Manufacturer's Software version code
char Version[] = VERSION_STR;

tN2kFluidType gFluidType = N2kft_GrayWater;

bool gSaveParams = false;

tN2kSyncScheduler FluidLevelScheduler(false, 2500, 500);
tN2kSyncScheduler MeasurementScheduler(true, 1000, 0);

RingBuf<uint16_t, TANK_RINGBUF_SIZE> gAverageTankFilled;

String gStatusSensor = "";


SFEVL53L1X VL53L1X;
Neotimer temperatureUpdate = Neotimer(30 * 60 * 1000); // Update temperature compensation every 30 minutes
Neotimer debouncer = Neotimer(100); // 100ms debounce time

void calibrateSensor(int referenceDistance = 140, bool automatic = false);

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

		// calibrateSensor(targetDistanceInMm, true); // Calibrate with default distance
         loadCalibration();
		 
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
        "VL53L1X level sensor",  // Manufacturer's Model ID
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
    NMEA2000.SetMode(tNMEA2000::N2km_NodeOnly, Config.Source());

    // Here we tell library, which PGNs we transmit
    NMEA2000.ExtendTransmitMessages(TransmitMessages);

    NMEA2000.SetOnOpen(OnN2kOpen);
    NMEA2000.Open();

    Serial.println("NMEA2000 started");

	Serial.println("Setup complete");
}

void resetCalibration() {
    Preferences prefs_;
    prefs_.begin("vl53cal", false);
    prefs_.clear();
    prefs_.end();
    Serial.println("Calibration reset to defaults.");
    
	loadCalibration();
}

void loadCalibration() {
    Preferences prefs_;
    prefs_.begin("vl53cal", true);
    int offset_ = prefs_.getInt("offset", 0);
    int xTalk_ = prefs_.getInt("xtalk", 0);
	int roiX_ = prefs_.getInt("roix", 16);
	int roiY_ = prefs_.getInt("roiy", 16);
	int roic_ = prefs_.getInt("roic", 163);
	int mode_ = prefs_.getInt("mode", 2); // 1=short, 2=long
	int sigma_ = prefs_.getInt("sigma", 50);
	int timing_ = prefs_.getInt("timing", 300);
    prefs_.end();

	if (offset_ < -500 || offset_ > 500) offset_ = 0;
	if (xTalk_ < 0 || xTalk_ > 1000) xTalk_ = 0;
	if (roiX_ < 4 || roiX_ > 16) roiX_ = 16;
	if (roiY_ < 4 || roiY_ > 16) roiY_ = 16;
	if (roic_ < 0 || roic_ > 255) roic_ = 163;
	if (mode_ != 1 && mode_ != 2) mode_ = 2;
	if (sigma_ > 200) sigma_ = 50;

    if (VL53L1X.isRanging()) {
        VL53L1X.stopRanging();
    }

    if (mode_ == 1) {
        VL53L1X.setDistanceModeShort();
	}
	else {
        VL53L1X.setDistanceModeLong();
	}

    VL53L1X.setOffset(offset_);
    VL53L1X.setXTalk(xTalk_);
    VL53L1X.setROI(roiX_, roiY_, roic_);
    VL53L1X.setTimingBudgetInMs(timing_);
	VL53L1X.setIntermeasurementPeriod(timing_ + 10); // intermeasurement must be > timing
    VL53L1X.setSigmaThreshold(sigma_);

    VL53L1X.startRanging();

    Serial.printf("Loaded offset: %dmm\n", offset_);
    Serial.printf("Loaded cross-talk: %dkcps\n", xTalk_);
	Serial.printf("Loaded ROI: %dx%d, center %d\n", roiX_, roiY_, roic_);
	Serial.printf("Loaded mode: %s\n", (mode_ == 1) ? "short" : "long");
    Serial.printf("Loaded sigma threshold: %d mm\n", sigma_);
	Serial.printf("Loaded timing budget: %d ms\n", timing_);

}

void calibrateSensor(int referenceDistance, bool automatic) {

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

    if (VL53L1X.isRanging()) {
        VL53L1X.stopRanging();
	}

    if (!automatic) {
        // Wait until the button is released (HIGH)
        while (digitalRead(GPIO_NUM_0) == LOW) {
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
    uint16_t sumDistance_ = 0;
    uint16_t Average_ = 0;

    for (uint8_t j_ = 0; j_ < gAverageTankFilled.size(); j_++) {
        sumDistance_ += gAverageTankFilled[j_];
    }

    if (gAverageTankFilled.size() > 0) {
        Average_ = sumDistance_ / gAverageTankFilled.size();
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

    if (gStatusSensor == "NOK") {
        WebSerial.printf("%s : Failed to detect and initialize VL53L1X!\n", getCurrentTime());
        return;
    }

    if (!VL53L1X.checkForDataReady()) {
        WebSerial.printf("%s : Sensor not ready\n", getCurrentTime());
        return;
    }

    uint16_t distanceEmpty_ = sensor.getDistanceEmpty();
    uint16_t distanceFull_ = sensor.getDistanceFull();

    if (distanceEmpty_ <= distanceFull_) {
        WebSerial.printf("%s : Invalid distance configuration: Empty distance (%dmm) must be greater than full distance (%dmm)\n",
            getCurrentTime(), distanceEmpty_, distanceFull_);
        return;
    }

    auto distance_ = VL53L1X.getDistance();
	auto status_ = VL53L1X.getRangeStatus();
    float signal_ = VL53L1X.getSignalRate();
    float signalPerSpad_ = VL53L1X.getSignalPerSpad();
    float ambientRate_ = VL53L1X.getAmbientRate();
	VL53L1X.clearInterrupt();

    WebSerial.printf("Distanz: %d mm | Status: %d | Signalrate: %.2f kcps | Signal/SPAD: %.2f kcps/SPAD | Ambientrate: %.2f kcps\n",
        distance_, status_, signal_, signalPerSpad_, ambientRate_);

    if (distance_ > distanceEmpty_) distance_ = distanceEmpty_;

    gAverageTankFilled.pushOverwrite(distance_);

    uint16_t averageDistance_ = GetAverageDistance();

    // Schnelle Änderung erkennen und Puffer resetten
    if (abs((int)distance_ - (int)averageDistance_) > 0.1 * tank.getHeight()) {
        for (int i = 0; i < TANK_RINGBUF_SIZE; ++i) {
            gAverageTankFilled.pushOverwrite(distance_);
        }
    }

}

double GetFillLevel() {
    uint16_t distanceEmpty_ = sensor.getDistanceEmpty();
    uint16_t distanceFull_ = sensor.getDistanceFull();
    uint16_t averageDistance_ = GetAverageDistance();

    double fillLevel_ = 0.0;

    if (averageDistance_ <= distanceFull_) {
        fillLevel_ = 100.0;
    }
    else if (averageDistance_ >= distanceEmpty_) {
        fillLevel_ = 0.0;
    }
    else {
        fillLevel_ = 100.0 * (distanceEmpty_ - averageDistance_) / (distanceEmpty_ - distanceFull_);
    }

    // Auf eine Nachkommastelle runden
    fillLevel_ = round(fillLevel_ * 10.0) / 10.0;

    return fillLevel_;
}

void SendN2kFluidLevel(void) {
    tN2kMsg N2kMsg_;

    if (FluidLevelScheduler.IsTime()) {
        FluidLevelScheduler.UpdateNextTime();

        SetN2kFluidLevel(N2kMsg_, Config.Instance(), tank.getFluidType(), GetFillLevel(), tank.getCapacity());
        NMEA2000.SendMsg(N2kMsg_);
    }
}

void loop() {
    SendN2kFluidLevel();
    NMEA2000.ParseMessages();
    wifiLoop();

    if (MeasurementScheduler.IsTime()) {
        MeasurementScheduler.UpdateNextTime();
        GetDistance();
    }

    if (NMEA2000.GetN2kSource() != Config.Source()) {
        Config.SetSource(NMEA2000.GetN2kSource());
        gSaveParams = true;
    }

    if (gParamsChanged) {
        gParamsChanged = false;
		NMEA2000.SetInstallationDescription1(iotWebConf.getThingName());
		loadCalibration();
	}

    if(temperatureUpdate.repeat()) {
        VL53L1X.startTemperatureUpdate();
		VL53L1X.startRanging();
        WebSerial.printf("%s : Temperature compensation updated\n", getCurrentTime());
	}

    // Dummy to empty input buffer to avoid board to stuck with e.g. NMEA Reader
    if (Serial.available()) {
        Serial.read();
    }
}