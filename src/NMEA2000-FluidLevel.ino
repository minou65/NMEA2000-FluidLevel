// ============================================================================
// NMEA2000 Fluid Level Sensor - Resistive Sensor (240-33 Ohm)
// ============================================================================
// Sensor: 240-33 Ohm resistive level sensor
// - 33 Ohm  = Tank FULL (100%)
// - 240 Ohm = Tank EMPTY (0%)
// - Open circuit (infinite resistance) = Tank EMPTY
// - Short circuit (0 Ohm) = Tank EMPTY
// ============================================================================;



// #define DEBUG_NMEA_MSG_ASCII // If you want to use simple ascii monitor like Arduino Serial Monitor, uncomment this line
// #define DEBUG_NMEA_Actisense // Uncomment this, so you can test code without CAN bus chips on Arduino Mega
// #define DEBUG_MSG



// ============================================================================
// INCLUDES
// ============================================================================

#include "Config.h"
#include "common.h"
#include "webhandling.h"
#include "version.h"

#include <Arduino.h>
#include <esp_mac.h>
#include <N2kMessages.h>
#include <NMEA2000_CAN.h>
#include <Preferences.h>
#include <esp_task_wdt.h>
#include <RebootManager.h>
#include <EventLogger.h>
#include <time.h>
#include <sys/time.h>
#include <driver/twai.h>
#



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

    uint8_t newSource_ = NMEA2000.GetN2kSource();
    Serial.printf("NMEA2000 source address changed to %u\n", newSource_);
    
    Preferences prefs_;
    prefs_.begin("n2k", false);
    prefs_.putUChar("N2KSource", newSource_);
    prefs_.end();
    
    // Update NMEAConfig in Web UI
    Config.SetSource(newSource_);
}

