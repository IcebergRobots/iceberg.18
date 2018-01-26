#ifndef Config_h
#define Config_h

#include "Arduino.h"
#include "Pin.h"
#include "Pilot.h"
#include "Utility.h"

#define DEBUG true
#define DEBUG_SERIAL Serial
#define BLUETOOTH true
#define BLUETOOTH_SERIAL Serial1

#define START_MARKER '<'
#define END_MARKER '>'

#define debug(_str_); if(DEBUG){DEBUG_SERIAL.print(_str_);}
#define debugln(_str_); if(DEBUG){DEBUG_SERIAL.println(_str_);}
#define bluetooth(_str_); if(BLUETOOTH){BLUETOOTH_SERIAL.print(START_MARKER);BLUETOOTH_SERIAL.print(_str_);BLUETOOTH_SERIAL.print(END_MARKER);}

// Makros für Zeitumwandlung
/* Useful Constants */
#define MILS_PER_SEC  (1000UL)
#define MILS_PER_MIN  (60000UL)
#define SECS_PER_MIN  (60UL)

/* Useful Macros for getting elapsed time */
#define numberOfMillis(_time_) (_time_ % MILS_PER_SEC)
#define numberOfSeconds(_time_) ((_time_ / MILS_PER_SEC) % SECS_PER_MIN)
#define numberOfMinutes(_time_) (_time_ / MILS_PER_MIN)

void pinModes();

#endif
