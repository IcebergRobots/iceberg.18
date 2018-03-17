#ifndef Config_h
#define Config_h

#include "Arduino.h"
//#include "Config.h"
#include "Pin.h"
#include "Pilot.h"
#include "Mate.h"
#include "Utility.h"

// Fahren
#define ANGLE_SIDEWAY 100       // [0 bis 180]~100
#define ROTATION_SIDEWAY 100    // [0 bis *]~100
#define ROTATION_AWAY 40        // [0 bis *]~40
#define ROTATION_18CM 50        // [0 bis *]~70
#define ROTATION_10CM 70        // [0 bis *]~90
#define ROTATION_TOUCH 40       // [0 bis *]~20
#define SPEED 60                // [0 bis 255]~60
#define SPEED_SIDEWAY 60        // [0 bis 255]~60
#define SPEED_BALL_IN_FRONT 120 // [0 bis 255]~255
#define SPEED_BACKWARDS 50      // [0 bis 255]~40
#define SPEED_KEEPER 40         // [0 bis 255]~40
#define SPEED_HEADSTART 120     // [0 bis 255]~120
#define SPEED_LINE 80           // [0 bis 255]~80

// TIMES
#define LINE_DURATION 100       // wie lange steuern wir der Linie entgegen
#define HEADSTART_DURATION 700  // wie lange fahren wir volle Geschwindigkeit
#define DRIFT_DURATION 200      // wir lange steuern wir einem Drift entgegen

// Feld
#define COURT_WIDTH 140         // Summe der Abstände nach rechts und links
#define COURT_GOAL_TO_BORDER 50 // Abstand nach rechts bzw. links am Torpfosten

// LEDS
#define ANIMATION true        // soll die Animation stattfinden?
#define ANIMATION_SPEED .01   // [0 bis *] Geschwindigkeit der Animation
#define BOTTOM_LENGTH 16      // [0 bis *] Anzahl der Boden-Leds
#define BOTTOM_BRIGHTNESS 255 // [0 bis 255] Helligkeit der Boden-Leds
#define MATRIX_LENGTH  12     // [0 bis *] Anzahl der Matrix-Leds
#define MATRIX_BRIGHTNESS 40  // [0 bis 255] Helligkeit der Matrix-Leds
#define INFO_LENGTH 3         // [0 bis *] Anzahl der Info-Leds
#define INFO_BRIGHTNESS 100   // [0 bis 255] Helligkeit der Info-Leds

// PID-Regler
#define PID_FILTER_P .27 // [0 bis *]~.27 p:proportional
#define PID_FILTER_I 0   // [0 bis *]~0   i:vorausschauend
#define PID_FILTER_D .03 // [0 bis *]~.03 d:Schwung herausnehmen (nicht zu weit drehen)

// LIGHTBARRIER
#define LIGHT_BARRIER_TRIGGER_LEVEL 50  // [0 bis 1023]~50 Wert, ab dem Lichtschranke ausschlägt

// PIYX
#define SIGNATURE_BALL 1  // Pixy-Signature des Balls
#define SIGNATURE_GOAL 2  // Pixy-Signature des Tors

// ROTARY-ENCODER
#define ROTARY_RANGE 11   // [0 bis *] Anzahl von Bildschirmseiten, die angezeigt werden können

// Serielle Kommunikationen
#define DEBUG false               // soll eine Usb-Kommunikation bestehen?
#define DEBUG_SERIAL Serial       // Serial der Usb-Schnittstelle
#define BLUETOOTH true            // soll eine Bluetooth-Kommunikation bestehen?
#define BLUETOOTH_SERIAL Serial1  // Serial des Bluetooth-Moduls
#define BOTTOM_SERIAL Serial2     // Serial des Bodensensor-Arduinos
#define US_SERIAL Serial3         // Serial des Ultraschall-Arduinos
#define START_MARKER 254          // Startzeichen einer Bluetooth-Nachricht
#define END_MARKER 255            // Endzeichen einer Bluetooth-Nachricht

// BATTERY_VOLTAGE
#define VOLTAGE_MOTOR_CRIT 90 // [0 bis 126]~90  Kritische Akkuspannung beim Fahren
#define VOLTAGE_MOTOR_LOW 108 // [0 bis 126]~108 Geringe Akkuspannung beim Fahren
#define VOLTAGE_CRIT 90       // [0 bis 126]~90  Kritische Akkuspannung
#define VOLTAGE_LOW 110       // [0 bis 126]~110 Geringe Akkuspannung

// Zeitumwandlung
#define MILS_PER_SEC  (1000UL)  // Millisekunden pro Sekunde
#define MILS_PER_MIN  (60000UL) // Millisekunden pro Minute
#define SECS_PER_MIN  (60UL)    // Sekunden pro Minute
#define MINS_PER_HOUR  (60UL)   // Minuten pro Stunde

// Makro-Methoden
#define X_CENTER ((PIXY_MAX_X-PIXY_MIN_X)/2)                                  // PIXY: Die Mitte des Bildes der Pixy (in Pixeln)
#define debug(_str_); if(DEBUG){DEBUG_SERIAL.print(_str_);}                   // SERIAL: sende Text zum PC
#define debugln(_str_); if(DEBUG){DEBUG_SERIAL.println(_str_);}               // SERIAL: sende Zeile zum PC
#define numberOfSeconds(_millis_) ((_millis_ / MILS_PER_SEC) % SECS_PER_MIN)  // TIME: wandle Zeit in Sekundenanzeige um
#define numberOfMinutes(_time_) (_time_ / MILS_PER_MIN) % MINS_PER_HOUR       // TIME: wandle Zeit in Minutenanzeige um

void pinModes();

#endif
