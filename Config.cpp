#include "Config.h"

// Globale Definition: FAHREN
bool start = false;       // ist der funkstart aktiviert
bool onLine = false;      // befinden wir uns auf einer Linie?
bool isHeadstart = false; // fahren wir mit voller Geschwindigkeit?
bool isKeeperLeft = false; // deckten wir zuletzt das Tor mit einer Linksbewegung?
int rotMulti;             // Scalar, um die Rotationswerte zu verstärken
int drivePwr = 0;         // maximale Motorstärke [0 bis 255]
int driveRot = 0;         // korrigiere Kompass
int driveDir = 0;         // Zielrichtung
int lineDir = -1;         // Richtung, in der ein Bodensensor ausschlug
unsigned long lineTimer = 0;      // Zeitpunkt des Interrupts durch einen Bodensensor
unsigned long headstartTimer = 0; // Zeitpunkt des Betätigen des Headstarts
unsigned long lastKeeperToggle = 0; // Zeitpunkt des letzten Richtungswechsel beim Tor schützen
Pilot m;                // OBJEKTINITIALISIERUNG

// Globale Definition: KOMPASS
int heading = 0;                    // Wert des Kompass
int startHeading = 0;               // Startwert des Kompass
int rotation = 0;                   // rotationswert für die Motoren
Adafruit_9DOF                 dof   = Adafruit_9DOF();
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);
sensors_event_t accel_event;
sensors_event_t mag_event;
sensors_vec_t   orientation;

// Globale Definition: BLUETOOTH
bool startLast = false; // war zuletzt der Funktstart aktiviert
unsigned long startTimer = 0; // Zeitpunkt des letzten Start Drückens
unsigned long bluetoothTimer = 0; // Zeitpunkt des letzten Sendens
unsigned long heartbeatTimer = 0; // Zeitpunkt des letzten empfangenen Heartbeat

// Globale Definition: WICHTUNG DER PID-REGLER
double pidSetpoint;       // Nulllevel [-180 bis 180]:Winkel des Tours
double pidIn;             // Kompasswert [-180 bis 180]
double pidOut;            // Rotationsstärke [-255 bis 255]
PID myPID = PID(&pidIn, &pidOut, &pidSetpoint, PID_FILTER_P, PID_FILTER_I, PID_FILTER_D, DIRECT); // OBJEKTINITIALISIERUNG

// Globale Definition: BATTERY
byte batState = 0;  // ist du Spannung zu gering?
int batVol = 0;       // Spannung MAL 10!

// Globale Definition: PIXY
bool driftLeft = false; // steuern wir nach links gegen
bool isDrift = false;   // driften wir
bool seeBall = false;   // sehen wir den Ball?
bool seeGoal = false;   // sehen wir das Tor?
int ball = 0;       // Abweichung der Ball X-Koordinate
int ballWidth = 0;  // Ballbreite
int ballSize = 0;   // Ballgröße (Flächeninhalt)
int goal = 0;       // Abweichung der Tor X-Koordinate
int goalWidth = 0;  // Torbreite
int goalSize = 0;   // Torgröße (Flächeninhalt)
unsigned long seeBallTimer = 0;   // Zeitpunkt des letzten Ball Sehens
unsigned long seeGoalTimer = 0;   // Zeitpunkt des letzen Tor Sehens
unsigned long driftTimer = 0;     // Zeitpunkt seit wann wir gegensteuern
unsigned long ballLeftTimer = 0;  // Zeitpunkt wann der Ball zuletzt links war
unsigned long ballRightTimer = 0; // Zeitpunkt wann der Ball zuletzt rechts war
unsigned long pixyResponseTimer = 0;  // Zeitpunkt der letzten Antwort der Pixy
unsigned long pixyTimer = 0;  // Zeitpunkt des letzten Auslesens der Pixy
Pixy pixy;                    // OBJEKTINITIALISIERUNG

// Globale Definition: ULTRASCHALL
byte us[] = {255, 255, 255, 255};   // Werte des US-Sensors
unsigned long usTimer = 0;  // wann wurde der Us zuletzt ausgelesen?

// Globale Definition: KICK
unsigned long kickTimer = 0;  // Zeitpunkt des letzten Schießens

