
//#define DEBUG_WIFI(m) SERIAL_DBG.print(m)

#include <Arduino.h>
#include <ArduinoOTA.h>
#if defined(ESP32)
#include <WiFi.h>
// #include <esp_wifi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#error "Unsupported platform"
#endif

#include "common.h"
#include "webhandling.h"
#include "favicon.h"
#include "neotimer.h"

#include <DNSServer.h>
#include <IotWebConf.h>
#include <IotWebConfAsyncClass.h>
#include <IotWebConfAsyncUpdateServer.h>
#include <IotWebRoot.h>

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "A7"

// -- When CONFIG_PIN is pulled to ground on startup, the Thing will use the initial
//      password to buld an AP. (E.g. in case of lost password)
#define CONFIG_PIN  GPIO_NUM_13

// -- Status indicator pin.
//      First it will light up (kept LOW), on Wifi connection it will blink,
//      when connected to the Wifi it will turn off (kept HIGH).
#define STATUS_PIN LED_BUILTIN
#if ESP32 
#define ON_LEVEL HIGH
#else
#define ON_LEVEL LOW
#endif

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "NMEA2000-FluidLevel";

uint8_t APModeOfflineTime = 0;
Neotimer APModeTimer = Neotimer();

// -- Method declarations.
void handleData(AsyncWebServerRequest* request);
void handleRoot(AsyncWebServerRequest* request);
void convertParams();

// -- Callback methods.
void configSaved();
void wifiConnected();

bool gParamsChanged = true;

DNSServer dnsServer;
AsyncWebServer server(80);
AsyncWebServerWrapper asyncWebServerWrapper(&server);
AsyncUpdateServer AsyncUpdater;

IotWebConf iotWebConf(thingName, &dnsServer, &asyncWebServerWrapper, wifiInitialApPassword, CONFIG_VERSION);

char APModeOfflineValue[STRING_LEN];
iotwebconf::NumberParameter APModeOfflineParam = iotwebconf::NumberParameter("AP offline mode after (minutes)", "APModeOffline", APModeOfflineValue, NUMBER_LEN, "0", "0..30", "min='0' max='30', step='1'");

NMEAConfig Config = NMEAConfig();

iotwebconf::ParameterGroup TankGroup = iotwebconf::ParameterGroup("TankGroup", "Tank");
iotwebconf::ParameterGroup CalibrationGroup = iotwebconf::ParameterGroup("CalibrationGroup", "Sensor");

char FluidTypeValue[STRING_LEN];
char TankCapacityValue[NUMBER_LEN];
char TankHeightValue[NUMBER_LEN];
char SensorCalibrationFactorValue[NUMBER_LEN];
char DeadzoneUpperValue[NUMBER_LEN];
char DeadzoneLowerValue[NUMBER_LEN];

iotwebconf::NumberParameter TankCapacity = iotwebconf::NumberParameter("Capacity(l)", "TankCapacity", TankCapacityValue, NUMBER_LEN, "150", "1..1000", "min = '1' max = '1000' step = '1'");
iotwebconf::NumberParameter TankHeight = iotwebconf::NumberParameter("Height(mm)", "TankHeight", TankHeightValue, NUMBER_LEN, "1000", "1..2000", "min = '1' max = '2000' step = '1'");
iotwebconf::SelectParameter FluidType = iotwebconf::SelectParameter("Fluid type", "FluidType", FluidTypeValue, STRING_LEN, (char*)FluidValues, (char*)(FluidNames), sizeof(FluidValues) / STRING_LEN, STRING_LEN, FluidNames[gFluidType]);
iotwebconf::NumberParameter SensorCalibrationFactor = iotwebconf::NumberParameter("Calibration factor", "CalibrationFactor", SensorCalibrationFactorValue, NUMBER_LEN, "1.0000", "e.g. 1.00001", "step='0.00001'");
iotwebconf::NumberParameter DeadzoneUpper = iotwebconf::NumberParameter("Upper dead zone (mm)", "UpperDeadZone", DeadzoneUpperValue, NUMBER_LEN, "0", "e.g. 1", "step='1'");
iotwebconf::NumberParameter DeadzoneLower = iotwebconf::NumberParameter("Lower dead zone (mm)", "LowerDeadZone", DeadzoneLowerValue, NUMBER_LEN, "0", "e.g. 1", "step='1'");

class CustomHtmlFormatProvider : public iotwebconf::HtmlFormatProvider {
protected:
    virtual String getFormEnd() {
        String _s = HtmlFormatProvider::getFormEnd();
        _s += F("</br>Return to <a href='/'>home page</a>.");
        return _s;
    }
};
CustomHtmlFormatProvider customHtmlFormatProvider;

