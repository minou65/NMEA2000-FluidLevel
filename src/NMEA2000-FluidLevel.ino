// #define DEBUG_NMEA_MSG // Uncomment to see, what device will send to bus. Use e.g. OpenSkipper or Actisense NMEA Reader  
// #define DEBUG_NMEA_MSG_ASCII // If you want to use simple ascii monitor like Arduino Serial Monitor, uncomment this line
// #define DEBUG_NMEA_Actisense // Uncomment this, so you can test code without CAN bus chips on Arduino Mega
// #define DEBUG_MSG

// use the following Pins

#define ESP32_CAN_TX_PIN GPIO_NUM_5  // Set CAN TX port to D5 
#define ESP32_CAN_RX_PIN GPIO_NUM_4  // Set CAN RX port to D4

#include <Wire.h>
#include <VL53L0X.h>
#include <N2kMessages.h>
#include <NMEA2000_CAN.h>

#include "common.h"
#include "webhandling.h"

char Version[] = "1.0.0.3 (2024-04-28)"; // Manufacturer's Software version code

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


VL53L0X sensor;

// Uncomment this line to use long range mode. This
// increases the sensitivity of the sensor and extends its
// potential range, but increases the likelihood of getting
// an inaccurate reading because of reflections from objects
// other than the intended target. It works best in dark
// conditions.
//#define LONG_RANGE

#define HIGH_SPEED 20000
#define HIGH_ACCURACY 200000

// Task handle (Core 0 on ESP32)
TaskHandle_t TaskHandle;

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

#ifdef DEBUG_MSG
    Serial.begin(115200);

    // wait for serial port to open on native usb devices
    while (!Serial) {
        delay(1);
    }
#endif // DEBUG_MSG

    // init wifi
    wifiInit();

    Wire.begin();

    sensor.setTimeout(500);
    if (sensor.init()) {
        Serial.println("Detected and initialized sensor");

        sensor.setTimeout(1000);

#if defined LONG_RANGE
        // lower the return signal rate limit (default is 0.25 MCPS)
        sensor.setSignalRateLimit(0.1);
        // increase laser pulse periods (defaults are 14 and 10 PCLKs)
        sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
        sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
#endif

        sensor.setMeasurementTimingBudget(HIGH_ACCURACY);
    }
    else {
        Serial.println("Failed to detect and initialize sensor!");
    }



    xTaskCreatePinnedToCore(
        loop2, /* Function to implement the task */
        "TaskHandle", /* Name of the task */
        10000,  /* Stack size in words */
        NULL,  /* Task input parameter */
        0,  /* Priority of the task */
        &TaskHandle,  /* Task handle. */
        0 /* Core where the task should run */
    );

    // Reserve enough buffer for sending all messages. This does not work on small memory devices like Uno or Mega
    NMEA2000.SetN2kCANMsgBufSize(8);
    NMEA2000.SetN2kCANReceiveFrameBufSize(150);
    NMEA2000.SetN2kCANSendFrameBufSize(150);

    // Set Product information
    NMEA2000.SetProductInformation(
        "1", // Manufacturer's Model serial code
        100, // Manufacturer's product code
        "FluidLevel",  // Manufacturer's Model ID
        Version,  // Manufacturer's Software version code
        Version // Manufacturer's Model version
    );

    // Set device information
    NMEA2000.SetDeviceInformation(
        id, // Unique number. Use e.g. Serial number.
        150, // Device function=Fluid. See codes on https://web.archive.org/web/20190531120557/https://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
        75, // Device class=Sensor Communication Interface. See codes on https://web.archive.org/web/20190531120557/https://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
        2040 // Just choosen free from code list on https://web.archive.org/web/20190529161431/http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf
    );

#ifdef DEBUG_NMEA_MSG
    Serial.begin(115200);
    NMEA2000.SetForwardStream(&Serial);

#ifdef DEBUG_NMEA_MSG_ASCII
    NMEA2000.SetForwardType(tNMEA2000::fwdt_Text)
