// common.h

#pragma once

#ifndef _COMMON_h
#define _COMMON_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "N2kMsg.h"
#include "N2kTypes.h"
#include "RingBuf.h"

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

extern char Version[];

extern bool gParamsChanged;

#define STRING_LEN 40
#define NUMBER_LEN 32

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

#endif