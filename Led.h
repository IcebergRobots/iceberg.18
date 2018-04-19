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
    void animation();
    bool isAnimation();
  private:
    void showState(Adafruit_NeoPixel & board, byte pos, byte state, bool showRed);
    void showState(Adafruit_NeoPixel & board, byte pos, byte state);
    void wheelBoard(Adafruit_NeoPixel & board, int boardLength, int offset);
    void setBoard(Adafruit_NeoPixel & board, int boardLength, uint32_t color);
    uint32_t wheelToColor(Adafruit_NeoPixel & board, byte pos);

    unsigned int animationPosition = 0;  // Position in der Animation
    unsigned long animationTimer = 0;    // Zeitpunkt des Animationsstarts
};

#endif