#endif // DEBUG_NMEA_MSG_ASCII

#ifdef  DEBUG_NMEA_Actisense
        NMEA2000.SetDebugMode(tNMEA2000::dm_Actisense);
#endif //  DEBUG_NMEA_Actisense

#else
    NMEA2000.EnableForward(false); // Disable all msg forwarding to USB (=Serial)

#endif // DEBUG_NMEA_MSG


    // If you also want to see all traffic on the bus use N2km_ListenAndNode instead of N2km_NodeOnly below
    NMEA2000.SetMode(tNMEA2000::N2km_NodeOnly, gN2KSource);

    // Here we tell library, which PGNs we transmit
    NMEA2000.ExtendTransmitMessages(TransmitMessages);

    NMEA2000.SetOnOpen(OnN2kOpen);
    NMEA2000.Open();

    Serial.println("NMEA2000 started");

}

uint16_t GetAverageDistance() {
    uint16_t SumTankFilled = 0;
    uint16_t Average = 0;

    for (uint8_t j = 0; j < gAverageTankFilled.size(); j++) {
        SumTankFilled += gAverageTankFilled[j];
    }

    if (gAverageTankFilled.size() > 0) {
        Average = SumTankFilled / gAverageTankFilled.size();
    }

    return Average;
}

void GetDistance() {

    if(MeasurementScheduler.IsTime()){
        MeasurementScheduler.UpdateNextTime();

        uint32_t range = uint32_t(sensor.readRangeSingleMillimeters() * gSensorCalibrationFactor);

        if (!(sensor.timeoutOccurred())) {
            gStatusSensor = "Ok";

            if (range < gDeadzoneUpper) {
                range = 0;
            }

            if (range > gTankHeight - gDeadzoneLower) {
                range = gTankHeight;
            }

            gAverageTankFilled.pushOverwrite(gTankHeight - range);

            uint16_t Tankfilled = GetAverageDistance();
            if (Tankfilled != 0) {
                gTankFilledPercent = 100 * Tankfilled / gTankHeight; // %
            }
            else {
                gTankFilledPercent = 0;
            }

#ifdef DEBUG_MSG
            Serial.print(F("Height: "));
            Serial.print(String(gTankHeight));
            Serial.println(F("mm"));
            Serial.print(F("Distance: "));
            Serial.print(String(sensor.readRangeSingleMillimeters()));
            Serial.println(F("mm"));
            Serial.print(F("Filled: "));
            Serial.print(String(gTankHeight - range));
            Serial.print(F("mm ("));
            Serial.print(gTankFilledPercent);
            Serial.println(F("%)"));
            Serial.print(F("calibration factor: "));
            Serial.println(gSensorCalibrationFactor, 5);
            Serial.println(F(""));
#endif

        }
        else {
            gStatusSensor = "Timeout";

#ifdef DEBUG_MSG
            Serial.println(gStatusSensor);
#endif // DEBUG_MSG

        }

    }
}

void SendN2kFluidLevel(void) {
    tN2kMsg N2kMsg;

    if (FluidLevelScheduler.IsTime()) {
        FluidLevelScheduler.UpdateNextTime();

        SetN2kFluidLevel(N2kMsg, Config.Instance(), gFluidType, gTankFilledPercent, gTankCapacity);
        NMEA2000.SendMsg(N2kMsg);
    }
}

void loop() {
    SendN2kFluidLevel();
    NMEA2000.ParseMessages();
    wifiLoop();

    if (NMEA2000.GetN2kSource() != gN2KSource) {
        gN2KSource = NMEA2000.GetN2kSource();
        gSaveParams = true;
    }

    gParamsChanged = false;

    // Dummy to empty input buffer to avoid board to stuck with e.g. NMEA Reader
    if (Serial.available()) {
        Serial.read();
    }
}

void loop2(void* parameter) {
    for (;;) {   // Endless loop
        GetDistance();
        vTaskDelay(100);
    }
}