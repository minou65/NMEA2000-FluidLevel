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

extern uint16_t gFluidCapacity;

extern tN2kFluidType gFluidType;

extern String gStatusSensor;

extern uint16_t gTankHeight; // mm
extern uint16_t gTankfilled; // mm
extern RingBuf gAverageTankFilled;
extern uint8_t gTankPercentFilled; // %

extern uint8_t gN2KSource;

extern char Version[];

extern bool gParamsChanged;


#endif