/*** 
  _____ _______ _______ ______  _______  ______  ______
    |   |       |______ |_____] |______ |_____/ |  ____
  __|__ |_____  |______ |_____] |______ |    \_ |_____|

   ______  _____  ______   _____  _______ _______
  |_____/ |     | |_____] |     |    |    |______
  |    \_ |_____| |_____] |_____|    |    ______|

*/

// Implementierung: DATEIEN
#include "Config.h"

// Globale Definition: FAHREN
bool isMotor = false;       // sind die Motoren aktiviert?
bool start = false;         // ist der funkstart aktiviert
bool onLine = false;        // befinden wir uns auf einer Linie?
bool isHeadstart = false;   // fahren wir mit voller Geschwindigkeit?
bool isKeeperLeft = false;  // deckten wir zuletzt das Tor mit einer Linksbewegung?
byte role = 0;              // Spielrolle: Stürmer(2) / Torwart(1) / Aus(0)
int rotMulti;               // Scalar, um die Rotationswerte zu verstärken
int drivePower = 0;         // [-255 bis 255] aktuelle maximale Motorstärke
int driveRotation = 0;       // [-255 bis 255] aktuelle Rotationsstärke
int driveDirection = 0;     // [-180 bis 180] Ziel-Fahrrichtung
int driveOrientation = 0;   // [-180 bis 180] Ziel-Orientierungswinkel
int lineDir = -1;           // Richtung, in der ein Bodensensor ausschlug
unsigned long lineTimer = 0;        // Zeitpunkt des Interrupts durch einen Bodensensor
unsigned long headstartTimer = 0;   // Zeitpunkt des Betätigen des Headstarts
unsigned long lastKeeperToggle = 0; // Zeitpunkt des letzten Richtungswechsel beim Tor schützen
unsigned long lastGoalFree = 0;     // Zeitpunkt des letzten freien Tors
unsigned long flatTimer = 0;    // Zeitpunktm zu dem der Roboter das letzte mal flach auf dem Boden stand
String driveState = "";             // Zustand des Fahrens
Keeper keeper;  // OBJEKTINITIALISIERUNG
Pilot m;  // OBJEKTINITIALISIERUNG

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

// Globale Definition: BLUETOOTH, MATE
bool startLast = false; // war zuletzt der Funktstart aktiviert
unsigned long startTimer = 0; // Zeitpunkt des letzten Start Drückens
unsigned long bluetoothTimer = 0; // Zeitpunkt des letzten Sendens
Mate mate;  // OBJEKTINITIALISIERUNG

// Globale Definition: WICHTUNG DER PID-REGLER
double pidSetpoint;       // Nulllevel [-180 bis 180] Winkel des Tours
double pidIn;             // Kompasswert [-180 bis 180]
double pidOut;            // Rotationsstärke [-255 bis 255]
PID myPID = PID(&pidIn, &pidOut, &pidSetpoint, PID_FILTER_P, PID_FILTER_I, PID_FILTER_D, DIRECT); // OBJEKTINITIALISIERUNG

// Globale Definition: BATTERY
byte batState = 0;  // ist du Spannung zu gering?
int batVol = 0;       // Spannung MAL 10!

// Globale Definition: PIXY
bool seeBall = false;   // sehen wir den Ball?
bool seeGoal = false;   // sehen wir das Tor?
bool isDrift = false;   // driften wir
bool driftLeft = false; // steuern wir nach links gegen
byte pixyState = 0;     // Verbindungsstatus per Pixy
byte blockCount = 0;    // Anzahl der gesehenen Blöcke
byte blockCountBall = 0;// Anzahl der Ball Blöcke
byte blockCountGoal = 0;// Anzahl der Tor Blöcke
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
Pixy pixy;  // OBJEKTINITIALISIERUNG

// Globale Definition: ULTRASCHALL
unsigned long usTimer = 0;  // Zeitpunkt des letzten Auslesens
Ultrasonic us;  // OBJEKTINITIALISIERUNG


