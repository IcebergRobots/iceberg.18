#ifndef Config_h
#define Config_h

#include "Arduino.h"
#include "Pin.h"
#include "Pilot.h"
#include "Utility.h"

// Fahren
#define ROT_MULTI 70    // in Prozent, default=90
#define ROT_MAX 0.5     // der maximale Wert der Rotation

// PID-Regler
#define PID_FILTER_P 0.32 // p:proportional
#define PID_FILTER_I 0.03 // i:vorausschauend
#define PID_FILTER_D 0.03 // d:Schwung herausnehmen (nicht zu weit drehen)

// Pixy
#define SPEED 100             // Geschwindigkeit des Roboters in %
#define PIXY_BALL_NUMMER 1    // Pixy-Signature des Balls

// Einstellungen: STATUS-LEDS & LED-MATRIX
#define PWR_LED 20            // Helligkeit der Status-Leds

// Serielle Kommunikationen
#define DEBUG true
#define DEBUG_SERIAL Serial
#define BLUETOOTH true
#define BLUETOOTH_SERIAL Serial1
#define START_MARKER '<'
#define END_MARKER '>'

// Zeitumwandlung
#define MILS_PER_SEC  (1000UL)
#define MILS_PER_MIN  (60000UL)
#define SECS_PER_MIN  (60UL)
#define MINS_PER_HOUR  (60UL)


// Makro-Methoden
#define X_CENTER ((PIXY_MAX_X-PIXY_MIN_X)/2)    // PIXY: Die Mitte des Bildes der Pixy (in Pixeln)
#define debug(_str_); if(DEBUG){DEBUG_SERIAL.print(_str_);}     // SERIAL: sende Text zum PC
#define debugln(_str_); if(DEBUG){DEBUG_SERIAL.println(_str_);} // SERIAL: sende Zeile zum PC
#define bluetooth(_str_); if(BLUETOOTH){BLUETOOTH_SERIAL.print(START_MARKER);BLUETOOTH_SERIAL.print(_str_);BLUETOOTH_SERIAL.print(END_MARKER);} // SERIAL: sende Zeile zum anderen Roboter
#define numberOfSeconds(_millis_) ((_millis_ / MILS_PER_SEC) % SECS_PER_MIN)  // TIME: wandle Zeit in Sekundenanzeige um
#define numberOfMinutes(_time_) (_time_ / MILS_PER_MIN) % MINS_PER_HOUR       // TIME: wandle Zeit in Minutenanzeige um


void pinModes();

#endif
