#ifndef Config_h
#define Config_h

#include "Arduino.h"
#include "Pin.h"
#include "Pilot.h"
#include "Utility.h"

#define DEBUG true

#define debug(_str_); if(DEBUG){Serial.print(_str_);}
#define debugln(_str_); if(DEBUG){Serial.println(_str_);}

void pinModes();
void motorConfig();

Pilot m;

#endif
