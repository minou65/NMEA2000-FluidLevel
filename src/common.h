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

#define TANK_RINGBUF_SIZE 30

extern String SensorStatus;
extern RingBuf<float, TANK_RINGBUF_SIZE> gAverageFillLevel;

extern bool DampingPaused;
extern class Neotimer DampingPauseTimer;

extern bool ParamsSave;
extern bool ParamsChanged;

extern char Version[];

extern float getAverageFillLevel();
extern float getCurrentFillLevel();

#define STRING_LEN 40
#define NUMBER_LEN 6

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