#ifndef Config_h
#define Config_h

#include "Arduino.h"
#include "Pin.h"
#include "Pilot.h"
#include "Utility.h"

// Fahren
#define ROT_MULTI 80     // in Prozent, default=90

// PID-Regler
#define PID_FILTER_P 0.2//.52 // p:proportional
#define PID_FILTER_I 0//.03 // i:vorausschauend
#define PID_FILTER_D 0.02//.03 // d:Schwung herausnehmen (nicht zu weit drehen)

// LIGHTBARRIER
#define LIGHT_BARRIER_TRIGGER_LEVEL 50  // Wert, ab dem Lichtschranke ausschlägt

// Pixy
#define SPEED_BALL_IN_FRONT 80
#define SPEED_BACKWARDS 40
#define SPEED 60              // Geschwindigkeit des Roboters [0 bis 255]
#define PIXY_BALL_NUMMER 1    // Pixy-Signature des Balls

// Einstellungen: STATUS-LEDS & LED-MATRIX
#define PWR_LED 20            // Helligkeit der Status-Leds

// Serielle Kommunikationen
#define DEBUG true
#define DEBUG_SERIAL Serial
#define BLUETOOTH true
#define BLUETOOTH_SERIAL Serial1
#define BOTTOM_SERIAL Serial2
#define US_SERIAL Serial3
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
