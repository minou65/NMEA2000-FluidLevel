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

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "A5"

// -- When CONFIG_PIN is pulled to ground on startup, the Thing will use the initial
//      password to buld an AP. (E.g. in case of lost password)
#define CONFIG_PIN  -1

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
    _response += "\"capacity\":\"" + String(gTankCapacity) + "\",";
    _response += "\"filledpercent\":\"" + String(gTankFilledPercent) + "\",";
    //_response += "\"filledpercent\":\"" + String(random(0, 100)) +"\",";
    _response += "\"status\":\"" + String(gStatusSensor) + "\"";
    _response += "}";
	server.send(200, "text/plain", _response);
}

void handleRoot() {
    // -- Let IotWebConf test and handle captive portal requests.
    if (iotWebConf.handleCaptivePortal())
    {
        // -- Captive portal request were already served.
        return;
    }

    String Type = FluidNames[gFluidType];

    String page = HTML_Start_Doc;
    
    page.replace("{v}", iotWebConf.getThingName());
    page += "<style>";
    page += ".de{background-color:#ffaaaa;} .em{font-size:0.8em;color:#bb0000;padding-bottom:0px;} .c{text-align: center;} div,input,select{padding:5px;font-size:1em;} input{width:95%;} select{width:100%} input[type=checkbox]{width:auto;scale:1.5;margin:10px;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#16A1E7;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} fieldset{border-radius:0.3rem;margin: 0px;}";
    // page.replace("center", "left");
    page += ".dot-grey{height: 12px; width: 12px; background-color: #bbb; border-radius: 50%; display: inline-block; }";
    page += ".dot-green{height: 12px; width: 12px; background-color: green; border-radius: 50%; display: inline-block; }";
    page += ".tank-lightgrey{color:#000; background-color:#f1f1f1}";
    page += ".tank-green{color: #000; background-color:#ddffdd}";
    page += ".tank-blue{color:#fff; background-color:#2196F3}";
    page += ".round{border-radius:32px}";

    page += "</style>";

    page += HTML_Start_Body;
    page += HTML_JAVA_Script
    page += "<table border=0 align=center>";
    page += "<tr><td>";

    page += HTML_Start_Fieldset;
    page += HTML_Fieldset_Legend;
    String Title = Type + " tank";
    page.replace("{l}", Title);
    page += HTML_Start_Table;
        page += "<tr><td align=left>Level: </td><td>";
        //page += "<progress id=tank style=height:50px max=100 value=" + String(gTankFilledPercent) + ">" + String(gTankFilledPercent) + "%</progress>";
        page += "<progress id=tankValue style=height:50px max=100 value='0'>0%</progress>";
        page += "</td></tr>";

        page += "<tr><td align=left>Volume: </td><td><span id='capacityValue'>0</span>l</td></tr>";
        page += "<tr><td align=left>Filled: </td><td><span id='filledpercentValue'>0</span>%</td></tr>";
        page += "<tr><td align=left>Sensor Status:</td><td><span id='statusValue'>no response</span></td></tr>";

    page += HTML_End_Table;
    page += HTML_End_Fieldset;

    page += HTML_Start_Fieldset;
    page += HTML_Fieldset_Legend;
    page.replace("{l}", "Network");
    page += HTML_Start_Table;

    page += "<tr><td align=left>MAC Address:</td><td>" + String(WiFi.macAddress()) + "</td></tr>";
    page += "<tr><td align=left>IP Address:</td><td>" + String(WiFi.localIP().toString().c_str()) + "</td></tr>";

    page += HTML_End_Table;
    page += HTML_End_Fieldset;

    page += "<br>";
    page += "<br>";

    page += HTML_Start_Table;
    page += "<tr><td align=left>Go to <a href = 'config'>configure page</a> to change configuration.</td></tr>";
    // page += "<tr><td align=left>Go to <a href='setruntime'>runtime modification page</a> to change runtime data.</td></tr>";
    page += "<tr><td><font size=1>Version: " + String(Version) + "</font></td></tr>";
    page += HTML_End_Table;
    page += HTML_End_Body;

    page += HTML_End_Doc;


    server.send(200, "text/html", page);


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