// ============================================================================
// SETUP
// ============================================================================
void setup() {
    uint8_t chipid_[6];
    uint32_t id_ = 0;

    esp_efuse_mac_get_default(chipid_);
    for (int i_ = 0; i_ < 6; i_++) id_ += (chipid_[i_] << (7 * i_));

    Serial.begin(115200);
    while (!Serial) {
        static unsigned long startWait_ = millis();
        if (millis() - startWait_ > 3000) break;
    }

    eventLogger.log(F("========================================"));
    eventLogger.log(F("Hardware Configuration Check"));
    eventLogger.log(F("========================================"));
    eventLogger.logf("Board: %s", BOARD_NAME);
    eventLogger.logf("CAN TX Pin: GPIO%d", ESP32_CAN_TX_PIN);
    eventLogger.logf("CAN RX Pin: GPIO%d", ESP32_CAN_RX_PIN);
    eventLogger.logf("Sensor Pin: GPIO%d", SENSOR_PIN);
    eventLogger.log(F("CAN Transceiver: MCP2562"));
    eventLogger.log(F("========================================"));

    eventLogger.log(F("========================================"));
    eventLogger.log(F("NMEA2000 Resistive Fluid Level Sensor"));
    eventLogger.log(F("========================================"));
    eventLogger.logf("Firmware version: %s", Version);
    eventLogger.log(F("Sensor type: Resistive (240-33 Ohm)"));
    eventLogger.logf("Sensor pin: GPIO_%d", SENSOR_PIN);
    eventLogger.log(F("========================================"));

    RebootManager::begin();
    eventLogger.logf("Reboot count: %d", RebootManager::getRebootCount());
    eventLogger.logf("Last reboot reason: %s", RebootManager::getLastRebootReasonText().c_str());

    randomSeed(esp_random());

    // ===============================================================
    // MAC-basierte eindeutige Adressierung (Bereich 20-50)
    // ===============================================================
    uint8_t mac_[6];
    esp_efuse_mac_get_default(mac_);
    
    // Verwendet MAC[4] und MAC[5] für maximale Variation im Bereich 20-50
    uint8_t baseAddress_ = 20 + ((mac_[4] + mac_[5]) % 31);  // Ergibt 20-50
    
    eventLogger.logf("Device MAC: %02X:%02X:%02X:%02X:%02X:%02X",
        mac_[0], mac_[1], mac_[2], mac_[3], mac_[4], mac_[5]);
    eventLogger.logf("Calculated base N2K address: %u (range 20-50, based on MAC[4]+MAC[5])", baseAddress_);

    // MAC-basierter Startup-Delay (0-5100ms)
    uint32_t randomDelay_ = (mac_[5] * 20);
    eventLogger.logf("Startup delay: %lu ms (prevents address collision)", randomDelay_);
    delay(randomDelay_);

    eventLogger.log(F("Initializing sensor..."));
    initSensor();

    // Load N2K source address from Preferences
    Preferences prefs_;
    prefs_.begin("n2k", false);  // read-write

    uint8_t val_ = prefs_.getUChar("N2KSource", 255);  // 255 = nicht gesetzt
    uint8_t source_;

    if (val_ == 255) {
        // Erste Inbetriebnahme: MAC-basierte Adresse speichern
        source_ = baseAddress_;
        prefs_.putUChar("N2KSource", baseAddress_);
        eventLogger.logf("First boot: Set N2KSource = %u (MAC-based)", baseAddress_);
    }
    else {
        source_ = val_;
    }

    prefs_.end();

    eventLogger.logf("Using N2K address: %u", source_);

    eventLogger.log(F("Initializing WiFi..."));
    wifiInit();

    eventLogger.log(F("========================================"));
    eventLogger.log(F("Setting up NMEA2000..."));
    eventLogger.log(F("========================================"));

    NMEA2000.SetN2kCANMsgBufSize(8);
    NMEA2000.SetN2kCANReceiveFrameBufSize(150);
    NMEA2000.SetN2kCANSendFrameBufSize(150);
    eventLogger.log(F("Buffer sizes configured"));

    // Kurzer, eindeutiger Device Model ID basierend auf MAC
    char deviceModelId_[16];
    snprintf(deviceModelId_, sizeof(deviceModelId_), "FL-%02X", mac_[5]);

    // Kurzer Product Name
    char productName_[32];
    snprintf(productName_, sizeof(productName_), "Level-%02X", mac_[5]);

    NMEA2000.SetProductInformation(
        "1",
        100,
        productName_,
        Version,
        Version,
        1,
        0xffff,
        0xff
    );
    eventLogger.log(F("Product information set"));

    NMEA2000.SetDeviceInformation(
        id_,
        150,
        75,
        2040
    );
    eventLogger.log(F("Device information set"));

    NMEA2000.SetConfigurationInformation(
        deviceModelId_,
        iotWebConf.getThingName(),
        ""
    );
    eventLogger.log(F("Configuration information set"));

    NMEA2000.EnableForward(false);
    NMEA2000.SetMode(tNMEA2000::N2km_NodeOnly);
    NMEA2000.SetN2kSource(source_);

    NMEA2000.ExtendTransmitMessages(TransmitMessages);
    NMEA2000.ExtendReceiveMessages(ReceiveMessages);

    NMEA2000.SetMsgHandler(handleN2kMessages);
    NMEA2000.SetOnOpen(OnN2kOpen);

    eventLogger.log(F("Opening NMEA2000..."));
    NMEA2000.Open();

    esp_task_wdt_add(NULL);

    eventLogger.log(F("========================================"));
    eventLogger.log(F("NMEA2000 started"));
    eventLogger.logf("Base address: %u, Using address: %u",
        baseAddress_, source_);
    eventLogger.log(F("Listening for GPS time on NMEA2000 bus"));
    eventLogger.log(F("Setup complete"));
    eventLogger.log(F("========================================"));
}

// ============================================================================
// SENSOR FUNCTIONS
// ============================================================================

/**
 * Initialize ADC for resistive sensor
 */