// Globale Definition: KICK, LIGHT-BARRIER
bool hasBall = false;   // besitzen der Roboter den Ball?
unsigned long kickTimer = 0;  // Zeitpunkt des letzten Schießens
unsigned int lightBarrierTriggerLevel = 80; // [0 bis 1023]~50 Wert, ab dem Lichtschranke ausschlägt

// Globale Definition: LIFT
bool isLifted = false;  // ist der Roboter hochgehoben?

// Globale Definition: DISPLAY
bool isTypeA; // ist das Roboter A?
unsigned long lastDisplay = 0; // Zeitpunkt des letzten Displayaktualisierens
String displayDebug = "";      // unterste Zeile des Bildschirms;
Display d = Display(PIN_4); // OBJEKTINITIALISIERUNG

// Globale Definition: LEDS
bool stateFine = true;            // liegt kein Fehler vor?
unsigned long ledTimer = 0;       // Zeitpunkt der letzten Led-Aktualisierung
Adafruit_NeoPixel bottom = Adafruit_NeoPixel(BOTTOM_LENGTH, BOTTOM_LED, NEO_GRB + NEO_KHZ800); // OBJEKTINITIALISIERUNG (BODEN-LEDS)
Adafruit_NeoPixel matrix = Adafruit_NeoPixel(MATRIX_LENGTH, MATRIX_LED, NEO_GRB + NEO_KHZ800); // OBJEKTINITIALISIERUNG (LED-MATRIX)
Adafruit_NeoPixel info = Adafruit_NeoPixel(INFO_LENGTH, INFO_LED, NEO_GRB + NEO_KHZ800);       // OBJEKTINITIALISIERUNG (STATUS-LEDS)
Led led;  // OBJEKTINITIALISIERUNG

// Globale Definition: BUZZER
byte silent = false;
unsigned long buzzerStopTimer = 0; // Zeitpunkt, wann der Buzzer ausgehen soll

// Globale Definition: ROTARY-ENCODER
RotaryEncoder rotaryEncoder = RotaryEncoder(ROTARY_B, ROTARY_A);  // OBJEKTINITIALISIERUNG
int rotaryPositionLast = 0; // letzter Zustand des Reglers
bool wasSelect = false;     // war der Auswählen-Knopf gedrückt?
bool wasBack = false;       // war der Zurück-Knopf gedrückt?

//###################################################################################################

void setup() {
  // Prüfe, ob die Pixy angeschlossen ist
  SPI.begin();
  pixyResponseTimer = SPI.transfer(0x00) == 255;

  silent = !digitalRead(SWITCH_DEBUG);  // Schnellstart?
  if (analogRead(LIGHT_BARRIER) > 50 && analogRead(LIGHT_BARRIER) < 400) lightBarrierTriggerLevel = analogRead(LIGHT_BARRIER) + 100;

  d.init();  // initialisiere Display mit Iceberg Schriftzug

  // Roboter bremst aktiv
  m.brake(true);

  // Start der Seriellen Kommunikation
  DEBUG_SERIAL.begin(115200);
  BLUETOOTH_SERIAL.begin(115200);
  ULTRASONIC_SERIAL.begin(115200);
  BOTTOM_SERIAL.begin(115200);
  Wire.begin();         // Start der I2C-Kommunikation

  // konfiguriere PID-Regler
  myPID.SetTunings(PID_FILTER_P, PID_FILTER_I, PID_FILTER_D);

  // weiche den Linien aus
  attachInterrupt(digitalPinToInterrupt(INT_BODENSENSOR), avoidLine, RISING);

  // setzte die PinModes
  d.setupMessage(1, "PIN", "pinModes");
  pinModes();
  isTypeA = digitalRead(TYPE); // lies den Hardware Jumper aus

  // setzte Pins und Winkel des Pilot Objekts
  d.setupMessage(2, "MOTOR", "setPins");
  setupMotor();

  // initialisiere Kamera
  d.setupMessage(3, "PIXY", "Kamera");
  pixy.init();
  pixy.setLED(0, 0, 0); // schalte die Hauptled der Pixy aus

  // lies EEPROM aus
  d.setupMessage(4, "EEPROM", "auslesen");
  if (EEPROM.read(0) == 0) {
    startHeading = EEPROM.read(1);
  } else {
    startHeading = -EEPROM.read(1);
  }

  // initialisiere Beschleunigungssensor
  d.setupMessage(5, "ACCEL", "Beschleunigungssensor");
  if (!accel.begin()) {
    stateFine = false;
    d.setupMessage(5, "ACCEL", "failed");
  }

  // initialisiere Magnetfeldsensor
  d.setupMessage(6, "MAG", "Magnetfeldsensor");
  if (!mag.begin()) {
    stateFine = false;
    d.setupMessage(6, "MAG", "failed");
  }
  delay(1);

  // initialisiere Kompasssensor
  d.setupMessage(7, "COMPASS", "Orientierung");
  mag.enableAutoRange(true);  // aktiviere automatisches Messen
  readCompass();  // lies Kompassrichtung aus

  // initialisiere PID-Regler
  d.setupMessage(8, "PID", "Rotation");
  pidSetpoint = 0;
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(-255, 255);

  // initialisiere Leds
  d.setupMessage(9, "LED", "Animation");
  bottom.begin();   // BODEN-LEDS initialisieren
  matrix.begin();   // MATRIX-LEDS initialisieren
  info.begin();     // STATUS-LEDS initialisieren
  led.animation();
  d.setupMessage(10, "B: " + String(lightBarrierTriggerLevel), "");
  debugln("setup done");

  // sorge dafür, dass alle Timer genügend Abstand haben
  while (millis() < 1000) {}
}
//###################################################################################################

