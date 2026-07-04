// webhandling.h

#ifndef _WEBHANDLING_h
#define _WEBHANDLING_h

#include <Arduino.h>
#include <IotWebConfAsync.h>
#include <WebSerial.h>

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "123456789";

extern void wifiInit();
extern void wifiLoop();

static WiFiClient wifiClient;
extern AsyncIotWebConf iotWebConf;

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

class Tank : public iotwebconf::ParameterGroup {
public:
    Tank(const char* id, const char* name)
        : ParameterGroup(id, name),
        _CapacityParam("Capacity (l)", _capacityId, _capacityValue, NUMBER_LEN, "150", "1..1000", "min='1' max='1000' step='1'"),
        _FluidTypeParam(
            "Fluid type", _fluidTypeId, _fluidTypeValue, sizeof(FluidValues[0]),
            (char*)FluidValues, (char*)FluidNames,
            sizeof(FluidValues) / sizeof(FluidValues[0]), sizeof(FluidNames[0]),
            FluidNames[0])
    {
        snprintf(_capacityId, STRING_LEN, "%s-capacity", this->getId());
        snprintf(_fluidTypeId, STRING_LEN, "%s-fluidtype", this->getId());

        addItem(&_CapacityParam);
        addItem(&_FluidTypeParam);
    }

    uint16_t Capacity() const { return static_cast<uint16_t>(atoi(_capacityValue)); }
    tN2kFluidType Type() const { return static_cast<tN2kFluidType>(atoi(_fluidTypeValue)); }
    const char* TypeName() const { return FluidNames[Type()]; }

    void resetToDefaults() {
        _CapacityParam.applyDefaultValue();
        _FluidTypeParam.applyDefaultValue();
    }

private:
    char _capacityId[STRING_LEN];
    char _fluidTypeId[STRING_LEN];

    char _capacityValue[NUMBER_LEN]{};
    char _fluidTypeValue[sizeof(FluidValues[0])]{};

    iotwebconf::NumberParameter _CapacityParam;
    iotwebconf::SelectParameter _FluidTypeParam;
};


class SensorConfig : public iotwebconf::ParameterGroup {
public:
    SensorConfig() : ParameterGroup("sensorconfig", "Sensor Calibration") {
        snprintf(_resistanceFullId, STRING_LEN, "%s-rfull", this->getId());
        snprintf(_resistanceEmptyId, STRING_LEN, "%s-rempty", this->getId());

        this->addItem(&_ResistanceFullParam);
        this->addItem(&_ResistanceEmptyParam);
    }

    float getResistanceFull() const {
        float value = atof(_resistanceFullValue);
        // Validate range
        if (value < 1.0f || value > 500.0f) {
            return 33.0f; // Default fallback
        }
        return value;
    }

    float getResistanceEmpty() const {
        float value = atof(_resistanceEmptyValue);
        // Validate range
        if (value < 1.0f || value > 500.0f) {
            return 205.0f; // Default fallback
        }
        return value;
    }

    void resetToDefaults() {
        _ResistanceFullParam.applyDefaultValue();
        _ResistanceEmptyParam.applyDefaultValue();
    }

private:
    char _resistanceFullId[STRING_LEN];
    char _resistanceEmptyId[STRING_LEN];

    char _resistanceFullValue[NUMBER_LEN]{};
    char _resistanceEmptyValue[NUMBER_LEN]{};

    iotwebconf::NumberParameter _ResistanceFullParam =
        iotwebconf::NumberParameter(
            "Resistance Full (Ohm)",
            _resistanceFullId,
            _resistanceFullValue,
            NUMBER_LEN,
            "33.0",
            "1..500",
            "min='1' max='500' step='0.1'"
        );

    iotwebconf::NumberParameter _ResistanceEmptyParam =
        iotwebconf::NumberParameter(
            "Resistance Empty (Ohm)",
            _resistanceEmptyId,
            _resistanceEmptyValue,
            NUMBER_LEN,
            "240.0",
            "1..500",
            "min='1' max='500' step='0.1'"
        );
};


extern NMEAConfig Config;
extern Tank tank;
extern SensorConfig sensorConfig;

#endif