void initSensor() {
    eventLogger.log(F("  Configuring ADC..."));
    pinMode(SENSOR_PIN, INPUT);
    analogSetAttenuation(ADC_11db);
    analogReadResolution(12);

    eventLogger.log(F("  Taking test reading..."));
    float testResistance_ = readResistance();

    eventLogger.logf("  Test resistance: %.1f Ohm", testResistance_);

    if (testResistance_ >= RESISTANCE_MIN && testResistance_ <= RESISTANCE_MAX) {
        SensorStatus = "Ok";
        eventLogger.logf("  Sensor OK: %.1f Ohm", testResistance_);
        eventLogger.logf("  Fill level: %.1f%%", calculateFillLevel(testResistance_));
    }
    else if (testResistance_ < RESISTANCE_MIN) {
        SensorStatus = "Short circuit";
        eventLogger.log(F("  WARNING: Short circuit detected!"));
    }
    else {
        SensorStatus = "Open circuit";
        eventLogger.log(F("  WARNING: Open circuit detected!"));
    }

    eventLogger.log(F("Sensor initialization complete"));
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
            eventLogger.logf("%s : Sensor fault: %s", getCurrentTime().c_str(), SensorStatus.c_str());
        }
        return;
    }

    // Read resistance
    float resistance_ = readResistance();

    // Check for faults
    if (resistance_ < RESISTANCE_MIN) {
        SensorStatus = "Short circuit";
        if (debugMode) {
            eventLogger.logf("%s : Short circuit detected (%.1f Ohm)", getCurrentTime().c_str(), resistance_);
        }
        gAverageFillLevel.pushOverwrite(0.0f);
        return;
    }

    if (resistance_ > RESISTANCE_MAX) {
        SensorStatus = "Open circuit";
        if (debugMode) {
            eventLogger.logf("%s : Open circuit detected (%.1f Ohm)", getCurrentTime().c_str(), resistance_);
        }
        gAverageFillLevel.pushOverwrite(0.0f);
        return;
    }

    // Reset status if it was faulty before
    if (SensorStatus != "Ok") {
        SensorStatus = "Ok";
        if (debugMode) {
            eventLogger.log(F("Sensor recovered"));
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
        eventLogger.logf("%s : Resistance: %.1f Ohm | Fill: %.1f%% | Avg: %.1f%%",
            getCurrentTime().c_str(), resistance_, fillLevel_, avgFillLevel_);
    }

    // Detect fast changes (>10% threshold)
    float threshold_ = 10.0f;
    if (abs(fillLevel_ - avgFillLevel_) > threshold_) {
        if (debugMode) {
            eventLogger.logf("Fast change detected: %.1f%% -> %.1f%%", avgFillLevel_, fillLevel_);
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
        eventLogger.logf("DEBUG - Tank Capacity: %.1fL", tank.Capacity());
        eventLogger.logf("DEBUG - Tank Type: %d", tank.Type());
        eventLogger.logf("DEBUG - Instance: %d", Config.Instance());
        eventLogger.logf("DEBUG - Fill Level: %.1f%%", fillLevel_);
    }

    // Create and send message
    tN2kMsg N2kMsg_;
    SetN2kFluidLevel(N2kMsg_, Config.Instance(), tank.Type(), fillLevel_, tank.Capacity());

    bool success = NMEA2000.SendMsg(N2kMsg_);

    // Output
    if (debugMode) {
        if (success) {
            if (fillLevel_ == N2kDoubleNA) {
                eventLogger.log(F("Sent NMEA2000: Level unavailable"));
            }
            else {
                float volumeLiters = fillLevel_ * tank.Capacity() / 100.0f;
                eventLogger.logf("Sent NMEA2000: %.1f%% (%.1fL / %.1fL)",
                    fillLevel_, volumeLiters, tank.Capacity());

                // WARNING if capacity is zero
                if (tank.Capacity() <= 0.0f) {
                    eventLogger.log(F("WARNING: Tank capacity is zero! Check configuration."));
                }
            }
        }
        else {
            eventLogger.log(F("Failed to send NMEA2000 message!"));
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
        eventLogger.log(F("Warning: No time update received for 1 hour"));
        TimeSet = false;
    }

    if (Serial.available()) {
        Serial.read();
    }

    if (ShouldReboot) {
        eventLogger.log(F("Rebooting..."));
        delay(1000);
        ESP.restart();
    }

    esp_task_wdt_reset();
}
