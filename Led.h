#ifndef Led_h
#define Led_h

#include "Config.h"

#include <Adafruit_NeoPixel.h>

// Implementierung: FAHREN
extern bool onLine, isHeadstart;

// Implementierung: PIXY
bool driftLeft, isDrift, seeBall, seeGoal;
unsigned long seeBallTimer, seeGoalTimer, driftTimer, ballLeftTimer, ballRightTimer, pixyResponseTimer, pixyTimer;

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
