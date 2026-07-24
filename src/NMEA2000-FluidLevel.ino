// ============================================================================
// NMEA2000 Fluid Level Sensor - Resistive Sensor (240-33 Ohm)
// ============================================================================
// Sensor: 240-33 Ohm resistive level sensor
// - 33 Ohm  = Tank FULL (100%)
// - 240 Ohm = Tank EMPTY (0%)
// - Open circuit (infinite resistance) = Tank EMPTY
// - Short circuit (0 Ohm) = Tank EMPTY
// ============================================================================;

// ============================================================================
// BOARD CONFIGURATION
// ============================================================================
// Select your target board by uncommenting ONE of the following definitions:
// - Uncomment the board you are using
// - Leave all others commented out
// ============================================================================

#define BOARD_WEMOS_D1_MINI_ESP32       // Wemos D1 Mini ESP32
// #define BOARD_NODE32S                // NodeMCU-32S
// #define BOARD_ESP32_DEVKIT           // ESP32 DevKit V1
// #define BOARD_CUSTOM                 // Custom board (define pins below)


#if defined(BOARD_WEMOS_D1_MINI_ESP32)
    // ----------------------------------------------------
    // Wemos D1 Mini ESP32 Pin Configuration
    // ----------------------------------------------------
#define ESP32_CAN_TX_PIN GPIO_NUM_5     // CAN Bus TX Pin
#define ESP32_CAN_RX_PIN GPIO_NUM_4     // CAN Bus RX Pin
#define SENSOR_PIN       GPIO_NUM_36    // ADC pin for resistive sensor (VP, ADC1_CH0)

#define BOARD_NAME "Wemos D1 Mini ESP32"
#define BOARD_INFO "Standard Wemos D1 Mini ESP32 configuration"

#define DISABLE_BROWNOUT_DETECTOR

#elif defined(BOARD_NODE32S)
    // ----------------------------------------------------
    // NodeMCU-32S Pin Configuration
    // ----------------------------------------------------
#define ESP32_CAN_TX_PIN GPIO_NUM_5     // CAN Bus TX Pin
#define ESP32_CAN_RX_PIN GPIO_NUM_4     // CAN Bus RX Pin
#define SENSOR_PIN       GPIO_NUM_22    // ADC pin for resistive sensor (VP, ADC1_CH0)

#define BOARD_NAME "NodeMCU-32S"
#define BOARD_INFO "NodeMCU-32S standard configuration"
#elif defined(BOARD_CUSTOM)
    // ----------------------------------------------------
    // Custom Board Pin Configuration
    // ----------------------------------------------------
    // Define your custom pin assignments here
#define ESP32_CAN_TX_PIN GPIO_NUM_17    // CAN Bus TX Pin (customize)
#define ESP32_CAN_RX_PIN GPIO_NUM_16    // CAN Bus RX Pin (customize)
#define ONE_WIRE_BUS     GPIO_NUM_25    // DS18B20 OneWire Data Pin (customize)

#define BOARD_NAME "Custom ESP32 Board"
#define BOARD_INFO "User-defined custom pin configuration"

#else
    // ----------------------------------------------------
    // No Board Selected - Compilation Error
    // ----------------------------------------------------
#error "ERROR: No valid board configuration selected! Please uncomment ONE board definition at the top of this file."
#error "Available options: BOARD_WEMOS_D1_MINI_ESP32, BOARD_NODE32S, BOARD_ESP32_DEVKIT, BOARD_CUSTOM"

#endif

// #define DEBUG_NMEA_MSG_ASCII // If you want to use simple ascii monitor like Arduino Serial Monitor, uncomment this line
// #define DEBUG_NMEA_Actisense // Uncomment this, so you can test code without CAN bus chips on Arduino Mega
// #define DEBUG_MSG

// ============================================================================
// RESISTIVE SENSOR CONFIGURATION
// ============================================================================

#define REFERENCE_RESISTOR   270.0f   // Reference resistor in voltage divider (Ohm)
#define ADC_VOLTAGE           3.3f    // ADC reference voltage
#define ADC_RESOLUTION       4095.0f  // 12-bit ADC resolution
#define ADC_SAMPLES            10     // Number of ADC samples for averaging

