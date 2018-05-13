#ifndef Config_h
#define Config_h

#include "Arduino.h"
#include "Display.h"
#include "Led.h"
#include "Pilot.h"
#include "Player.h"
#include "Mate.h"
#include "Ultrasonic.h"
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
extern byte role;
extern int rotMulti, drivePower, driveRotation, driveDirection, driveOrientation, lineDir;
extern unsigned long lineTimer, headstartTimer, lastKeeperToggle, lastGoalFree, flatTimer;
extern String driveState;

// Implementierung: KOMPASS
extern int heading, startHeading, rotation;
extern Adafruit_9DOF dof;
extern Adafruit_LSM303_Accel_Unified accel;
extern Adafruit_LSM303_Mag_Unified mag;
extern sensors_event_t accel_event, mag_event;
extern sensors_vec_t orientation;

// Implementierung: BLUETOOTH, MATE
extern bool startLast;
extern unsigned long startTimer, bluetoothTimer;

// Implementierung: WICHTUNG DER PID-REGLER
extern double pidSetpoint, pidIn, pidOut;
extern PID myPID;

// Implementierung: BATTERY
extern byte batState;
extern int batVol;

// Implementierung: PIXY
extern bool seeBall, seeGoal, seeEast, seeWest, closeBall, isDrift, driftLeft;
extern byte pixyState, blockCount, blockCountBall, blockCountGoal, blockCountEast, blockCountWest;
extern int ball, ballWidth, ballArea, goal, goalWidth, goalArea, east, eastHeight, west, westHeight;
extern unsigned long seeBallTimer, seeGoalTimer, seeEastTimer, seeWestTimer, closeBallTimer, driftTimer, ballLeftTimer, ballRightTimer, pixyResponseTimer, pixyTimer;
extern Pixy pixy;

// Implementierung: ULTRASCHALL
extern unsigned long usTimer;

// Implementierung: KICK, LIGHT-BARRIER
extern bool hasBall;
extern unsigned long kickTimer;
extern unsigned int lightBarrierTriggerLevel;

// Implementierung: LIFT
extern bool isLifted;

// Implementierung: DISPLAY
extern bool isTypeA;
extern unsigned long lastDisplay;
extern String displayDebug;

// Implementierung: LEDS
extern bool stateFine;
extern unsigned long ledTimer;
extern Adafruit_NeoPixel bottom, matrix, info;
//extern Led led;

// Implementierung: BUZZER
extern byte silent;
extern unsigned long buzzerStopTimer;

// Implementierung: ROTARY-ENCODER
extern RotaryEncoder rotaryEncoder;
extern int rotaryPositionLast;
extern bool wasSelect, wasBack;

// Fahren
#define ROLE_COOLDOWN 1000      // [0 bis *]~1000 Zeitspanne, in dem kein Rollenwechsel stattfindet
#define ANGLE_SIDEWAY 100       // [0 bis 180]~100
#define ANGLE_GOAL_MAX 45       // [0 bis 180]~45 maximaler Orientierungswinkel zum Tor
#define ANGLE_PASSIVE_MAX 35    // [0 bis 180]~45 maximaler Orientierungswinkel beim Zurückfahren
#define ANGLE_TURN_MAX 90       // [0 bis 180]~90 maximaler Orientierungswinkel zur Ballsuche
#define ANGLE_CENTER 15         // [0 bis 180]~15 Toleranz für mittige Objekte
#define ROTATION_SIDEWAY 100    // [0 bis *]~100
#define ROTATION_AWAY 40        // [0 bis *]~40
#define ROTATION_18CM 50        // [0 bis *]~70
#define ROTATION_10CM 70        // [0 bis *]~90
#define ROTATION_TOUCH 40       // [0 bis *]~20
#define SPEED_BACKWARDS 70      // [0 bis 255]~40  STATUS 0: Nach hinten
#define SPEED_PENALTY 50        // [0 bis 255]~40  STATUS 0: Nach hinten
#define SPEED_KEEPER 60         // [0 bis 255]~40  STATUS 1: Torverteidigung
#define SPEED_FREE 70           // [0 bis 255]~255 STATUS 4: Befreiung
#define SPEED_LOST 100          // [0 bis 255]~100 STATUS 5: Seitlich verloren
#define SPEED_SIDEWAY 100       // [0 bis 255]~60  STATUS 6: Ballverfolgung
#define SPEED_BALL 72           // [0 bis 255]~72  STATUS 6: Ballverfolgung
#define SPEED_CLOSE 100         // [0 bis 255]~100 STATUS 7: Torausrichtung
#define SPEED_ATTACK 100        // [0 bis 255]~100 STATUS 8: Angriff
#define SPEED_DRIFT 140         // [0 bis 255]~140
#define SPEED_LINE 90           // [0 bis 255]~80



