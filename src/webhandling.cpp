#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>

#include "common.h"
#include "webhandling.h"
#include "favicon.h"
#include "neotimer.h"

#include <DNSServer.h>
#include <IotWebConfAsyncUpdateServer.h>
#include <IotWebRootTab.h>
#include <Preferences.h>

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "A7"

// -- When CONFIG_PIN is pulled to ground on startup, the Thing will use the initial
//      password to buld an AP. (E.g. in case of lost password)
#define CONFIG_PIN  GPIO_NUM_13

// -- Status indicator pin.
//      First it will light up (kept LOW), on Wifi connection it will blink,
//      when connected to the Wifi it will turn off (kept HIGH).
#define STATUS_PIN LED_BUILTIN
#define ON_LEVEL HIGH

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "NMEA2000-FluidLevel";

const char html_form_end[] PROGMEM = R"=====(
</br><form action='/reboot' method='get'><button type='submit'>Reboot</button></form>
</br><form action='/' method='get' style='display:inline;'>
  <button type='submit'>Home</button>
  </br>
</form>
</br><form action='/post' method='post' style='display:inline;' onsubmit="event.preventDefault(); postReset();">
  <button type='submit'>Reset to factory defaults</button>
</form>
<script>
function postReset() {
    fetch('/post', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: 'reset=true'
    })
    .then(response => {
        if (response.ok) { 
            window.location.reload(true);  // Hard reload
        }
    })
    .catch(error => {
        console.error('Reset fehlgeschlagen:', error);
        alert('Reset fehlgeschlagen!');
    });
}
</script>
)=====";

const char html_end_template[] PROGMEM = R"=====(
</div></body></html>
)=====";

uint8_t APModeOfflineTime = 0;
Neotimer APModeTimer = Neotimer();

// Damping pause functionality
bool DampingPaused = false;
Neotimer DampingPauseTimer = Neotimer(15 * 60 * 1000); // 15 Minuten

// -- Method declarations.
void onWebSerialMessage(uint8_t* data, size_t len);
void handleData(AsyncWebServerRequest* request);
void handlePost(AsyncWebServerRequest* request);
void handleReboot(AsyncWebServerRequest* request);
void handleRoot(AsyncWebServerRequest* request);
void handleToggleDamping(AsyncWebServerRequest* request);

// -- Callback methods.
void wifiConnected();
void convertParams();
void configSaved();

bool ParamsChanged = true;
bool ShouldReboot = false;

DNSServer dnsServer;
AsyncWebServer server(80);
AsyncWebServerWrapper asyncWebServerWrapper(&server);
AsyncUpdateServer AsyncUpdater;

AsyncIotWebConfTab iotWebConf(thingName, &dnsServer, &asyncWebServerWrapper, wifiInitialApPassword, CONFIG_VERSION);

char APModeOfflineValue[STRING_LEN];
iotwebconf::NumberParameter APModeOfflineParam = iotwebconf::NumberParameter("AP offline mode after (minutes)", "APModeOffline", APModeOfflineValue, NUMBER_LEN, "0", "0..30", "min='0' max='30', step='1'");

NMEAConfig Config = NMEAConfig();
Tank tank = Tank("tank", "Tank configuration");
SensorConfig sensorConfig;

class CustomHtmlFormatProvider : public AsyncTabHtmlFormatProvider {
public:
    CustomHtmlFormatProvider(std::vector<AsyncTabInfo>* tabs, int minWidth = 500, int maxWidth = 600)
        : AsyncTabHtmlFormatProvider(tabs, minWidth, maxWidth) {
    }

protected:
    String getFormEnd() override {
        return AsyncTabHtmlFormatProvider::getFormEnd() + String(FPSTR(html_form_end));
    }
};
CustomHtmlFormatProvider* customHtmlFormatProvider = nullptr;

