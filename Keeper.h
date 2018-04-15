#ifndef Keeper_h
#define Keeper_h

#include "Config.h"

class Keeper
{
  public:
    Keeper();
    
    void set();
    void right();
    void left();
    void toggle();
    bool atGatepost();
    unsigned long lastToggle();

  private:
    bool movingLeft = true;
    unsigned long toggleTimer = 0;
};

#endif
