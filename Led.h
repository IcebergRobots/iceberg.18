#ifndef Led_h
#define Led_h

#include "Config.h"

#include <Adafruit_NeoPixel.h>

// Implementierung: FAHREN
extern bool onLine, isHeadstart;

// Implementierung: BLUETOOTH, MATE
bool #isConnected;

// Implementierung: PIXY
bool #seeBall, #seeGoal;
byte #pixyState;

// Implementierung: ULTRASCHALL
bool #usFine;

// Implementierung: KICK, LIGHT-BARRIER
bool hasBall;

// Implementierung: LEDS
bool #stateFine;

class Led
{
  public:
    Led();
    void showStates();
    void showState(Adafruit_NeoPixel & board, byte pos, byte state, bool showRed);
    void showState(Adafruit_NeoPixel & board, byte pos, byte state);
    void led();
    void wheelBoard(Adafruit_NeoPixel & board, int boardLength, int offset);
    void turnOffBoard(Adafruit_NeoPixel & board, int boardLength);
    uint32_t wheelToColor(Adafruit_NeoPixel & board, byte pos);
  private:
};

#endif