class MyHtmlRootFormatProvider : public TabHtmlRootFormatProvider {
public:
    virtual String getScriptInner() {
        String s_ = TabHtmlRootFormatProvider::getScriptInner();
        s_.replace("{millisecond}", "1000");
        s_ += F("function updateData(jsonData) {\n");
        s_ += F("   document.getElementById('RSSIValue').innerHTML = jsonData.rssi + \"dBm\" \n");

        s_ += F("   const tankelement = document.getElementById('tankValue'); \n");
        s_ += F("   tankelement.value = jsonData.filledpercent; \n");
        s_ += F("   tankelement.innerHTML = jsonData.filledpercent; \n");
        s_ += F("   document.getElementById('capacityValue').innerHTML = jsonData.capacity; \n");
        s_ += F("   document.getElementById('filledpercentValue').innerHTML = jsonData.filledpercent; \n");
        s_ += F("   document.getElementById('statusValue').innerHTML = jsonData.status; \n");
        
        // Damping status update
        s_ += F("   const dampingBtn = document.getElementById('dampingBtn'); \n");
        s_ += F("   if (jsonData.dampingPaused) { \n");
        s_ += F("       dampingBtn.innerHTML = 'Enable damping (' + jsonData.dampingRemaining + ')'; \n");
        s_ += F("       dampingBtn.className = 'button button-on'; \n");
        s_ += F("       document.getElementById('dampingStatus').innerHTML = 'Damping paused'; \n");
        s_ += F("       document.getElementById('dampingStatus').style.color = 'orange'; \n");
        s_ += F("   } else { \n");
        s_ += F("       dampingBtn.innerHTML = 'Pause damping'; \n");
        s_ += F("       dampingBtn.className = 'button'; \n");
        s_ += F("       document.getElementById('dampingStatus').innerHTML = 'Damping active'; \n");
        s_ += F("       document.getElementById('dampingStatus').style.color = 'green'; \n");
        s_ += F("   } \n");
        s_ += F("}\n");
        
        // Toggle damping function
        s_ += F("function toggleDamping() {\n");
        s_ += F("   fetch('/toggledamping').then(() => getData());\n");
        s_ += F("}\n");

        return s_;
    }
    
    virtual String getStyleInner() {
        String s_ = TabHtmlRootFormatProvider::getStyleInner();
        s_ += F(".button { \n");
        s_ += F("   background-color: #4CAF50; \n");
        s_ += F("   border: none; \n");
        s_ += F("   color: white; \n");
        s_ += F("   padding: 15px 32px; \n");
        s_ += F("   text-align: center; \n");
        s_ += F("   text-decoration: none; \n");
        s_ += F("   display: inline-block; \n");
        s_ += F("   font-size: 16px; \n");
        s_ += F("   margin: 4px 2px; \n");
        s_ += F("   cursor: pointer; \n");
        s_ += F("   border-radius: 4px; \n");
        s_ += F("} \n");
        s_ += F(".button-on { \n");
        s_ += F("   background-color: #ff9800; \n");
        s_ += F("} \n");
        return s_;
    }
};