// Resistance thresholds for fault detection
#define RESISTANCE_MIN         10.0f  // Below this = short circuit (fault)
#define RESISTANCE_MAX        500.0f  // Above this = open circuit (fault)

// ============================================================================
// INCLUDES
// ============================================================================
#include <Arduino.h>
#include <esp_mac.h>
#include <N2kMessages.h>
#include <NMEA2000_CAN.h>
#include <Preferences.h>
#include <esp_task_wdt.h>
#include <RebootManager.h>
#include <time.h>
#include <sys/time.h>
#include <driver/twai.h>

#include "common.h"
#include "webhandling.h"
#include "version.h"

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================
bool debugMode = false;
char Version[] = VERSION_STR;
bool ParamsSave = false;

tN2kSyncScheduler FluidLevelScheduler(false, 2500, 500);
tN2kSyncScheduler MeasurementScheduler(true, 1000, 0);

RingBuf<float, TANK_RINGBUF_SIZE> gAverageFillLevel;
String SensorStatus = "";

bool TimeSet = false;
unsigned long LastTimeUpdate = 0;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================
float readResistance();
float calculateFillLevel(float resistance);
float getAverageFillLevel();
void initSensor();
String getCurrentTime();
void handleN2kMessages(const tN2kMsg& N2kMsg);
void ParseN2kSystemTime(const tN2kMsg& N2kMsg);
void ParseN2kGNSS(const tN2kMsg& N2kMsg);

// NMEA2000 message handlers
typedef struct {
    unsigned long PGN;
    void (*Handler)(const tN2kMsg& N2kMsg);
} tNMEA2000Handler;

tNMEA2000Handler NMEA2000Handlers[] = {
    {126992L, ParseN2kSystemTime},  // System Time
    {129029L, ParseN2kGNSS},        // GNSS Position Data
    {0, 0}
};

// List of messages to receive
const unsigned long ReceiveMessages[] PROGMEM = {
    126992L,  // System Time
    129029L,  // GNSS Position Data
    0
};

// List here messages your device will transmit.
const unsigned long TransmitMessages[] PROGMEM = {
    127505L,  // Fluid Level
    0
};

// ============================================================================
// TIME SYNCHRONIZATION FUNCTIONS
// ============================================================================

/**
 * Set ESP32 system time from NMEA2000 time
 */
void SetSystemTime(uint16_t DaysSince1970, double SecondsSinceMidnight) {
    if (DaysSince1970 == 0xFFFF || SecondsSinceMidnight > 86400) {
        return;
    }
    
    // Update only every 10 minutes (except first time)
    if (TimeSet && (millis() - LastTimeUpdate < 600000)) {
        return;
    }
    
    time_t timestamp = (DaysSince1970 * 86400UL) + (time_t)SecondsSinceMidnight;
    
    struct timeval tv;
    tv.tv_sec = timestamp;
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);
    
    TimeSet = true;
    LastTimeUpdate = millis();
    
    char timeStr[64];
    struct tm* timeinfo = localtime(&timestamp);
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);
    Serial.printf("System time set from NMEA2000: %s UTC\n", timeStr);
}

/**
 * Parse PGN 126992 - System Time
 */
void ParseN2kSystemTime(const tN2kMsg& N2kMsg) {
    unsigned char SID;
    uint16_t SystemDate;
    double SystemTime;
    tN2kTimeSource TimeSource;
    
    if (ParseN2kPGN126992(N2kMsg, SID, SystemDate, SystemTime, TimeSource)) {
        if (TimeSource == N2ktimes_GPS || TimeSource == N2ktimes_GLONASS) {
            SetSystemTime(SystemDate, SystemTime);
            
            if (debugMode) {
                Serial.printf("Received System Time (PGN 126992): Date=%u, Time=%.1fs, Source=%d\n",
                    SystemDate, SystemTime, TimeSource);
            }
        }
    }
}

