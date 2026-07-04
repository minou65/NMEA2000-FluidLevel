#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>

#include "common.h"
#include "webhandling.h"
#include "favicon.h"
#include "neotimer.h"

#include <DNSServer.h>
#include <IotWebConfAsyncUpdateServer.h>
#include <IotWebRoot.h>
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

uint8_t APModeOfflineTime = 0;
Neotimer APModeTimer = Neotimer();

// Damping pause functionality
bool DampingPaused = false;
Neotimer DampingPauseTimer = Neotimer(15 * 60 * 1000); // 15 Minuten

// -- Method declarations.
void onWebSerialMessage(uint8_t* data, size_t len);
void handleData(AsyncWebServerRequest* request);
void handleRoot(AsyncWebServerRequest* request);
void handleToggleDamping(AsyncWebServerRequest* request);
void convertParams();

// -- Callback methods.
void configSaved();
void wifiConnected();

bool ParamsChanged = true;

DNSServer dnsServer;
AsyncWebServer server(80);
AsyncWebServerWrapper asyncWebServerWrapper(&server);
AsyncUpdateServer AsyncUpdater;

AsyncIotWebConf iotWebConf(thingName, &dnsServer, &asyncWebServerWrapper, wifiInitialApPassword, CONFIG_VERSION);

char APModeOfflineValue[STRING_LEN];
iotwebconf::NumberParameter APModeOfflineParam = iotwebconf::NumberParameter("AP offline mode after (minutes)", "APModeOffline", APModeOfflineValue, NUMBER_LEN, "0", "0..30", "min='0' max='30', step='1'");

NMEAConfig Config = NMEAConfig();
Tank tank = Tank("tank", "Tank configuration");
SensorConfig sensorConfig;

class CustomHtmlFormatProvider : public iotwebconf::HtmlFormatProvider {
protected:
    virtual String getFormEnd() {
        String s_ = HtmlFormatProvider::getFormEnd();
        s_ += F("</br>Return to <a href='/'>home page</a>.");
        return s_;
    }
};
CustomHtmlFormatProvider customHtmlFormatProvider;

class MyHtmlRootFormatProvider : public HtmlRootFormatProvider {
public:
    virtual String getScriptInner() {
        String s_ = HtmlRootFormatProvider::getScriptInner();
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
        String s_ = HtmlRootFormatProvider::getStyleInner();
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
    iotWebConf.setHtmlFormatProvider(&customHtmlFormatProvider);

    iotWebConf.addParameterGroup(&Config);
    iotWebConf.addParameterGroup(&tank);
    iotWebConf.addParameterGroup(&sensorConfig);

    iotWebConf.addSystemParameter(&APModeOfflineParam);

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

    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest* request) {
        AsyncWebServerResponse* response = request->beginResponse_P(200, "image/x-icon", favicon_ico, sizeof(favicon_ico));
        request->send(response);
        }
    );

    server.on("/apple-touch-icon.png", HTTP_GET, [](AsyncWebServerRequest* request) {
        AsyncWebServerResponse* response = request->beginResponse_P(200, "image/png", favicon_ico, sizeof(favicon_ico));
        request->send(response);
        }
    );

    server.on("/data", HTTP_GET, [](AsyncWebServerRequest* request) { handleData(request); });
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
        delay(1000);
        ESP.restart();
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
        WebSerial.println("Rebooting...");
        ESP.restart();
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

