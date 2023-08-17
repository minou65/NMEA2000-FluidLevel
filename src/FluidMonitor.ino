

// use the following Pins

#define ESP32_CAN_TX_PIN GPIO_NUM_5  // Set CAN TX port to D5 
#define ESP32_CAN_RX_PIN GPIO_NUM_4  // Set CAN RX port to D4

#include <DFRobot_VL6180X.h>
#include <N2kMessages.h>
#include <NMEA2000_CAN.h>

#include "common.h"
#include "webhandling.h"

// Set time offsets
#define SlowDataUpdatePeriod 1000  // Time between CAN Messages sent
#define DistanceUpdatePeriod 5000  
#define TempSendOffset 0

uint8_t gN2KSource = 22;

uint16_t gFluidCapacity = 150;
double gFluidLevel = 100;
tN2kFluidType gFluidType = N2kft_GrayWater;

String gStatusSensor = "";

uint16_t gTankHeight = 1000;
uint16_t gTankfilled = 0;
uint8_t gTankPercentFilled = 0;
RingBuf<uint16_t, 30> gAverageTankFilled;


char Version[] = "0.0.0.1 (2023-07-25)"; // Manufacturer's Software version code

DFRobot_VL6180X Sensor;

// Task handle (Core 0 on ESP32)
TaskHandle_t TaskHandle;

// List here messages your device will transmit.
const unsigned long TransmitMessages[] PROGMEM = {
    127505L,
    0
};

void setup() {
    uint8_t chipid[6];
    uint32_t id = 0;
    int i = 0;

    // Generate unique number from chip id
    esp_efuse_mac_get_default(chipid);
    for (i = 0; i < 6; i++) id += (chipid[i] << (7 * i));

    Serial.begin(115200);

    // wait for serial port to open on native usb devices
    while (!Serial) {
        delay(1);
    }

    // init wifi
    wifiInit();

    if (!(Sensor.begin())) {
        gStatusSensor = "Please check that the IIC device is properly connected!";

    }
    else {
        gStatusSensor = "IIC device found!";
    }

    Serial.println(gStatusSensor);

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
        "FluidMonitor",  // Manufacturer's Model ID
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


    NMEA2000.Open();

}

bool IsTimeToUpdate(unsigned long NextUpdate) {
    return (NextUpdate < millis());
}

unsigned long InitNextUpdate(unsigned long Period, unsigned long Offset = 0) {
    return millis() + Period + Offset;
}

void SetNextUpdate(unsigned long& NextUpdate, unsigned long Period) {
    while (NextUpdate < millis()) NextUpdate += Period;
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

    static unsigned long DistanceUpdateTime = InitNextUpdate(DistanceUpdatePeriod, 0);

    if (IsTimeToUpdate(DistanceUpdateTime)) {
        SetNextUpdate(DistanceUpdateTime, DistanceUpdatePeriod);

        float lux = Sensor.alsPoLLMeasurement();
        String str = "ALS: " + String(lux) + " lux";
        Serial.println(str);
        uint8_t range = Sensor.rangePollMeasurement();
        uint8_t status = Sensor.getRangeResult();

        switch (status) {
        case VL6180X_NO_ERR:
            gStatusSensor = "Ok";
            Serial.print(F("Range: "));
            Serial.print(String(range));
            Serial.println(F(" mm"));

            // gTankfilled = gTankHeight - range;
            gAverageTankFilled.lockedPush(gTankHeight - range);
            
            gTankfilled = GetAverageDistance();
            if (gTankfilled != 0) {
                gTankPercentFilled = 100 * gTankfilled / gTankHeight; // %
            }
            else {
                gTankPercentFilled = 0;
            }
            

            break;
        case VL6180X_EARLY_CONV_ERR:
            gStatusSensor = "RANGE ERR: ECE check failed !";
            gTankPercentFilled = 99;
            break;
        case VL6180X_MAX_CONV_ERR:
            gStatusSensor = "RANGE ERR: System did not converge before the specified max!";
            gTankPercentFilled = 99;
            break;
        case VL6180X_IGNORE_ERR:
            gStatusSensor = "RANGE ERR: Ignore threshold check failed !";
            gTankPercentFilled = 99;
            break;
        case VL6180X_MAX_S_N_ERR:
            gStatusSensor = "RANGE ERR: Measurement invalidated!";
            gTankPercentFilled = 99;
            break;
        case VL6180X_RAW_Range_UNDERFLOW_ERR:
            gStatusSensor = "RANGE ERR: RESULT_RANGE_RAW < 0!";
            gTankPercentFilled = 100;
            break;
        case VL6180X_RAW_Range_OVERFLOW_ERR:
            gStatusSensor = "RESULT_RANGE_RAW is out of range !";
            gTankPercentFilled = 0;
            break;
        case VL6180X_Range_UNDERFLOW_ERR:
            gStatusSensor = "RANGE ERR: RESULT__RANGE_VAL < 0 !";
            gTankPercentFilled = 100;
            break;
        case VL6180X_Range_OVERFLOW_ERR:
            gStatusSensor = "RANGE ERR: RESULT__RANGE_VAL is out of range !";
            gTankPercentFilled = 0;
            break;
        default:
            gStatusSensor = "RANGE ERR: Systerm err!";
            gTankPercentFilled = 99;
            break;
        }

        Serial.println(gStatusSensor);
    }
}

void SendN2kFluidLevel(void) {

    tN2kMsg N2kMsg;
    static unsigned long SlowDataUpdated = InitNextUpdate(SlowDataUpdatePeriod, TempSendOffset);

    if (IsTimeToUpdate(SlowDataUpdated)) {
        SetNextUpdate(SlowDataUpdated, SlowDataUpdatePeriod);

        SetN2kFluidLevel(N2kMsg, 255, gFluidType, gFluidLevel, gFluidCapacity);
        NMEA2000.SendMsg(N2kMsg);

    }
}

void loop() {
    SendN2kFluidLevel();
    NMEA2000.ParseMessages();

    gParamsChanged = false;

    // Dummy to empty input buffer to avoid board to stuck with e.g. NMEA Reader
    if (Serial.available()) {
        Serial.read();
    }
}

void loop2(void* parameter) {
    for (;;) {   // Endless loop
        wifiLoop();
        GetDistance();
        vTaskDelay(100);
    }
}