/**
 * Parse PGN 129029 - GNSS Position Data (contains time)
 */
void ParseN2kGNSS(const tN2kMsg& N2kMsg) {
    unsigned char SID;
    uint16_t DaysSince1970;
    double SecondsSinceMidnight;
    double Latitude;
    double Longitude;
    double Altitude;
    tN2kGNSStype GNSStype;
    tN2kGNSSmethod GNSSmethod;
    unsigned char nSatellites;
    double HDOP;
    double PDOP;
    double GeoidalSeparation;
    unsigned char nReferenceStations;
    tN2kGNSStype ReferenceStationType;
    uint16_t ReferenceSationID;
    double AgeOfCorrection;
    
    if (ParseN2kPGN129029(N2kMsg, SID, DaysSince1970, SecondsSinceMidnight,
                          Latitude, Longitude, Altitude, GNSStype, GNSSmethod,
                          nSatellites, HDOP, PDOP, GeoidalSeparation,
                          nReferenceStations, ReferenceStationType,
                          ReferenceSationID, AgeOfCorrection)) {
        
        SetSystemTime(DaysSince1970, SecondsSinceMidnight);
        
        if (debugMode) {
            Serial.printf("Received GNSS Time (PGN 129029): Date=%u, Time=%.1fs, Sats=%u\n",
                DaysSince1970, SecondsSinceMidnight, nSatellites);
        }
    }
}

/**
 * Handle incoming NMEA2000 messages
 */
void handleN2kMessages(const tN2kMsg& N2kMsg) {
    for (uint8_t i = 0; NMEA2000Handlers[i].PGN != 0; i++) {
        if (N2kMsg.PGN == NMEA2000Handlers[i].PGN) {
            NMEA2000Handlers[i].Handler(N2kMsg);
            break;
        }
    }
}

// ============================================================================
// NMEA2000 CALLBACK
// ============================================================================
void OnN2kOpen() {
    FluidLevelScheduler.UpdateNextTime();
    MeasurementScheduler.UpdateNextTime();
}

void CheckN2kSourceAddressChange() {
    if (!NMEA2000.ReadResetAddressChanged()) {
        return;
    }

	uint8_t newSource = NMEA2000.GetN2kSource();
    Serial.printf("NMEA2000 source address changed to %u\n", newSource);
    Preferences prefs;
    prefs.begin("n2k", false);
    prefs.putUChar("N2KSource", newSource);
	prefs.end();
}