// Globale Definition: DISPLAY
bool isTypeA; // ist das Roboter A?
unsigned long lastDisplay = 0; // Zeitpunkt des letzten Displayaktualisierens
String displayDebug = "";      // unterste Zeile des Bildschirms;
Adafruit_SSD1306 d = Adafruit_SSD1306(PIN_4);     // OBJEKTINITIALISIERUNG

// Globale Definition: LEDS
bool stateFine = true;  // liegt kein Fehler vor?
bool hasBall = false;   // besitzen der Roboter den Ball?
bool showBottom = true; // sollen die Boden-Leds an sein?
byte pixyState = 0;     // Verbindungsstatus per Pixy
unsigned int animationPos = 1;    // Aktuelle Position in der Animation
Adafruit_NeoPixel bottom = Adafruit_NeoPixel(BOTTOM_LENGTH, BOTTOM_LED, NEO_GRB + NEO_KHZ800); // OBJEKTINITIALISIERUNG (BODEN-LEDS)
Adafruit_NeoPixel matrix = Adafruit_NeoPixel(MATRIX_LENGTH, MATRIX_LED, NEO_GRB + NEO_KHZ800); // OBJEKTINITIALISIERUNG (LED-MATRIX)
Adafruit_NeoPixel info = Adafruit_NeoPixel(INFO_LENGTH, INFO_LED, NEO_GRB + NEO_KHZ800);       // OBJEKTINITIALISIERUNG (STATUS-LEDS)

// Globale Definition: BUZZER
unsigned long buzzerStopTimer = 0; // Zeitpunkt, wann der Buzzer ausgehen soll

// Globale Definition: ROTARY-ENCODER
RotaryEncoder rotaryEncoder = RotaryEncoder(ROTARY_B, ROTARY_A);  // OBJEKTINITIALISIERUNG
int rotaryPosition = 0; // Zustand, der vom Regler eingestellt ist

// Globale Definition: MATE
bool isConnected = false; // besteht eine Bluetooth Verbindung zum Parter
Mate mate;  // OBJEKTINITIALISIERUNG

void pinModes() {
  pinMode(LIGHT_BARRIER,	INPUT_PULLUP);
  pinMode(BATT_VOLTAGE,		INPUT);

  pinMode(POTI,				    INPUT);
  pinMode(TYPE,           INPUT_PULLUP);

  pinMode(BIG_BUTTON,		  INPUT_PULLUP);
  pinMode(SCHUSS_BUTTON,	INPUT_PULLUP);
  pinMode(BUTTON_1, 		  INPUT_PULLUP);
  pinMode(BUTTON_2,			  INPUT_PULLUP);
  pinMode(BUTTON_3,			  INPUT_PULLUP);

  pinMode(SWITCH_MOTOR,   INPUT_PULLUP);
  pinMode(SWITCH_SCHUSS,	INPUT_PULLUP);
  pinMode(SWITCH_KEEPER,	INPUT_PULLUP);
  pinMode(SWITCH_BODENS,	INPUT_PULLUP);
  pinMode(SWITCH_DEBUG,		INPUT_PULLUP);
  pinMode(SWITCH_MOTOR,		INPUT_PULLUP);
  pinMode(SWITCH_B,		  	INPUT_PULLUP);
  pinMode(SWITCH_A,		  	INPUT_PULLUP);

  pinMode(ROTARY_BUTTON,	INPUT_PULLUP);
  pinMode(ROTARY_A,			  INPUT_PULLUP);
  pinMode(ROTARY_B,			  INPUT_PULLUP);

  pinMode(BUZZER_AKTIV,   OUTPUT);
  pinMode(INFO_LED,	  	  OUTPUT);
  pinMode(BUZZER,			    OUTPUT);
  pinMode(MATRIX_LED,		  OUTPUT);

  pinMode(J_A8,				    INPUT);
  pinMode(J_A9,				    INPUT);

  pinMode(INT_BODENSENSOR,INPUT);
  pinMode(INT_US,			    OUTPUT);

  pinMode(SCHUSS,			    OUTPUT);
  pinMode(BOTTOM_LED,		  OUTPUT);

  pinMode(DRDY,				    INPUT);
  pinMode(TILTSWITCH,		  INPUT_PULLUP);

  pinMode(M0_CURR,			  INPUT);
  pinMode(M1_CURR,			  INPUT);
  pinMode(M2_CURR,			  INPUT);
  pinMode(M3_CURR,			  INPUT);
}