// TIMES
#define BACKWARD_MAX_DURATION 4000    // wann darf frühestens eingegriffen werden
#define GOAL_STUCK_DURATION 500       // wie lange soll nach vorne gefahren werden?
#define SIDEWARD_MAX_DURATION 2000    // max Zeit für Seitwärtsfahren
#define SIDEWARD_MIN_DURATION 800     // min Zeit für Seitwärtsfahren
#define TURN_MAX_DURATION 2000        // max Zeit für Drehmodus
#define TURN_BACK_MAX_DURATION 2000   // max Zeit für Drehmodus zurück
#define AVOID_MATE_DURATION 1000      // max Zeit für Ausweichmanöver
#define ROLE_LED_DURATION 350         // wie lange soll die Spielrolle angezeigt werden?
#define LINE_DURATION 300             // wie lange steuern wir der Linie entgegen?
#define HEADSTART_DURATION 350        // wie lange fahren wir volle Geschwindigkeit?
#define DRIFT_DURATION 200            // wie lange steuern wir einem Drift entgegen?
#define PIXY_RESPONSE_DURATION 20000  // wie lange soll die Pixy-Led grün nachleuchten?

// Feld
#define COURT_REARWARD_MAX 50           // optimaler Abstand nach hinten
#define COURT_REARWARD_MIN 15           // optimaler Abstand nach hinten
#define COURT_WIDTH 150         // Summe der Abstände nach rechts und links
#define COURT_GOAL_TO_BORDER 65 // Abstand nach rechts bzw. links am Torpfosten
#define COURT_WIDTH_FREE 140
#define COURT_POST_TO_BORDER 110 // Abstand nach rechts bzw. links am Torpfosten

// LEDS
#define ANIMATION true          // soll die Animation stattfinden?
#define ANIMATION_SPEED .01     // Geschwindigkeit der Animation
#define ANIMATION_DURATION 2000 // Länge der Animation
#define BOTTOM_LENGTH 16        // [0 bis *] Anzahl der Boden-Leds
#define BOTTOM_BRIGHTNESS 255   // [0 bis 255] Helligkeit der Boden-Leds
#define MATRIX_LENGTH  12       // [0 bis *] Anzahl der Matrix-Leds
#define MATRIX_BRIGHTNESS 50    // [0 bis 255] Helligkeit der Matrix-Leds
#define INFO_LENGTH 3           // [0 bis *] Anzahl der Info-Leds
#define INFO_BRIGHTNESS 100     // [0 bis 255] Helligkeit der Info-Leds

// PID-Regler
#define PID_FILTER_P .27 // [0 bis *]~.27 p:proportional
#define PID_FILTER_I 0   // [0 bis *]~0   i:vorausschauend
#define PID_FILTER_D .03 // [0 bis *]~.03 d:Schwung herausnehmen (nicht zu weit drehen)

// PIYX
#define SIGNATURE_BALL 1  // Pixy-Signature des Balls
#define SIGNATURE_GOAL 2  // Pixy-Signature des Tors
#define SIGNATURE_CC 28   // Pixy-Signature des Tors
#define BALL_WIDTH_TRIGGER 50 // Schwellwert eines großen Balles

// DISPLAY
#define SETUP_MESSAGE_RANGE 10 // [0 bis *] Anzahl von Schritten im Setup

// Serielle Kommunikationen
#define DEBUG true                // soll eine Usb-Kommunikation bestehen?
#define DEBUG_SERIAL Serial       // Serial der Usb-Schnittstelle
#define BLUETOOTH true            // soll eine Bluetooth-Kommunikation bestehen?
#define BLUETOOTH_SERIAL Serial1  // Serial des Bluetooth-Moduls
#define BOTTOM_SERIAL Serial2     // Serial des Bodensensor-Arduinos
#define ULTRASONIC_SERIAL Serial3         // Serial des Ultraschall-Arduinos
#define START_MARKER 254          // Startzeichen einer Bluetooth-Nachricht
#define END_MARKER 255            // Endzeichen einer Bluetooth-Nachricht

// BATTERY_VOLTAGE
#define VOLTAGE_MIN 40        // [0 bis 126]~40  Mindestspannung des Akkus
#define VOLTAGE_MOTOR_CRIT 100 // [0 bis 126]~90  Kritische Akkuspannung beim Fahren
#define VOLTAGE_MOTOR_LOW 108 // [0 bis 126]~108 Geringe Akkuspannung beim Fahren
#define VOLTAGE_CRIT 100       // [0 bis 126]~90  Kritische Akkuspannung
#define VOLTAGE_LOW 110       // [0 bis 126]~110 Geringe Akkuspannung

// Zeitumwandlung
#define MILS_PER_SEC  (1000UL)  // Millisekunden pro Sekunde
#define MILS_PER_MIN  (60000UL) // Millisekunden pro Minute
#define SECS_PER_MIN  (60UL)    // Sekunden pro Minute
#define MINS_PER_HOUR  (60UL)   // Minuten pro Stunde

// Makro-Methoden
#define X_CENTER ((PIXY_MAX_X-PIXY_MIN_X)/2)                                  // PIXY: Die Mitte des Bildes der Pixy (in Pixeln)
#define numberOfSeconds(_millis_) ((_millis_ / MILS_PER_SEC) % SECS_PER_MIN)  // TIME: wandle Zeit in Sekundenanzeige um
#define numberOfMinutes(_time_) (_time_ / MILS_PER_MIN) % MINS_PER_HOUR       // TIME: wandle Zeit in Minutenanzeige um

void pinModes();

#endif
