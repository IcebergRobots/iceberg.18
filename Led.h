#ifndef Led_h
#define Led_h

#include "Config.h"

#include <Adafruit_NeoPixel.h>

// Implementierung: FAHREN
extern bool onLine, isHeadstart;

// Implementierung: BLUETOOTH, MATE
extern bool isConnected;

// Implementierung: PIXY
extern bool seeBall, seeGoal;
extern byte pixyState;

// Implementierung: ULTRASCHALL
extern bool usFine;

// Implementierung: KICK, LIGHT-BARRIER
extern bool hasBall;

// Implementierung: LEDS
extern bool stateFine;
extern unsigned int animationPos;
extern Adafruit_NeoPixel bottom;
extern Adafruit_NeoPixel matrix;
extern Adafruit_NeoPixel info;

class Led
{
  public:
    Led();
    
    void led();
    void showStates();
    void showState(Adafruit_NeoPixel & board, byte pos, byte state, bool showRed);
    void showState(Adafruit_NeoPixel & board, byte pos, byte state);
    void wheelBoard(Adafruit_NeoPixel & board, int boardLength, int offset);
    void turnOffBoard(Adafruit_NeoPixel & board, int boardLength);
    uint32_t wheelToColor(Adafruit_NeoPixel & board, byte pos);
  private:
};

#endif
