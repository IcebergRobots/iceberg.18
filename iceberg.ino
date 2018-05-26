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
bool start = false;         // ist der funkstart aktiviert
bool onLine = false;        // befinden wir uns auf einer Linie?
bool isHeadstart = false;   // fahren wir mit voller Geschwindigkeit?
bool isKeeperLeft = false;  // deckten wir zuletzt das Tor mit einer Linksbewegung?
byte role = 0;              // Spielrolle: Stürmer(2) / Torwart(1) / Aus(0)
int rotMulti;               // Scalar, um die Rotationswerte zu verstärken
int drivePower = 0;         // [-255 bis 255] aktuelle maximale Motorstärke
int driveRotation = 0;      // [-255 bis 255] aktuelle Rotationsstärke
int driveDirection = 0;     // [-180 bis 180] Ziel-Fahrrichtung
int driveOrientation = 0;   // [-180 bis 180] Ziel-Orientierungswinkel
int lineDir = -1;           // Richtung, in der ein Bodensensor ausschlug
unsigned long lineTimer = 0;        // Zeitpunkt des Interrupts durch einen Bodensensor
unsigned long headstartTimer = 0;   // Zeitpunkt des Betätigen des Headstarts
unsigned long flatTimer = 0;    // Zeitpunktm zu dem der Roboter das letzte mal flach auf dem Boden stand
String driveState = "";             // Zustand des Fahrens
Pilot m;  // OBJEKTINITIALISIERUNG
Player p;  // OBJEKTINITIALISIERUNG

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
bool wasStartButton = false; // war zuletzt der Funktstart aktiviert
unsigned long startTimer = 0; // Zeitpunkt des letzten Start Drückens
unsigned long bluetoothTimer = 0; // Zeitpunkt des letzten Sendens
Mate mate;  // OBJEKTINITIALISIERUNG

// Globale Definition: WICHTUNG DER PID-REGLER
double pidSetpoint; // Nulllevel [-180 bis 180] Winkel des Tours
double pidIn;       // Kompasswert [-180 bis 180]
double pidOut;      // Rotationsstärke [-255 bis 255]
PID myPID = PID(&pidIn, &pidOut, &pidSetpoint, PID_FILTER_P, PID_FILTER_I, PID_FILTER_D, DIRECT); // OBJEKTINITIALISIERUNG

// Globale Definition: BATTERY
byte batState = 0;  // ist du Spannung zu gering?
int batVol = 0;     // Spannung MAL 10!

// Globale Definition: PIXY
bool seeBall = false;   // sehen wir den Ball?
bool seeGoal = false;   // sehen wir das Tor?
bool seeEast = false;   // sehen wir eine Farbmarkierung nach rechts
bool seeWest = false;   // sehen wir eine Farbmarkierung nach links
bool closeBall = false; // ist der Ball groß
bool isDrift = false;   // driften wir
bool driftLeft = false; // steuern wir nach links gegen
byte pixyState = 0;     // Verbindungsstatus per Pixy
byte blockCount = 0;    // Anzahl der gesehenen Blöcke
byte blockCountBall = 0;// Anzahl der Ball Blöcke
byte blockCountGoal = 0;// Anzahl der Tor Blöcke
byte blockCountEast = 0;// Anzahl der Color Code Blöcke
byte blockCountWest = 0;// Anzahl der Color Code Blöcke
int ball = 0;           // Abweichung der Ball X-Koordinate
int ballWidth = 0;      // Ballbreite
int ballArea = 0;       // Ballgröße (Flächeninhalt)
int goal = 0;           // Abweichung der Tor X-Koordinate
int goalWidth = 0;      // Torbreite
int goalArea = 0;       // Torgröße (Flächeninhalt)
int east = 0;           // Abweichung des Farbmarkierungs X-Koordinate
int eastHeight = 0;     // Farbmarkierungshöhe
int west = 0;           // Abweichung des Farbmarkierungs X-Koordinate
int westHeight = 0;     // Farbmarkierungshöhe
unsigned long seeBallTimer = 0;   // Zeitpunkt des letzten Ball Sehens
unsigned long seeGoalTimer = 0;   // Zeitpunkt des letzen Tor Sehens
unsigned long seeEastTimer = 0;   // Zeitpunkt der letzen Ost Sehens
unsigned long seeWestTimer = 0;   // Zeitpunkt der letzen West Sehens
unsigned long closeBallTimer = 0; // Zeitpunkt des letzten großen Balls
unsigned long driftTimer = 0;     // Zeitpunkt seit wann wir gegensteuern
unsigned long ballLeftTimer = 0;  // Zeitpunkt wann der Ball zuletzt links war
unsigned long ballRightTimer = 0; // Zeitpunkt wann der Ball zuletzt rechts war
unsigned long pixyResponseTimer = 0;  // Zeitpunkt der letzten Antwort der Pixy
unsigned long pixyTimer = 0;  // Zeitpunkt des letzten Auslesens der Pixy
Pixy pixy;  // OBJEKTINITIALISIERUNG