void handleRoot(AsyncWebServerRequest* request) {
    AsyncWebRequestWrapper asyncWebRequestWrapper(request);
    if (iotWebConf.handleCaptivePortal(&asyncWebRequestWrapper)) {
        return;
    }

    std::string content_;
    MyHtmlRootFormatProvider fp_;

    content_ += fp_.getHtmlHead(iotWebConf.getThingName()).c_str();
    content_ += fp_.getHtmlStyle().c_str();

    content_ += String(F("<link rel=\"icon\" type=\"image/png\" sizes=\"96x96\" href=\"/apple-touch-icon.png\">\n")).c_str();
    content_ += String(F("<link rel=\"apple-touch-icon\" sizes=\"96x96\" href=\"/apple-touch-icon.png\">\n")).c_str();

    content_ += fp_.getHtmlHeadEnd().c_str();
    content_ += fp_.getHtmlScript().c_str();

    content_ += fp_.getHtmlTable().c_str();
    content_ += fp_.getHtmlTableRow().c_str();
    content_ += fp_.getHtmlTableCol().c_str();

    content_ += String(F("<fieldset align=left style=\"border: 1px solid\">\n")).c_str();
    content_ += String(F("<table border=\"0\" align=\"center\" width=\"100%\">\n")).c_str();
    content_ += String(F("<tr><td align=\"left\"> </td></td><td align=\"right\"><span id=\"RSSIValue\">no data</span></td></tr>\n")).c_str();
    content_ += fp_.getHtmlTableEnd().c_str();
    content_ += fp_.getHtmlFieldsetEnd().c_str();

    content_ += fp_.getHtmlFieldset(String(tank.TypeName()) + " tank").c_str();
    content_ += fp_.getHtmlTable().c_str();
    content_ += fp_.getHtmlTableRow().c_str();
    content_ += fp_.getHtmlTableCol().c_str();
    content_ += String(F("<progress id=\"tankValue\" style=\"height:50px\" max=\"100\" value=\"0\">0%</progress>\n")).c_str();
    content_ += fp_.getHtmlTableColEnd().c_str();
    content_ += fp_.getHtmlTableRowEnd().c_str();
    content_ += String(F("<tr><td align=left>Volume: </td><td><span id='capacityValue'>0</span>l</td></tr>")).c_str();
    content_ += String(F("<tr><td align=left>Filled: </td><td><span id='filledpercentValue'>0</span>%</td></tr>")).c_str();
    content_ += String(F("<tr><td align=left>Sensor Status:</td><td><span id='statusValue'>no response</span></td></tr>")).c_str();
    content_ += fp_.getHtmlTableEnd().c_str();
    content_ += fp_.getHtmlFieldsetEnd().c_str();

    // Damping Control Section
    content_ += fp_.getHtmlFieldset("Sensor damping").c_str();
    content_ += fp_.getHtmlTable().c_str();
    content_ += String(F("<tr><td align=left>Status:</td><td><span id='dampingStatus'>Damping active</span></td></tr>")).c_str();
    content_ += String(F("<tr><td colspan='2' align='center'><button id='dampingBtn' class='button' onclick='toggleDamping()'>Pause damping</button></td></tr>")).c_str();
    content_ += String(F("<tr><td colspan='2' align='left' style='font-size:12px; color:#666;'>Damping will be paused for 15 minutes or can be manually reactivated.</td></tr>")).c_str();
    content_ += fp_.getHtmlTableEnd().c_str();
    content_ += fp_.getHtmlFieldsetEnd().c_str();

    content_ += fp_.getHtmlFieldset("Network").c_str();
    content_ += fp_.getHtmlTable().c_str();
    content_ += fp_.getHtmlTableRowText("MAC Address:", WiFi.macAddress()).c_str();
    content_ += fp_.getHtmlTableRowText("IP Address:", WiFi.localIP().toString().c_str()).c_str();
    content_ += fp_.getHtmlTableEnd().c_str();
    content_ += fp_.getHtmlFieldsetEnd().c_str();

    content_ += fp_.addNewLine(2).c_str();

    content_ += fp_.getHtmlTable().c_str();
    content_ += fp_.getHtmlTableRowText("Go to <a href = 'config'>configure page</a> to change configuration.").c_str();
    content_ += fp_.getHtmlTableRowText("Go to <a href = 'webserial'>sensor monitoring</a> page.").c_str();
    content_ += fp_.getHtmlTableRowText(fp_.getHtmlVersion(Version)).c_str();
    content_ += fp_.getHtmlTableEnd().c_str();

    content_ += fp_.getHtmlTableColEnd().c_str();
    content_ += fp_.getHtmlTableRowEnd().c_str();
    content_ += fp_.getHtmlTableEnd().c_str();
    content_ += fp_.getHtmlEnd().c_str();

    AsyncWebServerResponse* response = request->beginChunkedResponse("text/html", [content_](uint8_t* buffer, size_t maxLen, size_t index) -> size_t {

        std::string chunk_ = "";
        size_t len_ = min(content_.length() - index, maxLen);
        if (len_ > 0) {
            chunk_ = content_.substr(index, len_ - 1);
            chunk_.copy((char*)buffer, chunk_.length());
        }
        if (index + len_ <= content_.length())
            return chunk_.length();
        else
            return 0;

        });
    response->addHeader("Server", "ESP Async Web Server");
    request->send(response);
}

void convertParams() {
    APModeOfflineTime = atoi(APModeOfflineValue);
    ArduinoOTA.setHostname(iotWebConf.getThingName());
}

void configSaved() {
    convertParams();
    ParamsChanged = true;
}