void loop() {
  debug(String(millis()) + " ");

  displayDebug = "";

  readCompass();

  if (millis() - kickTimer > map(analogRead(POTI), 0, 1023, 0, 35)) digitalWrite(SCHUSS, 0); // schuß wieder ausschalten

  if (batState == 3) analogWrite(BUZZER, 127 * (millis() % 250 < 125));
  else analogWrite(BUZZER, 127 * millis() <= buzzerStopTimer);  // buzzer anschalten bzw. wieder ausschalten

  while (BOTTOM_SERIAL.available() > 1) { 
    BOTTOM_SERIAL.read(); 
  } 

  // Seitenauswahl
  // auswählen
  if (!digitalRead(ROTARY_BUTTON) && !wasSelect) {
    d.select();
  }
  wasSelect = !digitalRead(ROTARY_BUTTON);
  // zurück
  if (!digitalRead(BUTTON_3) && !wasBack ) {
    d.back();
  }
  wasBack = !digitalRead(BUTTON_3);
  // drehen
  rotaryEncoder.tick(); // erkenne Reglerdrehungen
  if (rotaryEncoder.getPosition() != rotaryPositionLast) {
    d.change(rotaryEncoder.getPosition() - rotaryPositionLast);
  }
  rotaryPositionLast = rotaryEncoder.getPosition();

  if (!digitalRead(BUTTON_1)) led.animation(); // starte die Animation

  // Torrichtung speichern
  if (!digitalRead(BUTTON_2)) {
    startHeading = 0;
    readCompass();
    startHeading = heading; //merke Torrichtung [-180 bis 179]
    EEPROM.write(0, startHeading < 0);  // speichere Vorzeichen
    EEPROM.write(1, abs(startHeading)); // speichere Winkel
    heading = 0;
    buzzerTone(200);
    d.update();   // aktualisiere Bildschirm und LEDs
  }

  // lösche Bodensensor Cache
  while (BOTTOM_SERIAL > 1) {
    BOTTOM_SERIAL.read();
  }

  if ((seeGoal && abs(heading - pidSetpoint) < 45 && hasBall) || !digitalRead(SCHUSS_BUTTON)) kick(); // schieße

  calculateStates();  // Berechne alle Statuswerte und Zustände

  if (led.isAnimation() || millis() - ledTimer > 100) {
    debug("led ");
    led.set();  // Lege Leds auf Statusinformation fest
    led.led();  // Aktualisiere alle Leds bzw. zeige die Animation
  }

  if (millis() - pixyTimer > 50) {
    debug("pixy ");
    readPixy(); // aktualisiere Pixywerte (max. alle 50ms)
  }

  if (millis() - usTimer > 100) us.receive(); // lese die Ultraschall Sensoren aus (max. alle 100ms)

  // remote start
  if (!digitalRead(BIG_BUTTON)) {
    if (!startLast && digitalRead(SWITCH_B) && !m.getMotEn()) headstartTimer = millis();
    if (!startLast || millis() - startTimer < 100) {
      start = true;
    } else if (millis() - startTimer > 1000) {
      byte data[1] = {'b'};
      mate.send(data, 1);
      m.brake(true);
      start = false;
    }
    startLast = true;
  } else {
    startLast = false;
    startTimer = millis();
  }

  // starte über Funk wenn Schalter Keeper aktiviert
  if (!digitalRead(SWITCH_MOTOR)) {
    m.setMotEn(!digitalRead(SWITCH_KEEPER) || start);
  } else {
    m.setMotEn(false);
    start = false;
  }

  if (millis() - bluetoothTimer > 100 || start != startLast)  transmitHeartbeat(); // Sende einen Herzschlag mit Statusinformationen an den Partner

  // bluetooth auslesen
  byte command = mate.receive();
  switch (command) {
    case 'h': // heartbeat
      if (mate.role > 0) start = true;
      break;
    case 's': // start
      start = true;
      break;
    case 'b': // brake
      start = false;
      if (digitalRead(SWITCH_KEEPER)) {
        m.brake(true);
      }
      break;
  }

  if (mate.timeout() || !mate.role) {
    m._role = 1;
  } else if (isTypeA) {
    if (seeBall && !mate.seeBall) m.setRusher();
    if (!seeBall && mate.seeBall) m.setKeeper();
    if (seeBall && mate.seeBall && abs(ballWidth - mate.ballWidth) >= 5) {
      if (ballWidth > mate.ballWidth) m.setRusher();
      if (ballWidth < mate.ballWidth) m.setKeeper();
    }
  } else {
    if (mate.role == 1) m._role = 2;
    if (mate.role == 2) m._role = 1;
  }

  if (!m.getMotEn()) m._role = 0;

  // Fahre
  if (!seeBall) {
    rotMulti = ROTATION_SIDEWAY;
  } else if (ballWidth > 100) {
    rotMulti = ROTATION_TOUCH;
  } else if (ballWidth > 40) {
    rotMulti = ROTATION_10CM;
  } else if (ballWidth > 20) {
    rotMulti = ROTATION_18CM;
  } else {
    rotMulti = ROTATION_AWAY;
  }

  driveState = "";
  driveOrientation = 0;
  if (isLifted) {
    // hochgehoben
    driveState = "lifted";
    drivePower = 0; // stoppe
    driveRotation = 0; // stoppe
  } else if (onLine) {
    // weiche einer Linie aus
    driveState = "avoid line";
    drivePower = SPEED_LINE;
  } else if (isHeadstart) {
    // führe einen Schnellstart aus
    driveState = "headstart";
  } else if (isDrift) {
    // steuere gegen
    driveState = "avoid drift";
    drivePower = SPEED_DRIFT;
    if (driftLeft) {
      driveDirection = 90;
    } else {
      driveDirection = -90;
    }
  } else {
    drivePower = SPEED;

    if (seeBall && !(!mate.timeout() && mate.seeBall && mate.ballWidth > ballWidth)) {
      // fahre in Richtung des Balls
      if (ball > 50) {
        debug("setLeft ");
        ballLeftTimer = millis();
      }
      if (ball < -50) {
        debug("setRight ");
        ballRightTimer = millis();
      }
      if (hasBall) {
        if (seeGoal && abs(heading < 20)) {
          drivePower = SPEED_DRIFT;
          driveState = "headstart";
        }
        driveDirection = constrain(map(goal, -X_CENTER, X_CENTER, 50, -50), -50, 50);
        driveOrientation = constrain(goal / 3 + heading, -ANGLE_GOAL_MAX, ANGLE_GOAL_MAX);
      } else {
        // verhindere das Driften
        if (ball > 0 && millis() - ballRightTimer < DRIFT_DURATION) {
          debug("runRight ");
          buzzerTone(500);
          driftTimer = millis();
          driftLeft = false;
        }
        if (ball < 0 && millis() - ballLeftTimer < DRIFT_DURATION) {
          debug("runLeft ");
          buzzerTone(500);
          driftTimer = millis();
          driftLeft = true;
        }
        // fahre in Richtung des Balls
        driveState = "follow";
        driveDirection = map(ball, -X_CENTER, X_CENTER, (float)rotMulti, -(float)rotMulti);
        if (driveDirection > 60) {
          // seitwärts bewegen, um Torsusrichtung aufrecht zu erhalten
          driveState = "sideway ri";
          driveDirection = 100;
          drivePower = SPEED_SIDEWAY;
        }
        if (driveDirection < -60) {
          // seitwärts bewegen, um Torsusrichtung aufrecht zu erhalten
          driveState = "sideway le";
          driveDirection = -100;
          drivePower = SPEED_SIDEWAY;
        }
        if (-15 < ball && ball < 15 && abs(heading) < 20) {
          // fahre geradeaus
          driveState = "straight";
          if (us.right() && us.left()) drivePower = map(constrain(min(us.left(), us.right()), 35, 65), 35, 65, 40, 120);
          driveOrientation = constrain(goal / 3 + heading, -ANGLE_GOAL_MAX, ANGLE_GOAL_MAX);
        } else if (ballWidth > 50) {
          if (us.right() && us.left()) drivePower = map(constrain(min(us.left(), us.right()), 35, 65), 35, 65, 40, 60);
        }
      }
    } else {
      // sehen den Ball nicht bzw. sollen ihn nicht sehen
      if (us.back() && us.back() < 50 && us.left() && us.right() && abs(heading) < 40) {
        // verteidige das Tor im Strafraum oder davor
        if (seeBall) {
          if (ball > 0 && !keeper.atGatepost()) keeper.left();
          if (ball < 0 && !keeper.atGatepost()) keeper.right();
        } else if (keeper.lastToggle() > 4000) {
          keeper.toggle();  // Richtungsänderung nach max. 4 Sekungen
        } else if (keeper.lastToggle() > 800 && keeper.atGatepost()) {
          keeper.toggle();  // Richtungsänderung am Torpfosten
        }

        keeper.set(); // übernehme die Steuerwerte
      } else {
        // fahre nach hinten
        if (!us.timeout() && us.back() && us.back() < 80) {
          driveState = "penalty";
          drivePower = SPEED_PENALTY;
        } else {
          driveState = "passive";
          drivePower = SPEED_BACKWARDS;
        }
        if (us.left() && us.left() < COURT_GOAL_TO_BORDER) driveDirection = -constrain(map(COURT_GOAL_TO_BORDER - us.left(), 0, 30, 180, 180 - ANGLE_PASSIVE_MAX), 180 - ANGLE_PASSIVE_MAX, 180);
        else if (us.right() && us.right() < COURT_GOAL_TO_BORDER) driveDirection = constrain(map(COURT_GOAL_TO_BORDER - us.right(), 0, 30, 180, 180 - ANGLE_PASSIVE_MAX), 180 - ANGLE_PASSIVE_MAX, 180);
        else driveDirection = 180;
      }
    }
  }

  drivePower = max(drivePower - abs(driveRotation), 0);
  driveRotation = ausrichten(driveOrientation);
  //driveRotation = ausrichten(0);
  if (isHeadstart) {
    for (int i = 0; i < 4; i++) {
      m.steerMotor(i, 255);
    }
  } else {
    m.drive(driveDirection, drivePower, driveRotation);
  }


  if (millis() - lastDisplay > 1000 || (d.getPage() == 3  && millis() - lastDisplay > 200)) {
    debug("display ");
    d.update();   // aktualisiere Bildschirm und LEDs
  }

  debugln();
}
