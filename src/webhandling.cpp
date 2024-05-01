// 
// 
// 

#define DEBUG_WIFI(m) SERIAL_DBG.print(m)

#include <Arduino.h>
#include <ArduinoOTA.h>
#if ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>      
#endif

#include <time.h>
//needed for library

#include <DNSServer.h>
#include <iostream>
#include <string.h>

#include <IotWebConf.h>
#include <IotWebConfTParameter.h>
#include <IotWebConfESP32HTTPUpdateServer.h>
#include "common.h"
#include "webhandling.h"
#include "IotWebRoot.h"

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "A6"

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

// -- Method declarations.
void handleData();
void handleRoot();
void convertParams();

// -- Callback methods.
void configSaved();
void wifiConnected();

bool gParamsChanged = true;

DNSServer dnsServer;
WebServer server(80);
HTTPUpdateServer httpUpdater;

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);

NMEAConfig Config = NMEAConfig();

iotwebconf::ParameterGroup TankGroup = iotwebconf::ParameterGroup("TankGroup", "Tank");
iotwebconf::ParameterGroup CalibrationGroup = iotwebconf::ParameterGroup("CalibrationGroup", "Sensor");

iotwebconf::UIntTParameter<uint16_t> TankCapacity =
    iotwebconf::Builder<iotwebconf::UIntTParameter<uint16_t>>("TankCapacity").
    label("Capacity (l)").
    defaultValue(150).
    min(1u).
    step(1).
    placeholder("1..1000").
    build();

iotwebconf::UIntTParameter<uint16_t> TankHeight =
    iotwebconf::Builder<iotwebconf::UIntTParameter<uint16_t>>("TankHeight").
    label("Height (mm)").
    defaultValue(1000).
    min(1u).
    step(1).
    placeholder("1..2000").
    build();

char FluidTypeValue[STRING_LEN];
iotwebconf::SelectParameter FluidType = iotwebconf::SelectParameter(
    "Fluid type",
    "FluidType",
    FluidTypeValue,
    STRING_LEN,
    (char*)FluidValues,
    (char*)FluidNames,
    sizeof(FluidValues) / STRING_LEN,
    STRING_LEN,
    FluidNames[gFluidType]
);

char SensorCalibrationFactorValue[NUMBER_LEN];
iotwebconf::NumberParameter SensorCalibrationFactor = iotwebconf::NumberParameter(
    "Calibration factor", 
    "CalibrationFactor", 
    SensorCalibrationFactorValue, 
    NUMBER_LEN, 
    "1.0000", 
    "e.g. 1.00001", 
    "step='0.00001'"
);

char DeadzoneUpperValue[NUMBER_LEN];
iotwebconf::NumberParameter DeadzoneUpper = iotwebconf::NumberParameter(
    "Upper dead zone (mm)",
    "UpperDeadZone",
    DeadzoneUpperValue,
    NUMBER_LEN,
    "0",
    "e.g. 1",
    "step='1'"
);

char DeadzoneLowerValue[NUMBER_LEN];
iotwebconf::NumberParameter DeadzoneLower = iotwebconf::NumberParameter(
    "Lower dead zone (mm)",
    "LowerDeadZone",
    DeadzoneLowerValue,
    NUMBER_LEN,
    "0",
    "e.g. 1",
    "step='1'"
);

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

    // -- Define how to handle updateServer calls.
    iotWebConf.setupUpdateServer(
        [](const char* updatePath) { httpUpdater.setup(&server, updatePath); },
        [](const char* userName, char* password) { httpUpdater.updateCredentials(userName, password); });

    iotWebConf.setConfigSavedCallback(&configSaved);
    iotWebConf.setWifiConnectionCallback(&wifiConnected);

    iotWebConf.getApTimeoutParameter()->visible = true;

    // -- Initializing the configuration.
    iotWebConf.init();

    convertParams();

    // -- Set up required URL handlers on the web server.
    server.on("/", handleRoot);
    server.on("/config", [] { iotWebConf.handleConfig(); });
    server.on("/data", HTTP_GET, []() { handleData(); });
    server.onNotFound([]() { iotWebConf.handleNotFound(); });

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
}

void wifiConnected() {
    ArduinoOTA.begin();
}