// ============================================================================
// SETUP
// ============================================================================
void setup() {
    uint8_t chipid[6];
    uint32_t id = 0;

    esp_efuse_mac_get_default(chipid);
    for (int i = 0; i < 6; i++) id += (chipid[i] << (7 * i));

    Serial.begin(115200);
    while (!Serial) {
        static unsigned long startWait = millis();
        if (millis() - startWait > 3000) break;
    }

    Serial.println("\n========================================");
    Serial.println("Hardware Configuration Check");
    Serial.println("========================================");
    Serial.printf("Board: %s\n", BOARD_NAME);
    Serial.printf("CAN TX Pin: GPIO%d\n", ESP32_CAN_TX_PIN);
    Serial.printf("CAN RX Pin: GPIO%d\n", ESP32_CAN_RX_PIN);
    Serial.printf("Sensor Pin: GPIO%d\n", SENSOR_PIN);
    Serial.println("CAN Transceiver: MCP2562");
    Serial.println("========================================");

    Serial.println("========================================");
    Serial.println("NMEA2000 Resistive Fluid Level Sensor");
    Serial.println("========================================");
    Serial.printf("Firmware version: %s\n", Version);
    Serial.println("Sensor type: Resistive (240-33 Ohm)");
    Serial.printf("Sensor pin: GPIO_%d\n", SENSOR_PIN);
    Serial.println("========================================\n");

    RebootManager::begin();
    Serial.printf("Reboot count: %d\n", RebootManager::getRebootCount());
    Serial.printf("Last reboot reason: %s\n", RebootManager::getLastRebootReasonText().c_str());

    Serial.println("\nInitializing sensor...");
    initSensor();

    Serial.println("\nInitializing WiFi...");
    wifiInit();

    Serial.println("\n========================================");
    Serial.println("Setting up NMEA2000...");
    Serial.println("========================================");

    // load N2K source address from Preferences if available
    Preferences prefs_;
    prefs_.begin("n2k", true); // read-only
    uint8_t source_ = prefs_.getUChar("N2KSource", 22); // fallback: default 22
    prefs_.end();

    Serial.printf("Requested N2K Source Address: %u\n", source_);

    NMEA2000.SetN2kCANMsgBufSize(8);
    NMEA2000.SetN2kCANReceiveFrameBufSize(150);
    NMEA2000.SetN2kCANSendFrameBufSize(150);
    Serial.println("Buffer sizes configured");

    NMEA2000.SetProductInformation(
        "1",
        100,
        "Resistive Level Sensor 240-33",
        Version,
        Version,
        1,
        0xffff,
        0xff
    );
    Serial.println("Product information set");

    NMEA2000.SetDeviceInformation(
        id,
        150,
        75,
        2040
    );
    Serial.println("Device information set");

    NMEA2000.SetConfigurationInformation(
        String(id).c_str(),
        iotWebConf.getThingName(),
        ""
    );
    Serial.println("Configuration information set");

    NMEA2000.EnableForward(false);
    NMEA2000.SetMode(tNMEA2000::N2km_NodeOnly);
    NMEA2000.SetN2kSource(source_);

    NMEA2000.ExtendTransmitMessages(TransmitMessages);
//    NMEA2000.ExtendReceiveMessages(ReceiveMessages);

//    NMEA2000.SetMsgHandler(handleN2kMessages);
    NMEA2000.SetOnOpen(OnN2kOpen);
    
    NMEA2000.Open();

    esp_task_wdt_add(NULL);
}

// ============================================================================
// SENSOR FUNCTIONS
// ============================================================================

/**
 * Initialize ADC for resistive sensor
 */
void initSensor() {
    Serial.println("  Configuring ADC...");
    pinMode(SENSOR_PIN, INPUT);
    analogSetAttenuation(ADC_11db);
    analogReadResolution(12);

    Serial.println("  Taking test reading...");
    float testResistance_ = readResistance();

    Serial.printf("  Test resistance: %.1f Ohm\n", testResistance_);

    if (testResistance_ >= RESISTANCE_MIN && testResistance_ <= RESISTANCE_MAX) {
        SensorStatus = "Ok";
        Serial.printf("  Sensor OK: %.1f Ohm\n", testResistance_);
        Serial.printf("  Fill level: %.1f%%\n", calculateFillLevel(testResistance_));
    }
    else if (testResistance_ < RESISTANCE_MIN) {
        SensorStatus = "Short circuit";
        Serial.println("  WARNING: Short circuit detected!");
    }
    else {
        SensorStatus = "Open circuit";
        Serial.println("  WARNING: Open circuit detected!");
    }

    Serial.println("Sensor initialization complete");
}

/**
 * Read resistance from sensor using voltage divider
 * Circuit: 3.3V --- R_ref --- ADC_PIN --- R_sensor --- GND
 */
float readResistance() {
    uint32_t adcSum_ = 0;

    for (int i_ = 0; i_ < ADC_SAMPLES; i_++) {
        adcSum_ += analogRead(SENSOR_PIN);
        delayMicroseconds(1000);
    }

    float adcAverage_ = (float)adcSum_ / ADC_SAMPLES;
    float voltage_ = (adcAverage_ / ADC_RESOLUTION) * ADC_VOLTAGE;

    if (voltage_ <= 0.01f) {
        return 0.0f;
    }
    if (voltage_ >= (ADC_VOLTAGE - 0.01f)) {
        return 999.0f;
    }

    float resistance_ = REFERENCE_RESISTOR * voltage_ / (ADC_VOLTAGE - voltage_);

    return resistance_;
}
/**
 * Convert resistance to fill level percentage
 * Automatically handles both orientations:
 * - FULL < EMPTY (e.g., 33? -> 205?)
 * - FULL > EMPTY (e.g., 240? -> 33?)
 */