void wifiInit() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("starting up...");

    iotWebConf.setStatusPin(STATUS_PIN, ON_LEVEL);
    iotWebConf.setConfigPin(CONFIG_PIN);

    // === TABS KONFIGURIEREN ===
    // System Tab soll an letzter Stelle sein
    iotWebConf.setSystemTabPosition(-1);  // -1 = letzte Position
    iotWebConf.setSystemTabName("System");

    // Parameter Groups zu Tabs hinzufügen
    iotWebConf.addParameterGroup(&tank, "Tank");           // Tab "Tank" mit Tank Config
    iotWebConf.addParameterGroup(&sensorConfig, "Tank");   // Sensor Calibration auch in "Tank"
    iotWebConf.addParameterGroup(&Config, "NMEA");         // Tab "NMEA"

    // System Parameter (wird automatisch im System Tab angezeigt)
    iotWebConf.addSystemParameter(&APModeOfflineParam);

    auto customProvider = new CustomHtmlFormatProvider(iotWebConf.getTabsVector());
    customProvider->setContainerWidth(700, 700); // Set a fixed width for the configuration portal
    iotWebConf.setHtmlFormatProvider(customProvider);

    iotWebConf.setupUpdateServer(
        [](const char* updatePath) { AsyncUpdater.setup(&server, updatePath); },
        [](const char* userName, char* password) { AsyncUpdater.updateCredentials(userName, password); });

    iotWebConf.setConfigSavedCallback(&configSaved);
    iotWebConf.setWifiConnectionCallback(&wifiConnected);

    iotWebConf.getApTimeoutParameter()->visible = true;

    // -- Initializing the configuration.
    iotWebConf.init();

    convertParams();

    // -- Set up required URL handlers on the web server.
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) { handleRoot(request); });
    server.on("/config", HTTP_ANY, [](AsyncWebServerRequest* request) {
        auto* asyncWebRequestWrapper_ = new AsyncWebRequestWrapper(request);
        iotWebConf.handleConfig(asyncWebRequestWrapper_);
        }
    );

    IconHandler::registerHandlers(&server, favicon_ico, sizeof(favicon_ico));

    server.on("/data", HTTP_GET, [](AsyncWebServerRequest* request) { handleData(request); });
    server.on("/post", HTTP_ANY, [](AsyncWebServerRequest* request) { handlePost(request); });
    server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest* request) { handleReboot(request); });
    server.on("/toggledamping", HTTP_GET, [](AsyncWebServerRequest* request) { handleToggleDamping(request); });
    
    server.onNotFound([](AsyncWebServerRequest* request) {
        AsyncWebRequestWrapper asyncWebRequestWrapper(request);
        iotWebConf.handleNotFound(&asyncWebRequestWrapper);
        }
    );

	WebSerial.begin(&server, "/webserial");
    WebSerial.onMessage(onWebSerialMessage);

    if (APModeOfflineTime > 0) {
        APModeTimer.start(APModeOfflineTime * 60 * 1000);
    }

    Serial.println("Ready.");
}

void wifiLoop() {
    // -- doLoop should be called as frequently as possible.
    iotWebConf.doLoop();
    ArduinoOTA.handle();

    if (ParamsSave) {
        Serial.println(F("Parameters are changed,save them"));
        iotWebConf.saveConfig();
        ParamsSave = false;
    }

    if (APModeTimer.done()) {
        Serial.println(F("AP mode offline time reached"));
        iotWebConf.goOffLine();
        APModeTimer.stop();
    }
    
    // Check if damping pause timer is done
    if (DampingPaused && DampingPauseTimer.done()) {
        DampingPaused = false;
        Serial.println(F("Damping pause ended, resuming normal operation"));
    }

    if (AsyncUpdater.isFinished()) {
        Serial.println(F("Firmware update finished"));
        ShouldReboot = true;
    }
}

void wifiConnected() {
    ArduinoOTA.begin();
}

void onWebSerialMessage(uint8_t* data, size_t len) {
    String cmd_ = "";
    for (size_t i_ = 0; i_ < len; i_++) {
        cmd_ += (char)data[i_];
    }
    cmd_.trim();

    if (cmd_.startsWith("reboot")) {
        ShouldReboot = true;
    }
    else if (cmd_.startsWith("status")) {
        WebSerial.println("========================================");
        WebSerial.println("Resistive Fluid Level Sensor Status");
        WebSerial.println("========================================");
        WebSerial.printf("Sensor type: Resistive (240-33 Ohm)\n");
        WebSerial.printf("Sensor status: %s\n", SensorStatus.c_str());
        WebSerial.printf("Fill level: %.1f%%\n", getAverageFillLevel());
        WebSerial.printf("Tank capacity: %d liters\n", tank.Capacity());
        WebSerial.printf("Fluid type: %s\n", tank.TypeName());
        WebSerial.println("========================================");
    }
    else if (cmd_.startsWith("help")) {
        WebSerial.println("Available commands:");
        WebSerial.println(" status  - Show sensor status");
        WebSerial.println(" reboot  - Reboot device");
        WebSerial.println(" help    - Show this help");
    }
    else {
        WebSerial.println("Unknown command. Type 'help' for available commands.");
    }
}