// Globale Definition: ULTRASCHALL
bool isPenaltyFree = true;
unsigned long penaltyFreeTimer = 0;
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

// Globale Definition: LEDS, DEBUG
bool wasLedButton = false;        // war der Animationsknopf gedrückt
bool hasDebugHead = false;        // wurden bereits die Metadaten gesendet
bool isSetupAnimantion = true;    // läuft die Setup Animation;
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
bool wasMenuButton = false; // war der Menü-Knopf gedrückt?

//###################################################################################################

void setup() {
  pinModes();
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
  isTypeA = digitalRead(TYPE); // lies den Hardware Jumper aus

  // setzte Pins und Winkel des Pilot Objekts
  d.setupMessage(2, "MOTOR", "setPins");
  setupMotor();

  // initialisiere Kamera
  d.setupMessage(3, "PIXY", "Kamera");
  pixy.init();

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
  if (!silent) led.start();
  d.setupMessage(10, "B: " + String(lightBarrierTriggerLevel), "");
  DEBUG_SERIAL.println();
  DEBUG_SERIAL.println("ICEBERG ROBOTS");
  DEBUG_SERIAL.println("-=-=-=-=-=-=-=-");
  if (silent) DEBUG_SERIAL.println("DEAKTIVIERT");
  else {
    DEBUG_SERIAL.println("AKTIVIERT");
    DEBUG_SERIAL.print(boolToSign(DEBUG_STATE));
    DEBUG_SERIAL.println("ROLLENSTATUS");
    DEBUG_SERIAL.print(boolToSign(DEBUG_FUNCTIONS));
    DEBUG_SERIAL.println("TÄTIGKEITEN");
    DEBUG_SERIAL.print(boolToSign(DEBUG_LOOP));
    DEBUG_SERIAL.println("TICK");
  }
  DEBUG_SERIAL.println("-=-=-=-=-=-=-=-");

  // sorge dafür, dass alle Timer genügend Abstand haben
  while (millis() < 500) {}
}
//###################################################################################################

