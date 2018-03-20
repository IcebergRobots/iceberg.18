#ifndef Display_h
#define Display_h

#include "Config.h"
#include "pin.h"
#include <Adafruit_SSD1306.h>

class Display: public Adafruit_SSD1306
{
  public:
    Display(int resetPin) : Adafruit_SSD1306(resetPin) {}
  private:
};

#endif