float calculateFillLevel(float resistance) {
    float resistanceFull_ = sensorConfig.getResistanceFull();
    float resistanceEmpty_ = sensorConfig.getResistanceEmpty();

    // Auto-detect if values are inverted
    if (resistanceFull_ > resistanceEmpty_) {
        // Swap values: User configured it backwards
        float temp_ = resistanceFull_;
        resistanceFull_ = resistanceEmpty_;
        resistanceEmpty_ = temp_;
    }

    // Handle faults
    if (resistance < RESISTANCE_MIN || resistance > RESISTANCE_MAX) {
        return 0.0f;  // Report empty on fault
    }

    // Clamp resistance to valid range
    if (resistance < resistanceFull_) resistance = resistanceFull_;
    if (resistance > resistanceEmpty_) resistance = resistanceEmpty_;

    // Linear interpolation
    // resistanceFull_ -> 100%, resistanceEmpty_ -> 0%
    float fillLevel_ = 100.0f - ((resistance - resistanceFull_) /
        (resistanceEmpty_ - resistanceFull_) * 100.0f);

    // Clamp to 0-100%
    if (fillLevel_ < 0.0f) fillLevel_ = 0.0f;
    if (fillLevel_ > 100.0f) fillLevel_ = 100.0f;

    // Round to 1 decimal place
    fillLevel_ = round(fillLevel_ * 10.0f) / 10.0f;

    return fillLevel_;
}

/**
 * Get averaged fill level from ring buffer
 */
float getAverageFillLevel() {
    if (gAverageFillLevel.size() == 0) {
        return 0.0f;
    }

    float sum_ = 0.0f;
    for (uint8_t i = 0; i < gAverageFillLevel.size(); i++) {
        sum_ += gAverageFillLevel[i];
    }

    return sum_ / gAverageFillLevel.size();
}

/**
 * Get current fill level (most recent reading)
 */
float getCurrentFillLevel() {
    // Returns the last added value from the ring buffer without averaging
    // This is the most recent sensor reading
    if (gAverageFillLevel.size() > 0) {
        return gAverageFillLevel[gAverageFillLevel.size() - 1];
    }
    return 0.0f;
}

/**
 * Get current time as string
 */
String getCurrentTime() {
    time_t now_ = time(nullptr);
    struct tm* timeInfo_ = localtime(&now_);

    char buffer_[9];
    strftime(buffer_, sizeof(buffer_), "%H:%M:%S", timeInfo_);

    return String(buffer_);
}

/**
 * Read sensor and update fill level
 */
void MeasureFillLevel() {
    if (SensorStatus != "Ok") {
        if (debugMode) {
            WebSerial.printf("%s : Sensor fault: %s\n", getCurrentTime().c_str(), SensorStatus.c_str());
        }
        return;
    }

    // Read resistance
    float resistance_ = readResistance();

    // Check for faults
    if (resistance_ < RESISTANCE_MIN) {
        SensorStatus = "Short circuit";
        if (debugMode) {
            Serial.printf("%s : Short circuit detected (%.1f Ohm)\n", getCurrentTime().c_str(), resistance_);
        }
        gAverageFillLevel.pushOverwrite(0.0f);
        return;
    }

    if (resistance_ > RESISTANCE_MAX) {
        SensorStatus = "Open circuit";
        if (debugMode) {
            Serial.printf("%s : Open circuit detected (%.1f Ohm)\n", getCurrentTime().c_str(), resistance_);
        }
        gAverageFillLevel.pushOverwrite(0.0f);
        return;
    }

    // Reset status if it was faulty before
    if (SensorStatus != "Ok") {
        SensorStatus = "Ok";
        if (debugMode) {
            Serial.println("Sensor recovered");
        }
    }

    // Calculate fill level
    float fillLevel_ = calculateFillLevel(resistance_);

    // Add to ring buffer
    gAverageFillLevel.pushOverwrite(fillLevel_);

    // Get average
    float avgFillLevel_ = getAverageFillLevel();

    // Debug output (only if debugMode is enabled)
    if (debugMode) {
        Serial.printf("%s : Resistance: %.1f Ohm | Fill: %.1f%% | Avg: %.1f%%\n",
            getCurrentTime().c_str(), resistance_, fillLevel_, avgFillLevel_);
    }

    WebSerial.printf("%s : Resistance: %.1f Ohm | Fill: %.1f%% | Avg: %.1f%%\n",
        getCurrentTime().c_str(), resistance_, fillLevel_, avgFillLevel_);

    // Detect fast changes (>10% threshold)
    float threshold_ = 10.0f;
    if (abs(fillLevel_ - avgFillLevel_) > threshold_) {
        if (debugMode) {
            Serial.printf("Fast change detected: %.1f%% -> %.1f%%\n", avgFillLevel_, fillLevel_);
        }
        // Reset buffer with new value
        for (int i_ = 0; i_ < TANK_RINGBUF_SIZE; i_++) {
            gAverageFillLevel.pushOverwrite(fillLevel_);
        }
    }
}

