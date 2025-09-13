// common.h

#pragma once

#ifndef _COMMON_h
#define _COMMON_h

#include "N2kMsg.h"
#include "N2kTypes.h"
#include "RingBuf.h"

extern bool debugMode;

#define DEBUG_PRINT(x) if (debugMode) Serial.print(x) 
#define DEBUG_PRINTLN(x) if (debugMode) Serial.println(x)
#define DEBUG_PRINTF(...) if (debugMode) Serial.printf(__VA_ARGS__)

extern tN2kFluidType gFluidType;

extern String gStatusSensor;

extern uint16_t gTankCapacity; // l
extern uint16_t gTankHeight; // mm
extern uint16_t gTankfilled; // mm
extern uint8_t gTankFilledPercent; // %
extern float gSensorCalibrationFactor;
extern uint8_t gDeadzoneUpper; //mm
extern uint8_t gDeadzoneLower; //mm

extern RingBuf<uint16_t, 30> gAverageTankFilled;


extern uint8_t gN2KSource;
extern bool gSaveParams;

extern char Version[];

extern bool gParamsChanged;

#define STRING_LEN 40
#define NUMBER_LEN 32

static char FluidValues[][3] = {
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6"
};
static char FluidNames[][15] = {
    "Fuel",
    "Water",
    "Gray water",
    "Live well",
    "Oil",
    "Black water",
    "Gasoline fuel"
};

#endif