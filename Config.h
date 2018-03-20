#ifndef Config_h
#define Config_h

#include "Arduino.h"
#include "Display.h"
#include "Led.h"
#include "Pilot.h"
#include "Mate.h"
#include "Utility.h"
#include "pin.h"

#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Pixy.h>
#include <RotaryEncoder.h>
#include <PID_v1.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_9DOF.h>
#include <Adafruit_L3GD20_U.h>

// Implementierung: FAHREN
extern bool isMotor, start, onLine, isHeadstart, isKeeperLeft;
extern int rotMulti, drivePwr, driveRot, driveDir, lineDir;
extern unsigned long lineTimer, headstartTimer, lastKeeperToggle;
//extern Pilot m;

// Implementierung: KOMPASS
extern int heading, startHeading, rotation;
extern Adafruit_9DOF dof;
extern Adafruit_LSM303_Accel_Unified accel;
extern Adafruit_LSM303_Mag_Unified mag;
extern sensors_event_t accel_event, mag_event;
extern sensors_vec_t orientation;

// Implementierung: BLUETOOTH, MATE
extern bool isConnected, startLast;
extern unsigned long startTimer, bluetoothTimer, heartbeatTimer;
//extern Mate mate;

// Implementierung: WICHTUNG DER PID-REGLER
extern double pidSetpoint, pidIn, pidOut;
extern PID myPID;

// Implementierung: BATTERY
extern byte batState;
extern int batVol;

// Implementierung: PIXY
extern bool seeBall, seeGoal, isDrift, driftLeft;
extern byte pixyState;
extern int ball, ballWidth, ballSize, goal, goalWidth, goalSize;
extern unsigned long seeBallTimer, seeGoalTimer, driftTimer, ballLeftTimer, ballRightTimer, pixyResponseTimer, pixyTimer;
extern Pixy pixy;

// Implementierung: ULTRASCHALL
extern bool usFine;
extern byte us[];
extern unsigned long usTimer;

// Implementierung: KICK, LIGHT-BARRIER
extern bool hasBall;
extern unsigned long kickTimer;

// Implementierung: DISPLAY
extern bool isTypeA;
extern unsigned long lastDisplay;
extern String displayDebug;
//extern Display d;

// Implementierung: LEDS
extern bool stateFine;
extern unsigned int animationPos;
extern Adafruit_NeoPixel bottom, matrix, info;
//extern Led led;

// Implementierung: BUZZER
extern unsigned long buzzerStopTimer;

// Implementierung: ROTARY-ENCODER
extern RotaryEncoder rotaryEncoder;
extern int rotaryPosition;

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
#define LINE_DURATION 100             // wie lange steuern wir der Linie entgegen?
#define HEADSTART_DURATION 700        // wie lange fahren wir volle Geschwindigkeit?
#define DRIFT_DURATION 200            // wie lange steuern wir einem Drift entgegen?
#define PIXY_RESPONSE_DURATION 20000  // wie lange soll die Pixy-Led grün nachleuchten?

// Feld
#define COURT_WIDTH 140         // Summe der Abstände nach rechts und links
#define COURT_GOAL_TO_BORDER 50 // Abstand nach rechts bzw. links am Torpfosten

// LEDS
#define ANIMATION false    // soll die Animation stattfinden?
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

// DISPLAY
#define ROTARY_RANGE 11        // [0 bis *] Anzahl von Bildschirmseiten, die angezeigt werden können
#define SETUP_MESSAGE_RANGE 10 // [0 bis *] Anzahl von Schritten im Setup

// Serielle Kommunikationen
#define DEBUG true                // soll eine Usb-Kommunikation bestehen?
#define DEBUG_SERIAL Serial       // Serial der Usb-Schnittstelle
#define BLUETOOTH true            // soll eine Bluetooth-Kommunikation bestehen?
#define BLUETOOTH_SERIAL Serial1  // Serial des Bluetooth-Moduls
#define BOTTOM_SERIAL Serial2     // Serial des Bodensensor-Arduinos
#define US_SERIAL Serial3         // Serial des Ultraschall-Arduinos
#define START_MARKER 254          // Startzeichen einer Bluetooth-Nachricht
#define END_MARKER 255            // Endzeichen einer Bluetooth-Nachricht

// BATTERY_VOLTAGE
#define VOLTAGE_MIN 40        // [0 bis 126]~40  Mindestspannung des Akkus
#define VOLTAGE_MOTOR_CRIT 95 // [0 bis 126]~90  Kritische Akkuspannung beim Fahren
#define VOLTAGE_MOTOR_LOW 108 // [0 bis 126]~108 Geringe Akkuspannung beim Fahren
#define VOLTAGE_CRIT 95       // [0 bis 126]~90  Kritische Akkuspannung
#define VOLTAGE_LOW 110       // [0 bis 126]~110 Geringe Akkuspannung

// Zeitumwandlung
#define MILS_PER_SEC  (1000UL)  // Millisekunden pro Sekunde
#define MILS_PER_MIN  (60000UL) // Millisekunden pro Minute
#define SECS_PER_MIN  (60UL)    // Sekunden pro Minute
#define MINS_PER_HOUR  (60UL)   // Minuten pro Stunde

// Makro-Methoden
#define X_CENTER ((PIXY_MAX_X-PIXY_MIN_X)/2)                                  // PIXY: Die Mitte des Bildes der Pixy (in Pixeln)
#define debug(_str_); if(DEBUG&&DEBUG_SERIAL){DEBUG_SERIAL.print(_str_);}                   // SERIAL: sende Text zum PC
#define debugln(_str_); if(DEBUG&&DEBUG_SERIAL){DEBUG_SERIAL.println(_str_);}               // SERIAL: sende Zeile zum PC
#define numberOfSeconds(_millis_) ((_millis_ / MILS_PER_SEC) % SECS_PER_MIN)  // TIME: wandle Zeit in Sekundenanzeige um
#define numberOfMinutes(_time_) (_time_ / MILS_PER_MIN) % MINS_PER_HOUR       // TIME: wandle Zeit in Minutenanzeige um

void pinModes();

#endif