/**
 * Send fluid level to NMEA2000 bus
 */
void SendN2kFluidLevel(void) {
    if (!FluidLevelScheduler.IsTime()) {
        return;
    }

    FluidLevelScheduler.UpdateNextTime();

    float fillLevel_ = DampingPaused ? getCurrentFillLevel() : getAverageFillLevel();

    // Validate and constrain level
    if (isnan(fillLevel_) || fillLevel_ < 0.0f) {
        fillLevel_ = N2kDoubleNA;
    }
    else {
        fillLevel_ = constrain(fillLevel_, 0.0f, 100.0f);
    }

    // DEBUG: Print tank info
    if (debugMode) {
        Serial.printf("DEBUG - Tank Capacity: %.1fL\n", tank.Capacity());
        Serial.printf("DEBUG - Tank Type: %d\n", tank.Type());
        Serial.printf("DEBUG - Instance: %d\n", Config.Instance());
        Serial.printf("DEBUG - Fill Level: %.1f%%\n", fillLevel_);
    }

    // Create and send message
    tN2kMsg N2kMsg_;
    SetN2kFluidLevel(N2kMsg_, Config.Instance(), tank.Type(), fillLevel_, tank.Capacity());

    bool success = NMEA2000.SendMsg(N2kMsg_);

    // Output
    if (debugMode) {
        if (success) {
            if (fillLevel_ == N2kDoubleNA) {
                Serial.println("Sent NMEA2000: Level unavailable");
            }
            else {
                float volumeLiters = fillLevel_ * tank.Capacity() / 100.0f;
                Serial.printf("Sent NMEA2000: %.1f%% (%.1fL / %.1fL)\n",
                    fillLevel_, volumeLiters, tank.Capacity());

                // WARNING if capacity is zero
                if (tank.Capacity() <= 0.0f) {
                    Serial.println("WARNING: Tank capacity is zero! Check configuration.");
                }
            }
        }
        else {
            Serial.println("Failed to send NMEA2000 message!");
        }
    }
}

// ============================================================================
// MAIN LOOP
// ============================================================================
void loop() {
    SendN2kFluidLevel();
    NMEA2000.ParseMessages();
    wifiLoop();

    CheckN2kSourceAddressChange();

    if (MeasurementScheduler.IsTime()) {
        MeasurementScheduler.UpdateNextTime();
        MeasureFillLevel();
    }

    if (NMEA2000.GetN2kSource() != Config.Source()) {
        Config.SetSource(NMEA2000.GetN2kSource());
        ParamsSave = true;
    }

    if (ParamsChanged) {
        ParamsChanged = false;
        NMEA2000.SetInstallationDescription1(iotWebConf.getThingName());
    }
    
    if (TimeSet && (millis() - LastTimeUpdate > 3600000)) {
        Serial.println("Warning: No time update received for 1 hour");
        TimeSet = false;
    }

    if (Serial.available()) {
        Serial.read();
    }

    esp_task_wdt_reset();
}