void handleToggleDamping(AsyncWebServerRequest* request) {
    if (DampingPaused) {
        // Re-enable damping
        DampingPaused = false;
        DampingPauseTimer.stop();
        Serial.println(F("Damping re-enabled by user"));
    }
    else {
        // Pause damping
        DampingPaused = true;
        DampingPauseTimer.restart();
        Serial.println(F("Damping paused for 15 minutes"));
    }
    request->send(200, "text/plain", "OK");
}

void handleData(AsyncWebServerRequest* request) {
    String json_ = "{";
    json_ += "\"rssi\":" + String(WiFi.RSSI());
    json_ += ",\"capacity\":" + String(tank.Capacity());

    // If damping is paused, show current value, otherwise show average
    float fillLevel_ = DampingPaused ? getCurrentFillLevel() : getAverageFillLevel();
    json_ += ",\"filledpercent\":" + String(fillLevel_, 0);
    json_ += ",\"status\":\"" + SensorStatus + "\"";

    // Damping status
    json_ += ",\"dampingPaused\":" + String(DampingPaused ? "true" : "false");

    if (DampingPaused) {
        unsigned long remaining_ = DampingPauseTimer.get() - DampingPauseTimer.getElapsed();
        int remainingMinutes_ = remaining_ / 60000;
        int remainingSeconds_ = (remaining_ % 60000) / 1000;
        json_ += ",\"dampingRemaining\":\"" + String(remainingMinutes_) + ":" +
            (remainingSeconds_ < 10 ? "0" : "") + String(remainingSeconds_) + "\"";
    }
    else {
        json_ += ",\"dampingRemaining\":\"\"";
    }

    json_ += "}";
    request->send(200, "application/json", json_);
}

void handlePost(AsyncWebServerRequest* request) {
    if (request->hasParam("reset", true)) {
        Serial.println(F("Resetting to defaults..."));

        Config.applyDefaultValue();
        tank.applyDefaultValue();
        sensorConfig.applyDefaultValue();

        iotWebConf.saveConfig();

        request->send(200, "text/plain", "OK");
    }
    else {
        request->send(400, "text/plain", "Bad Request");
    }
}

void handleReboot(AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response_ = request->beginResponse(200, "text/html",
        "<html>"
        "<head>"
        "<meta http-equiv=\"refresh\" content=\"15; url=/\">"
        "<title>Rebooting...</title>"
        "</head>"
        "<body>"
        "Please wait while the device is rebooting...<br>"
        "You will be redirected to the homepage shortly."
        "</body>"
        "</html>");
    request->client()->setNoDelay(true); // Disable Nagle's algorithm so the client gets the response immediately
    request->send(response_);
    ShouldReboot = true;
}

