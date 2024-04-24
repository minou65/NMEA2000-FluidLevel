// webhandling.h

#ifndef _WEBHANDLING_h
#define _WEBHANDLING_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#if ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>      
#endif

#include <IotWebConf.h>


#define HTML_Start_Doc "<!DOCTYPE html>\
    <html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>\
    <title>{v}</title>\
";

#define HTML_Start_Body "</head><body>";
#define HTML_Start_Fieldset "<fieldset align=left style=\"border: 1px solid\">";
#define HTML_Start_Table "<table border=0 align=center>";
#define HTML_End_Table "</table>";
#define HTML_End_Fieldset "</fieldset>";
#define HTML_End_Body "</body>";
#define HTML_End_Doc "</html>";
#define HTML_Fieldset_Legend "<legend>{l}</legend>"
#define HTML_Table_Row "<tr><td>{n}</td><td>{v}</td></tr>";
#define HTML_JAVA_Script \
"<script>\n \
    // get intial data straight away \n \
    requestData(); \n \
\n \
    // request data updates every milliseconds\n \
    setInterval(requestData, 5000);\n \
\n \
    function requestData() {\n \
		var xhttp = new XMLHttpRequest();\n \
		xhttp.onreadystatechange = function() {\n \
			if (this.readyState == 4 && this.status == 200) {\n \
				var json = JSON.parse(this.responseText);\n \
				updateData(json);\n \
			}\n \
		};\n \
        xhttp.open('GET', 'data', true);\n \
		xhttp.send();\n \
	}\n \
\n \
    function updateData(jsonData) { \n \
		const tankelement = document.getElementById('tankValue');\n \
        tankelement.value = jsonData.filledpercent;\n \
        tankelement.innerHTML = jsonData.filledpercent;\n \
        document.getElementById('capacityValue').innerHTML = jsonData.capacity;\n \
		document.getElementById('filledpercentValue').innerHTML = jsonData.filledpercent;\n \
		document.getElementById('statusValue').innerHTML = jsonData.status;\n \
	}\n \
</script>\
";
// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "123456789";

extern void wifiInit();
extern void wifiLoop();

static WiFiClient wifiClient;
extern IotWebConf iotWebConf;

class NMEAConfig : public iotwebconf::ParameterGroup {
public:
    NMEAConfig() : ParameterGroup("nmeaconfig", "NMEA configuration") {
        snprintf(instanceID, STRING_LEN, "%s-instance", this->getId());
        snprintf(sidID, STRING_LEN, "%s-sid", this->getId());
        snprintf(sourceID, STRING_LEN, "%s-source", this->getId());

        this->addItem(&this->InstanceParam);
        this->addItem(&this->SIDParam);

        iotWebConf.addHiddenParameter(&SourceParam);

    }

    uint8_t Instance() { return atoi(InstanceValue); };
    uint8_t SID() { return atoi(SIDValue); };
    uint8_t Source() { return atoi(SourceValue); };

    void SetSource(uint8_t source_) {
        String s;
        s = (String)source_;
        strncpy(SourceParam.valueBuffer, s.c_str(), NUMBER_LEN);
    }


private:
    iotwebconf::NumberParameter InstanceParam = iotwebconf::NumberParameter("Instance", instanceID, InstanceValue, NUMBER_LEN, "255", "1..255", "min='1' max='254' step='1'");
    iotwebconf::NumberParameter SIDParam = iotwebconf::NumberParameter("SID", sidID, SIDValue, NUMBER_LEN, "255", "1..255", "min='1' max='255' step='1'");
    iotwebconf::NumberParameter SourceParam = iotwebconf::NumberParameter("Source", sourceID, SourceValue, NUMBER_LEN, "22", nullptr, nullptr);

    char InstanceValue[NUMBER_LEN];
    char SIDValue[NUMBER_LEN];
    char SourceValue[NUMBER_LEN];


    char instanceID[STRING_LEN];
    char sidID[STRING_LEN];
    char sourceID[STRING_LEN];
};

extern NMEAConfig Config;

#endif

