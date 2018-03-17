#ifndef Utility_h
#define Utility_h

#include "Arduino.h"
#include "Config.h"
#include "Pin.h"
#include "Pilot.h"
#include "Mate.h"
//#include "Utility.h"

#include <Adafruit_NeoPixel.h>

void startSound();
void showLed(Adafruit_NeoPixel & board, byte pos, byte state, bool showRed);
void showLed(Adafruit_NeoPixel & board, byte pos, byte state);
void wheelBoard(Adafruit_NeoPixel & board, int boardLength, int offset);
void turnOffBoard(Adafruit_NeoPixel & board, int boardLength);
uint32_t wheelToColor(Adafruit_NeoPixel & board, byte pos);

#endif