class MyHtmlRootFormatProvider : public HtmlRootFormatProvider {
protected:
    virtual String getScriptInner() {
        String _s = HtmlRootFormatProvider::getScriptInner();
        _s.replace("{millisecond}", "5000");
        _s += F("function updateData(jsonData) {\n");
        _s += F("   document.getElementById('RSSIValue').innerHTML = jsonData.rssi + \"dBm\" \n");

        _s += F("   const tankelement = document.getElementById('tankValue'); \n");
        _s += F("   tankelement.value = jsonData.filledpercent; \n");
        _s += F("   tankelement.innerHTML = jsonData.filledpercent; \n");
        _s += F("   document.getElementById('capacityValue').innerHTML = jsonData.capacity; \n");
        _s += F("   document.getElementById('filledpercentValue').innerHTML = jsonData.filledpercent; \n");
        _s += F("   document.getElementById('statusValue').innerHTML = jsonData.status; \n");
        _s += F("}\n");

        return _s;
    }
};


void wifiInit() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("starting up...");

    DeadzoneLower.defaultValue = "0";
    DeadzoneUpper.defaultValue = "0";
    SensorCalibrationFactor.defaultValue = "1.00";


    iotWebConf.setStatusPin(STATUS_PIN, ON_LEVEL);
    iotWebConf.setConfigPin(CONFIG_PIN);
    iotWebConf.setHtmlFormatProvider(&customHtmlFormatProvider);

    TankGroup.addItem(&FluidType);
    TankGroup.addItem(&TankCapacity);
    TankGroup.addItem(&TankHeight);
    

    CalibrationGroup.addItem(&SensorCalibrationFactor);
    CalibrationGroup.addItem(&DeadzoneUpper);
    CalibrationGroup.addItem(&DeadzoneLower);

    iotWebConf.addParameterGroup(&Config);
    iotWebConf.addParameterGroup(&TankGroup);
    iotWebConf.addParameterGroup(&CalibrationGroup);

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
        AsyncWebRequestWrapper asyncWebRequestWrapper(request);
        iotWebConf.handleConfig(&asyncWebRequestWrapper);
        }
    );
    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest* request) {
        AsyncWebServerResponse* response = request->beginResponse_P(200, "image/x-icon", favicon_ico, sizeof(favicon_ico));
        request->send(response);
        }
    );
    server.on("/data", HTTP_GET, [](AsyncWebServerRequest* request) { handleData(request); });
    server.onNotFound([](AsyncWebServerRequest* request) {
        AsyncWebRequestWrapper asyncWebRequestWrapper(request);
        iotWebConf.handleNotFound(&asyncWebRequestWrapper);
        }
    );

	WebSerial.begin(&server, "/webserial");

    if (APModeOfflineTime > 0) {
        APModeTimer.start(APModeOfflineTime * 60 * 1000);
    }

    Serial.println("Ready.");
}

void wifiLoop() {
    // -- doLoop should be called as frequently as possible.
    iotWebConf.doLoop();
    ArduinoOTA.handle();

    if (gSaveParams) {
        Serial.println(F("Parameters are changed,save them"));

        Config.SetSource(gN2KSource);

        iotWebConf.saveConfig();
        gSaveParams = false;
    }

    if (APModeTimer.done()) {
        Serial.println(F("AP mode offline time reached"));
        iotWebConf.goOffLine();
        APModeTimer.stop();
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

void handleData(AsyncWebServerRequest* request) {
	String json_ = "{";
	json_ += "\"rssi\":" + String(WiFi.RSSI());
	json_ += ",\"capacity\":" + String(gTankCapacity);
	json_ += ",\"filledpercent\":" + String(gTankFilledPercent);
	json_ += ",\"status\":\"" + gStatusSensor + "\"";
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

    content_ += fp_.getHtmlFieldset(String(FluidNames[gFluidType]) + " tank").c_str();
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
    gTankCapacity = atoi(TankCapacityValue);
    gFluidType = tN2kFluidType(atoi(FluidTypeValue));
    gTankHeight = atoi(TankHeightValue);
    gSensorCalibrationFactor = atof(SensorCalibrationFactorValue);
    gDeadzoneUpper = atoi(DeadzoneUpperValue);
    gDeadzoneLower = atoi(DeadzoneLowerValue);

    gN2KSource = Config.Source();

    APModeOfflineTime = atoi(APModeOfflineValue);

    ArduinoOTA.setHostname(iotWebConf.getThingName());

}

void configSaved() {
    convertParams();
    gParamsChanged = true;
}