void loop() {
  hasDebugHead = false;
  if (DEBUG_LOOP) debug();
  displayDebug = "";

  readCompass();
  calculateStates();  // Berechne alle Statuswerte und Zustände

  if (millis() - kickTimer > map(analogRead(POTI), 0, 1023, 0, 35)) digitalWrite(SCHUSS, 0); // schuß wieder ausschalten

  if (batState == 3) analogWrite(BUZZER, 127 * (millis() % 250 < 125));
  else analogWrite(BUZZER, 127 * millis() <= buzzerStopTimer);  // buzzer anschalten bzw. wieder ausschalten

  while (BOTTOM_SERIAL.available() > 1) {
    BOTTOM_SERIAL.read();
  }

  // Seitenauswahl
  // auswählen
  if (!digitalRead(ROTARY_BUTTON) && !wasMenuButton) {
    d.toggle();
  }
  wasMenuButton = !digitalRead(ROTARY_BUTTON);
  // zurück
  if (!digitalRead(BUTTON_3)) reset();

  // drehen
  rotaryEncoder.tick(); // erkenne Reglerdrehungen
  if (rotaryEncoder.getPosition() != rotaryPositionLast) {
    d.change(rotaryEncoder.getPosition() - rotaryPositionLast);
  }
  rotaryPositionLast = rotaryEncoder.getPosition();

  if (!digitalRead(BUTTON_1) && !wasLedButton) {
    isSetupAnimantion = false;
    if (led.isAnimation()) led.cancel(); // beende die Animation
    else led.start();                   // starte die Animation
  }
  else if (!digitalRead(BUTTON_1) && wasLedButton && led.isAnimation() && led.lastAnimation() > 1000) led.hymne();
  wasLedButton = !digitalRead(BUTTON_1);
  if (isSetupAnimantion && millis() > 3000) {
    isSetupAnimantion = false;
    led.cancel();
  }

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

  if (!digitalRead(SCHUSS_BUTTON)) kick(); // schieße

  if (led.isAnimation() || millis() - ledTimer > 100) {
    if (DEBUG_FUNCTIONS) debug("led");
    led.set();  // Lege Leds auf Statusinformation fest
    led.led();  // Aktualisiere alle Leds bzw. zeige die Animation
  }

  if (millis() - pixyTimer > 30) {
    if (DEBUG_FUNCTIONS) debug("pixy");
    readPixy(); // aktualisiere Pixywerte (max. alle 50ms)
  }

  if (millis() - usTimer > 100) us.receive(); // lese die Ultraschall Sensoren aus (max. alle 100ms)

  // remote start
  if (!digitalRead(BIG_BUTTON)) {
    if (!wasStartButton) startTimer = millis();
    if (millis() - startTimer < 100) {
      start = true;
      led.cancel();
      if (digitalRead(SWITCH_B)) headstartTimer = millis();
    } else if (millis() - startTimer > 1000) {
      byte data[1] = {'b'};
      mate.send(data, 1);
      m.brake(true);
      start = false;
    }
  }
  bool tempMotor = m.getMotEn();
  // starte über Funk wenn Schalter Keeper aktiviert
  if (!digitalRead(SWITCH_MOTOR)) {
    m.setMotEn(start);
  } else {
    m.setMotEn(false);
    start = false;
  }
  if (millis() - bluetoothTimer > 100 || (!tempMotor && m.getMotEn()))  transmitHeartbeat(); // Sende einen Herzschlag mit Statusinformationen an den Partner
  wasStartButton = !digitalRead(BIG_BUTTON);


  // bluetooth auslesen
  byte command = mate.receive();
  switch (command) {
    case 'h': // heartbeat
      if (mate.getMotEn()) {
        start = true;
        led.cancel();
        if (!wasStartButton && digitalRead(SWITCH_B) && !m.getMotEn()) headstartTimer = millis();
      }
      break;
    case 'b': // brake
      start = false;
      m.brake(true);
      break;
  }

  if (mate.timeout() || !mate.getMotEn()) p.setKeeper(true);
  else if (isTypeA) {
    if (seeBall && !mate.seeBall) p.setRusher(false);
    if (!seeBall && mate.seeBall) p.setKeeper(false);
    if (seeBall && mate.seeBall && abs(ballWidth - mate.ballWidth) >= 5) {
      if (ballWidth > mate.ballWidth) p.setRusher(false);
      if (ballWidth < mate.ballWidth) p.setKeeper(false);
    }
  } else {
    if (mate.isKeeper()) p.setRusher(true);
    if (mate.isRusher()) p.setKeeper(true);
  }

  // Fahre
  if (isLifted) {
    // hochgehoben
    driveState = "lifted";
    m.brake(false);
  } else if (onLine) {
    // weiche einer Linie aus
    driveState = "line";
    m.drive(driveDirection, SPEED_LINE, 0);
  } else if (isHeadstart) {
    // führe einen Schnellstart aus
    driveState = "headstart";
    for (int i = 0; i < 4; i++) {
      m.steerMotor(i, 255);
    }
  } else if (isDrift) {
    // steuere gegen
    driveState = "drift";
    drivePower = SPEED_DRIFT;
    if (driftLeft) {
      driveDirection = 90;
    } else {
      driveDirection = -90;
    }
  } else {
    p.play();
  }

  if (millis() - lastDisplay > 1000 || (d.getPage() == 3  && millis() - lastDisplay > 200) || (!tempMotor && m.getMotEn())) {
    if (DEBUG_FUNCTIONS) debug("display");
    d.update();   // aktualisiere Bildschirm und LEDs
  }

  if (hasDebugHead) debugln();
}