void handleData() {
    String _response = "{";
    _response += "\"rssi\":\"" + String(WiFi.RSSI()) + "\",";
    _response += "\"capacity\":\"" + String(gTankCapacity) + "\",";
    _response += "\"filledpercent\":\"" + String(gTankFilledPercent) + "\",";
    //_response += "\"filledpercent\":\"" + String(random(0, 100)) +"\",";
    _response += "\"status\":\"" + String(gStatusSensor) + "\"";
    _response += "}";
	server.send(200, "text/plain", _response);
}


void handleRoot() {
    if (iotWebConf.handleCaptivePortal()){
        return;
    }

    MyHtmlRootFormatProvider rootFormatProvider;

    String _response = "";
    _response += rootFormatProvider.getHtmlHead(iotWebConf.getThingName());
    _response += rootFormatProvider.getHtmlStyle();
    _response += rootFormatProvider.getHtmlHeadEnd();
    _response += rootFormatProvider.getHtmlScript();

    _response += rootFormatProvider.getHtmlTable();
    _response += rootFormatProvider.getHtmlTableRow() + rootFormatProvider.getHtmlTableCol();

    _response += F("<fieldset align=left style=\"border: 1px solid\">\n");
    _response += F("<table border=\"0\" align=\"center\" width=\"100%\">\n");
    _response += F("<tr><td align=\"left\"> </td></td><td align=\"right\"><span id=\"RSSIValue\">no data</span></td></tr>\n");
    _response += rootFormatProvider.getHtmlTableEnd();
    _response += rootFormatProvider.getHtmlFieldsetEnd();

    _response += rootFormatProvider.getHtmlFieldset(String(FluidNames[gFluidType]) + " tank");
    _response += rootFormatProvider.getHtmlTable();
    _response += rootFormatProvider.getHtmlTableRow() + rootFormatProvider.getHtmlTableCol();
    _response += F("<progress id=\"tankValue\" style=\"height:50px\" max=\"100\" value=\"0\">0%</progress>\n");
    _response += rootFormatProvider.getHtmlTableColEnd() + rootFormatProvider.getHtmlTableRowEnd();
    _response += F("<tr><td align=left>Volume: </td><td><span id='capacityValue'>0</span>l</td></tr>");
    _response += F("<tr><td align=left>Filled: </td><td><span id='filledpercentValue'>0</span>%</td></tr>");
    _response += F("<tr><td align=left>Sensor Status:</td><td><span id='statusValue'>no response</span></td></tr>");
    _response += rootFormatProvider.getHtmlTableEnd();
    _response += rootFormatProvider.getHtmlFieldsetEnd();

    _response += rootFormatProvider.getHtmlFieldset("Network");
    _response += rootFormatProvider.getHtmlTable();
    _response += rootFormatProvider.getHtmlTableRowText("MAC Address:", WiFi.macAddress());
    _response += rootFormatProvider.getHtmlTableRowText("IP Address:", WiFi.localIP().toString().c_str());
    _response += rootFormatProvider.getHtmlTableEnd();
    _response += rootFormatProvider.getHtmlFieldsetEnd();

    _response += rootFormatProvider.addNewLine(2);

    _response += rootFormatProvider.getHtmlTable();
    _response += rootFormatProvider.getHtmlTableRowText("Go to <a href = 'config'>configure page</a> to change configuration.");
    _response += rootFormatProvider.getHtmlTableRowText(rootFormatProvider.getHtmlVersion(Version));
    _response += rootFormatProvider.getHtmlTableEnd();

    _response += rootFormatProvider.getHtmlTableColEnd() + rootFormatProvider.getHtmlTableRowEnd();
    _response += rootFormatProvider.getHtmlTableEnd();
    _response += rootFormatProvider.getHtmlEnd();

    server.send(200, "text/html", _response);
}

void convertParams() {
    gTankCapacity = TankCapacity.value();
    gFluidType = tN2kFluidType(atoi(FluidTypeValue));
    gTankHeight = TankHeight.value();
    gSensorCalibrationFactor = atof(SensorCalibrationFactorValue);
    gDeadzoneUpper = atoi(DeadzoneUpperValue);
    gDeadzoneLower = atoi(DeadzoneLowerValue);

    gN2KSource = Config.Source();

}

void configSaved() {
    convertParams();
    gParamsChanged = true;
}