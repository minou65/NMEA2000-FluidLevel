// webhandling.h

#ifndef _WEBHANDLING_h
#define _WEBHANDLING_h

#include <Arduino.h>
#include <IotWebConf.h>
#include <WebSerial.h>

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

class Tank : public iotwebconf::ParameterGroup {
public:
    Tank(const char* id, const char* name)
        : ParameterGroup(id, name),
        _CapacityParam("Capacity (l)", _capacityId, _capacityValue, NUMBER_LEN, "150", "1..1000", "min='1' max='1000' step='1'"),
        _HeightParam("Height (mm)", _heightId, _heightValue, NUMBER_LEN, "1000", "1..2000", "min='1' max='2000' step='1'"),
        _FluidTypeParam(
            "Fluid type", _fluidTypeId, _fluidTypeValue, sizeof(FluidValues[0]),
            (char*)FluidValues, (char*)FluidNames,
            sizeof(FluidValues) / sizeof(FluidValues[0]), sizeof(FluidNames[0]),
            FluidNames[0])
    {
        snprintf(_capacityId, STRING_LEN, "%s-capacity", this->getId());
        snprintf(_heightId, STRING_LEN, "%s-height", this->getId());
        snprintf(_fluidTypeId, STRING_LEN, "%s-fluidtype", this->getId());

        addItem(&_CapacityParam);
        addItem(&_HeightParam);
        addItem(&_FluidTypeParam);
    }

    uint16_t getCapacity() const { return static_cast<uint16_t>(atoi(_capacityValue)); }
    uint16_t getHeight() const { return static_cast<uint16_t>(atoi(_heightValue)); }
    uint8_t getFluidType() const { return static_cast<uint8_t>(atoi(_fluidTypeValue)); }
    const char* getFluidTypeName() const { return FluidNames[getFluidType()]; }

    void resetToDefaults() {
        _CapacityParam.applyDefaultValue();
        _HeightParam.applyDefaultValue();
        _FluidTypeParam.applyDefaultValue();
    }

private:
    char _capacityId[STRING_LEN];
    char _heightId[STRING_LEN];
    char _fluidTypeId[STRING_LEN];

    char _capacityValue[NUMBER_LEN]{};
    char _heightValue[NUMBER_LEN]{};
    char _fluidTypeValue[sizeof(FluidValues[0])]{};

    iotwebconf::NumberParameter _CapacityParam;
    iotwebconf::NumberParameter _HeightParam;
    iotwebconf::SelectParameter _FluidTypeParam;
};

class Sensor : public iotwebconf::ParameterGroup {
public:
    Sensor(const char* id, const char* name)
        : ParameterGroup(id, name),
        _CalibrationFactorParam("Calibration factor", _calibrationFactorId, _calibrationFactorValue, NUMBER_LEN, "1.0000", "e.g. 1.00001", "step='0.00001'"),
        _DeadzoneUpperParam("Sensor blind zone at top (mm)", _deadzoneUpperId, _deadzoneUpperValue, NUMBER_LEN, "0", "e.g. 1", "step='1'"),
        _DeadzoneLowerParam("Sensor blind zone at bottom (mm)", _deadzoneLowerId, _deadzoneLowerValue, NUMBER_LEN, "0", "e.g. 1", "step='1'")
    {
        snprintf(_calibrationFactorId, STRING_LEN, "%s-calibration", this->getId());
        snprintf(_deadzoneUpperId, STRING_LEN, "%s-deadzoneupper", this->getId());
        snprintf(_deadzoneLowerId, STRING_LEN, "%s-deadzonelower", this->getId());

        addItem(&_CalibrationFactorParam);
        addItem(&_DeadzoneUpperParam);
        addItem(&_DeadzoneLowerParam);

		_CalibrationFactorParam.visible = false; // hide calibration factor by default
    }

    float getCalibrationFactor() const { return atof(_calibrationFactorValue); }
    uint8_t getDeadzoneUpper() const { return static_cast<uint8_t>(atoi(_deadzoneUpperValue)); }
    uint8_t getDeadzoneLower() const { return static_cast<uint8_t>(atoi(_deadzoneLowerValue)); }

    void resetToDefaults() {
        _CalibrationFactorParam.applyDefaultValue();
        _DeadzoneUpperParam.applyDefaultValue();
        _DeadzoneLowerParam.applyDefaultValue();
    }

private:
    char _calibrationFactorId[STRING_LEN];
    char _deadzoneUpperId[STRING_LEN];
    char _deadzoneLowerId[STRING_LEN];

    char _calibrationFactorValue[NUMBER_LEN]{};
    char _deadzoneUpperValue[NUMBER_LEN]{};
    char _deadzoneLowerValue[NUMBER_LEN]{};

    iotwebconf::NumberParameter _CalibrationFactorParam;
    iotwebconf::NumberParameter _DeadzoneUpperParam;
    iotwebconf::NumberParameter _DeadzoneLowerParam;
};

extern NMEAConfig Config;
extern Tank tank;
extern Sensor sensor;

#endif