void handleRoot(AsyncWebServerRequest* request) {
    AsyncWebRequestWrapper asyncWebRequestWrapper(request);
    if (iotWebConf.handleCaptivePortal(&asyncWebRequestWrapper)) {
        return;
    }

    AsyncResponseStream* response_ = request->beginResponseStream("text/html");
    MyHtmlRootFormatProvider fp_;

    // === Create 3 Tabs ===
    fp_.addTab("Tank", "tank");
    fp_.addTab("Sensor", "sensor");
    fp_.addTab("System", "system");

    // Header
    response_->print(fp_.getHtmlHead(iotWebConf.getThingName()));
    response_->print(fp_.getIconLinks());
    response_->print(fp_.getHtmlStyle());
    response_->print(fp_.getHtmlHeadEnd());
    response_->print(fp_.getHtmlScript());
    response_->print(fp_.getHtmlTabButtons());

    // === TAB 1: Tank Status
    response_->print(fp_.getHtmlTabContentStart("tank"));
    response_->print(F("<fieldset align=left style=\"border: 1px solid\"><table border=\"0\" align=\"center\" width=\"100%\">"));
    response_->print(F("<tr><td align=\"left\">RSSI:</td><td align=\"right\"><span id=\"RSSIValue\">no data</span></td></tr></table></fieldset>"));

    response_->print(fp_.getHtmlFieldset(String(tank.TypeName()) + " tank"));
    response_->print(fp_.getHtmlTable());
    response_->print(fp_.getHtmlTableRow());
    response_->print(fp_.getHtmlTableCol());
    response_->print(F("<progress id=\"tankValue\" style=\"height:50px\" max=\"100\" value=\"0\">0%</progress>\n"));
    response_->print(fp_.getHtmlTableColEnd());
    response_->print(fp_.getHtmlTableRowEnd());
    response_->print(F("<tr><td align=left>Volume: </td><td><span id='capacityValue'>0</span>l</td></tr>"));
    response_->print(F("<tr><td align=left>Filled: </td><td><span id='filledpercentValue'>0</span>%</td></tr>"));
    response_->print(F("<tr><td align=left>Sensor Status:</td><td><span id='statusValue'>no response</span></td></tr>"));
    response_->print(fp_.getHtmlTableEnd());
    response_->print(fp_.getHtmlFieldsetEnd());
    response_->print(fp_.getHtmlTabContentEnd());

    // === TAB 2: Sensor Damping Control
    response_->print(fp_.getHtmlTabContentStart("sensor"));
    response_->print(fp_.getHtmlFieldset("Sensor damping"));
    response_->print(fp_.getHtmlTable());
    response_->print(F("<tr><td align=left>Status:</td><td><span id='dampingStatus'>Damping active</span></td></tr>"));
    response_->print(F("<tr><td colspan='2' align='center'><button id='dampingBtn' class='button' onclick='toggleDamping()'>Pause damping</button></td></tr>"));
    response_->print(F("<tr><td colspan='2' align='left' style='font-size:12px; color:#666;'>Damping will be paused for 15 minutes or can be manually reactivated.</td></tr>"));
    response_->print(fp_.getHtmlTableEnd());
    response_->print(fp_.getHtmlFieldsetEnd());
    response_->print(fp_.getHtmlTabContentEnd());

    // === TAB 3: System Info
    response_->print(fp_.getHtmlTabContentStart("system"));
    response_->print(fp_.getHtmlFieldset("Network"));
    response_->print(fp_.getHtmlTable());

    uint64_t chipid_ = ESP.getEfuseMac();
    char macStr_[18];
    snprintf(macStr_, sizeof(macStr_), "%02X:%02X:%02X:%02X:%02X:%02X",
        (uint8_t)(chipid_ >> 40), (uint8_t)(chipid_ >> 32),
        (uint8_t)(chipid_ >> 24), (uint8_t)(chipid_ >> 16),
        (uint8_t)(chipid_ >> 8), (uint8_t)(chipid_));
    response_->print(fp_.getHtmlTableRowText("MAC Address:", String(macStr_)));

    String ipAddress_;
    if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
        ipAddress_ = WiFi.softAPIP().toString();
    }
    else {
        ipAddress_ = WiFi.localIP().toString();
    }
    if (ipAddress_ == "0.0.0.0" || ipAddress_.isEmpty()) {
        ipAddress_ = "Not connected";
    }
    response_->print(fp_.getHtmlTableRowText("IP Address:", ipAddress_));

    response_->print(fp_.getHtmlTableEnd());
    response_->print(fp_.getHtmlFieldsetEnd());
    response_->print(fp_.getHtmlTabContentEnd());

    // Footer
    response_->print(fp_.addNewLine(2));
    response_->print(fp_.getHtmlTable());
    response_->print(fp_.getHtmlTableRowText("Go to <a href='config'>configure page</a> to change configuration."));
    response_->print(fp_.getHtmlTableRowText("Go to <a href='webserial'>sensor monitoring</a> page."));
    response_->print(fp_.getHtmlTableRowText(fp_.getHtmlVersion(Version)));
    response_->print(fp_.getHtmlTableEnd());

    response_->print(fp_.getBodyEnd());
    response_->print(fp_.getHtmlEnd());

    response_->addHeader("Server", "ESP Async Web Server");
    request->send(response_);
}

void convertParams() {
    APModeOfflineTime = atoi(APModeOfflineValue);
    ArduinoOTA.setHostname(iotWebConf.getThingName());
}

void configSaved() {
    convertParams();
    ParamsChanged = true;
}
