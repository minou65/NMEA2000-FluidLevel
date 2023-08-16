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
#include <IotWebConfUsing.h> // This loads aliases for easier class names.
#include <IotWebConfTParameter.h>
#include "common.h"
#include "webhandling.h"

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "A1"

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
const char thingName[] = "NMEA-FluidMonitor";

// -- Method declarations.
void handleRoot();
void convertParams();

// -- Callback methods.
void configSaved();
void wifiConnected();

bool gParamsChanged = true;

DNSServer dnsServer;
WebServer server(80);

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);

IotWebConfParameterGroup sysConfGroup = IotWebConfParameterGroup("SysConf", "Tank");

iotwebconf::UIntTParameter<uint16_t> TankCapacity =
    iotwebconf::Builder<iotwebconf::UIntTParameter<uint16_t>>("TankCapacity").
    label("Capacity").
    defaultValue(150).
    min(1u).
    step(1).
    placeholder("1..1000").
    build();

iotwebconf::UIntTParameter<uint16_t> TankHeight =
    iotwebconf::Builder<iotwebconf::UIntTParameter<uint16_t>>("TankHeight").
    label("Height").
    defaultValue(150).
    min(1u).
    step(1).
    placeholder("1..1000").
    build();


#define STRING_LEN 128

static char FluidValues[][STRING_LEN] = {
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6"
};
static char FluidNames[][STRING_LEN] = { 
    "Fuel", 
    "Water", 
    "Gray water", 
    "Live well", 
    "Oil", 
    "Black water", 
    "Gasoline fuel"
};


char FluidTypeValue[STRING_LEN];
IotWebConfSelectParameter FluidType = IotWebConfSelectParameter("Fluid type",
    "FluidType", 
    FluidTypeValue, 
    STRING_LEN, 
    (char*)FluidValues, 
    (char*)FluidNames, 
    sizeof(FluidValues) / STRING_LEN,
    STRING_LEN);



void wifiInit() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("starting up...");


    iotWebConf.setStatusPin(STATUS_PIN, ON_LEVEL);
    iotWebConf.setConfigPin(CONFIG_PIN);

    sysConfGroup.addItem(&TankCapacity);
    sysConfGroup.addItem(&TankHeight);
    sysConfGroup.addItem(&FluidType);

    iotWebConf.addParameterGroup(&sysConfGroup);

    iotWebConf.setConfigSavedCallback(&configSaved);
    iotWebConf.setWifiConnectionCallback(&wifiConnected);

    iotWebConf.getApTimeoutParameter()->visible = true;

    // -- Initializing the configuration.
    iotWebConf.init();

    convertParams();

    // -- Set up required URL handlers on the web server.
    server.on("/", handleRoot);
    server.on("/config", [] { iotWebConf.handleConfig(); });
    server.onNotFound([]() { iotWebConf.handleNotFound(); });

    Serial.println("Ready.");
}

void wifiLoop() {
    // -- doLoop should be called as frequently as possible.
    iotWebConf.doLoop();
    ArduinoOTA.handle();
}

void wifiConnected() {
    ArduinoOTA.begin();
}

void handleRoot() {
    // -- Let IotWebConf test and handle captive portal requests.
    if (iotWebConf.handleCaptivePortal())
    {
        // -- Captive portal request were already served.
        return;
    }

    String Type = "";
    switch(gFluidType) {
        case N2kft_Fuel:
            Type = "Fuel";
            break;
        case N2kft_Water:
            Type = "Water";
            break;
        case N2kft_GrayWater:
            Type = "Gray water";
            break;
        case N2kft_LiveWell:
            Type = "Live well";
            break;
        case N2kft_Oil:
            Type = "Oil";
            break;
        case N2kft_BlackWater:
            Type = "Black water";
            break;
        case N2kft_FuelGasoline:
            Type = "Fule gasoline";
            break;
        default:
            Type = "unknown";
    }

    String page = HTML_Start_Doc;
    
    page.replace("{v}", iotWebConf.getThingName());
    page += "<style>";
    page += ".de{background-color:#ffaaaa;} .em{font-size:0.8em;color:#bb0000;padding-bottom:0px;} .c{text-align: center;} div,input,select{padding:5px;font-size:1em;} input{width:95%;} select{width:100%} input[type=checkbox]{width:auto;scale:1.5;margin:10px;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#16A1E7;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} fieldset{border-radius:0.3rem;margin: 0px;}";
    // page.replace("center", "left");
    page += ".dot-grey{height: 12px; width: 12px; background-color: #bbb; border-radius: 50%; display: inline-block; }";
    page += ".dot-green{height: 12px; width: 12px; background-color: green; border-radius: 50%; display: inline-block; }";

    page += "</style>";

    page += "<meta http-equiv=refresh content=30 />";
    page += HTML_Start_Body;
    page += "<table border=0 align=center>";
    page += "<tr><td>";

    page += HTML_Start_Fieldset;
    page += HTML_Fieldset_Legend;
    String Title = Type + " tank";
    page.replace("{l}", Title);
    page += HTML_Start_Table;
        page += "<tr><td align=left>Filled: </td><td>";
        page += "<progress id=tank style=height:50px max=100 value=" + String(gTankPercentFilled) + ">" + String(gTankPercentFilled) + "%</progress>";
        page += "</td></tr>";

        page += "<tr><td align=left>Volume: </td><td>" + String(gFluidCapacity) + " l" + "</td></tr>";
        page += "<tr><td align=left>Sensor Status:</td><td>" + String(gStatusSensor) + "</td></tr>";


    page += HTML_End_Table;
    page += HTML_End_Fieldset;

    page += "<br>";
    page += "<br>";

    page += HTML_Start_Table;
    page += "<tr><td align=left>Go to <a href = 'config'>configure page</a> to change configuration.</td></tr>";
    // page += "<tr><td align=left>Go to <a href='setruntime'>runtime modification page</a> to change runtime data.</td></tr>";
    page += HTML_End_Table;
    page += HTML_End_Body;

    page += HTML_End_Doc;


    server.send(200, "text/html", page);


}

void convertParams() {
    gFluidCapacity = TankCapacity.value();
    gFluidType = tN2kFluidType(atoi(FluidTypeValue));
    gTankHeight = TankHeight.value();
}

void configSaved() {
    convertParams();
    gParamsChanged = true;
}