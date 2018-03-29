#ifndef Led_h
#define Led_h

#include "Config.h"

#include <Adafruit_NeoPixel.h>

class Led
{
  public:
    Led();

    void led();
    void set();
  private:
    void showState(Adafruit_NeoPixel & board, byte pos, byte state, bool showRed);
    void showState(Adafruit_NeoPixel & board, byte pos, byte state);
    void wheelBoard(Adafruit_NeoPixel & board, int boardLength, int offset);
    void turnOffBoard(Adafruit_NeoPixel & board, int boardLength);
    uint32_t wheelToColor(Adafruit_NeoPixel & board, byte pos);
};

#endif
