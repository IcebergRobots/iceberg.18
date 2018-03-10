#ifndef Config_h
#define Config_h

#include "Arduino.h"
#include "Pin.h"
#include "Pilot.h"
#include "Utility.h"

// Fahren
#define ANGLE_SIDEWAY 100       // [0 bis 180] default=100
#define ROTATION_SIDEWAY 100    // [0 bis *] default=100
#define ROTATION_AWAY 40        // [0 bis *] default=40
#define ROTATION_18CM 70        // [0 bis *] default=70
#define ROTATION_10CM 90        // [0 bis *] default=90
#define ROTATION_TOUCH 40       // [0 bis *] default=20
#define SPEED 60                // [0 bis 255] default=60
#define SPEED_SIDEWAY 60        // [0 bis 255] default=60
#define SPEED_BALL_IN_FRONT 120 // [0 bis 255] default=255
#define SPEED_BACKWARDS 50      // [0 bis 255] default=40
#define SPEED_KEEPER 40         // [0 bis 255] default=40
#define SPEED_HEADSTART 120     // [0 bis 255] default=120

// TIMES
#define LINE_DELAY 100
#define HEADSTART_DELAY 500
#define DRIFT_DELAY 200

// Feld
#define COURT_WIDTH 140         // Summe der Abstände nach rechts und links
#define COURT_GOAL_TO_BORDER 50 // Abstand nach rechts bzw. links am Torpfosten

// PID-Regler
#define PID_FILTER_P 0.27 //.52 // p:proportional
#define PID_FILTER_I 0 //.03 // i:vorausschauend
#define PID_FILTER_D 0.03 //.03 // d:Schwung herausnehmen (nicht zu weit drehen)

// LIGHTBARRIER
#define LIGHT_BARRIER_TRIGGER_LEVEL 50  // Wert, ab dem Lichtschranke ausschlägt

// PIYX
#define SIGNATURE_BALL 1    // Pixy-Signature des Balls
#define SIGNATURE_GOAL 2    // Pixy-Signature des Tors

// STATUS-LEDS & LED-MATRIX
#define PWR_LED 20            // Helligkeit der Status-Leds

// ROTARY-ENCODER
#define ROTARY_RANGE 11  // [0 bis *] Anzahl von Bildschirmseiten, die angezeigt werden können

// Serielle Kommunikationen
#define DEBUG false
#define DEBUG_SERIAL Serial
#define BLUETOOTH true
#define BLUETOOTH_SERIAL Serial1
#define BOTTOM_SERIAL Serial2
#define US_SERIAL Serial3
#define START_MARKER 254
#define END_MARKER 255
#define CACHE_SIZE 10

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
