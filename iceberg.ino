#include "Config.h"
#include "Pin.h"
#include "Pilot.h"
#include "Utility.h"

#include <SPI.h>
#include <Pixy.h>
#include <Wire.h>
#include <EEPROM.h>
#include <RotaryEncoder.h>
#include <PID_v1.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_9DOF.h>
#include <Adafruit_L3GD20_U.h>

// Einstellungen: FAHREN
int rotMulti;           // Scalar, um die Rotationswerte zu verstärken
bool start = false;     // ist der funkstart aktiviert
int drivePwr = 0;       // maximale Motorstärke [0 bis 255]
int driveRot = 0;       // korrigiere Kompass
int driveDir = 0;       // Zielrichtung
int lineDir = -1;       // Richtung, in der ein Bodensensor ausschlug
unsigned long lineTimer = 0;      // Zeitpunkt des Interrupts durch einen Bodensensor
unsigned long headstartTimer = 0; // Zeitpunkt des Betätigen des Headstarts
bool headstartStraigth = true;  // fahren wir genau gerade aus oder leicht nacht links?
bool isKeeperLeft = false; // deckten wir zuletzt das Tor mit einer Linksbewegung?
unsigned long lastKeeperToggle = 0; // Zeitpunkt des letzten Richtungswechsel beim Tor schützen
Pilot m;                // OBJEKTINITIALISIERUNG

// Einstellungen: KOMPASS
int heading = 0;                    // Wert des Kompass
int startHeading = 0;               // Startwert des Kompass
int rotation = 0;                   // rotationswert für die Motoren
Adafruit_9DOF                 dof   = Adafruit_9DOF();
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);
sensors_event_t accel_event;
sensors_event_t mag_event;
sensors_vec_t   orientation;

// Einstellungen: BLUETOOTH
byte cache[CACHE_SIZE]; // Zwischenspeicher für eingehende Bluetooth Nachrichten
byte cacheIndex = 255;  // aktuelle Schreibposition im Zwischenspeicher
bool startLast = false; // war zuletzt der Funktstart aktiviert
unsigned long startTimer = 0; // Zeitpunkt des letzten Start Drückens
unsigned long bluetoothTimer = 0; // Zeitpunkt des letzten Sendens
unsigned long heartbeatTimer = 0; // Zeitpunkt des letzten empfangenen Heartbeat

// Einstellungen: WICHTUNG DER PID-REGLER
double pidSetpoint;       // Nulllevel [-180 bis 180]:Winkel des Tours
double pidIn;             // Kompasswert [-180 bis 180]
double pidOut;            // Rotationsstärke [-255 bis 255]
PID myPID(&pidIn, &pidOut, &pidSetpoint, PID_FILTER_P, PID_FILTER_I, PID_FILTER_D, DIRECT); // OBJEKTINITIALISIERUNG

// Einstellungen: BATTERY
int batVol = 0;       // Spannung MAL 10!
bool batLow = false;  // ist du Spannung zu gering?

// Einstellungen: PIXY
int ball = 0;           // Abweichung der Ball X-Koordinate
int ballWidth = 0;      // Ballbreite
int ballSize = 0;       // Ballgröße (Flächeninhalt)
int goal = 0;           // Abweichung der Tor X-Koordinate
int goalWidth = 0;      // Torbreite
int goalSize = 0;       // Torgröße (Flächeninhalt)
unsigned long seeBallTimer = 0; // Zeitpunkt des letzten Ball Sehens
unsigned long seeGoalTimer = 0; // Zeitpunkt des letzen Tor Sehens
bool seeBall = false;      // sehen wir den Ball?
bool seeGoal = false;      // sehen wir das Tor?
unsigned long ballRightTimer = 0; // Zeitpunkt der letzten Ballsicht über 15°
unsigned long ballLeftTimer = 0;  // Zeitpunkt der letzten Ballsicht unter -15
unsigned long pixyTimer = 0;  // Zeitpunkt des letzten Auslesens der Pixy
Pixy pixy;                    // OBJEKTINITIALISIERUNG

// Einstellungen: US
byte us[] = {0, 0, 0, 0};   // Werte des US-Sensors
unsigned long usTimer = 0;  // wann wurde der Us zuletzt ausgelesen?

// Einstellungen: KICK
unsigned long kickTimer = 0;  // Zeitpunkt des letzten Schießens

// Einstellungen: DISPLAY
unsigned long lastDisplay = 0; // Zeitpunkt des letzten Displayaktualisierens
String displayDebug = "";      // unterste Zeile des Bildschirms;
Adafruit_SSD1306 d(PIN_4);     // OBJEKTINITIALISIERUNG

// Einstellungen: STATUS-LEDS & LED-MATRIX
bool stateFine = true;  // liegt kein Fehler vor?
bool hasBall = false;   // besitzen der Roboter den Ball?
bool showBottom = true; // sollen die Boden-Leds an sein?

// Einstellungen: BUZZER
unsigned long buzzerStopTimer = 0; // Zeitpunkt, wann der Buzzer ausgehen soll

// Einstellungen: ROTARY-ENCODER
RotaryEncoder rotaryEncoder(ROTARY_B, ROTARY_A);  // OBJEKTINITIALISIERUNG
int rotaryPosition = 0; // Zustand, der vom Regler eingestellt ist

// Einstellungen: MATE
bool isConnected = false; // besteht eine Bluetooth Verbindung zum Parter
bool seeBallMate = false;
int ballMate = 0;
unsigned int ballWidthMate = 0;
byte usMate[] = {0, 0, 0, 0};

// Einstellungen: DEBUG
bool isTypeA; // ist das Roboter A?
String messageLog = ""; // Protokoll der Display-Benachrihtigungen

Adafruit_NeoPixel bottom = Adafruit_NeoPixel(16, BODEN_LED, NEO_GRB + NEO_KHZ800);  // OBJEKTINITIALISIERUNG (BODEN-LEDS)
Adafruit_NeoPixel matrix = Adafruit_NeoPixel(12, MATRIX_LED, NEO_GRB + NEO_KHZ800); // OBJEKTINITIALISIERUNG (LED-MATRIX)
Adafruit_NeoPixel info = Adafruit_NeoPixel(3, INFO_LED, NEO_GRB + NEO_KHZ800);      // OBJEKTINITIALISIERUNG (STATUS-LEDS)

//###################################################################################################

void setup() {
  // Roboter bremst aktiv
  m.brake(true);

  // Start der Seriellen Kommunikation
  DEBUG_SERIAL.begin(115200);
  BLUETOOTH_SERIAL.begin(115200);
  US_SERIAL.begin(115200);
  BOTTOM_SERIAL.begin(115200);
  Wire.begin();         // Start der I2C-Kommunikation

  // weiche den Linien aus
  attachInterrupt(digitalPinToInterrupt(INT_BODENSENSOR), avoidLine, RISING);

  // initialisiere Display mit Iceberg Schriftzug
  setupDisplay();

  // setzte die PinModes
  displayMessage("1/9 Pins");
  pinModes();
  isTypeA = digitalRead(TYPE); // lies den Hardware Jumper aus

  // setzte Pins und Winkel des Pilot Objekts
  displayMessage("2/9 Motoren");
  setupMotor();

  // initialisiere Kamera
  displayMessage("3/9 Pixy");
  pixy.init();
  pixy.setLED(0, 0, 0); // schalte die Hauptled der Pixy aus

  // lies EEPROM aus
  displayMessage("4/9 EEPROM");
  if (EEPROM.read(0) == 0) {
    startHeading = EEPROM.read(1);
  } else {
    startHeading = -EEPROM.read(1);
  }

  // initialisiere Beschleunigungssensor
  displayMessage("5/9 Accel");
  if (!accel.begin()) {
    stateFine = false;
    displayMessage("5/9 Accel failed");
  }

  // initialisiere Magnetfeldsensor
  displayMessage("6/9 Mag");
  if (!mag.begin()) {
    stateFine = false;
    displayMessage("6/9 Mag failed");
  }
  delay(1);

  // initialisiere Kompasssensor
  displayMessage("7/9 Kompass");
  mag.enableAutoRange(true);  // aktiviere automatisches Messen
  heading = getCompassHeading();  // lies Kompassrichtung aus

  // initialisiere PID-Regler
  displayMessage("8/9 PID");
  pidSetpoint = 0;
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(-255, 255);

  // initialisiere Leds
  displayMessage("9/9 Leds");
  bottom.begin();   // BODEN-LEDS initialisieren
  matrix.begin();   // MATRIX-LEDS initialisieren
  info.begin();     // STATUS-LEDS initialisieren

  displayMessage("setup done");
  debugln("setup done");

  // sorge dafür, dass alle Timer genügend Abstand haben
  while (millis() < 200) {}
}

//###################################################################################################

void loop() {
  //debug("[" + String(millis()) + "]");
  rotaryEncoder.tick(); // erkenne Reglerdrehungen

  // starte über Funk wenn Schalter Keeper aktiviert
  if (digitalRead(SWITCH_KEEPER)) {
    displayDebug = "on";
  } else {
    displayDebug = "off";
  }
  if (!digitalRead(SWITCH_MOTOR)) {
    m.setMotEn(!digitalRead(SWITCH_KEEPER) || start);
  } else {
    m.setMotEn(false);
    start = false;
  }

  // schuß wieder ausschalten
  if (millis() - kickTimer > 30) {
    digitalWrite(SCHUSS, 0);
  }

  // buzzer anschalten bzw. wieder ausschalten
  digitalWrite(BUZZER_AKTIV, millis() <= buzzerStopTimer);

  // schneller vorstoß gerade
  if (!digitalRead(BUTTON_1)) {
    headstartStraigth = false;
    headstartTimer = millis() + 1500;
  }

  // schneller vorstoß nacht links
  if (!digitalRead(BUTTON_3)) {
    headstartStraigth = true;
    headstartTimer = millis() + 1500;
  }

  // regler auslesen
  rotaryEncoder.tick(); // erkenne Reglerdrehungen
  if (!digitalRead(ROTARY_BUTTON)) {
    rotaryEncoder.setPosition(0); // setze Regler zurück
    buzzerTone(50);
  }
  rotaryPosition = (ROTARY_RANGE + (rotaryEncoder.getPosition() % ROTARY_RANGE)) % ROTARY_RANGE;  // wandle Drehposition in Zustand von 0 bis ROTARY_RANGE um

  // Torrichtung speichern
  if (!digitalRead(BUTTON_2)) {
    startHeading = 0;
    startHeading = getCompassHeading(); //merke Torrichtung [-180 bis 179]
    EEPROM.write(0, startHeading < 0);  // speichere Vorzeichen
    EEPROM.write(1, abs(startHeading)); // speichere Winkel
    heading = 0;
    buzzerTone(200);
  }

  rotaryEncoder.tick(); // erkenne Reglerdrehungen

  // ermittle Statuswerte für Leds
  hasBall = analogRead(LIGHT_BARRIER) > LIGHT_BARRIER_TRIGGER_LEVEL;
  seeBall = millis() - seeBallTimer < 50;
  isConnected = millis() - heartbeatTimer < 500;
  batVol = analogRead(BATT_VOLTAGE) * 0.1220703;  // SPANNUNG MAL 10!
  if (batVol > 40) {
    if (m.getMotEn()) {
      batLow = batVol < 98;
    } else {
      batLow = batVol < 108;
    }
  } else {
    batLow = false;
  }
  if (batLow) {
    buzzerTone(20);
    displayMessage("lowVoltage");
  }

  // zeige Statuswerte an
  showLed(info, 0, stateFine);
  showLed(info, 1, !batLow);
  showLed(info, 2, millis() % 1000 < 200, true);

  showLed(matrix, 1, m.getMotEn());
  showLed(matrix, 2, seeBall);
  showLed(matrix, 3, hasBall);
  showLed(matrix, 4, isConnected);
  showLed(matrix, 11, ballWidth > 15);

  // schieße
  if ((seeGoal && abs(goal < 100) && hasBall) || !digitalRead(SCHUSS_BUTTON)) {
    kick();
  }

  // konfiguriere Boden Leds
  for (byte i = 0; i < 16; i++) {
    if (!digitalRead(SWITCH_BODENS)) {
      bottom.setPixelColor(i, 0, 0, 0);
    } else if (!digitalRead(SWITCH_A)) {
      bottom.setPixelColor(i, 255, 0, 0);
    } else {
      bottom.setPixelColor(i, 255, 255, 255);
    }
  }
  bottom.show();

  rotaryEncoder.tick(); // erkenne Reglerdrehungen

  // aktualisiere Pixywerte (max. alle 50ms)
  if (millis() - pixyTimer > 50) {
    readPixy();
  }

  rotaryEncoder.tick(); // erkenne Reglerdrehungen

  // lese die Ultraschall Sensoren aus (max. alle 100ms)
  if (millis() - usTimer > 100) {
    getUs();
    usTimer = millis();
  }

  rotaryEncoder.tick(); // erkenne Reglerdrehungen

  // remote start
  debugln(start);
  if (!digitalRead(BIG_BUTTON)) {
    if (!startLast || millis() - startTimer < 100) {
      byte data[1] = {'s'};
      sendBluetooth(data, 1);
      start = true;
    } else if (millis() - startTimer > 1000) {
      byte data[1] = {'b'};
      sendBluetooth(data, 1);
      m.brake(true);
      start = false;
    }
    startLast = true;
  } else {
    startLast = false;
    startTimer = millis();
  }

  // bluetooth senden
  if (millis() - bluetoothTimer > 100) {
    bluetoothTimer = millis();
    byte data[9];
    data[0] = 'h';
    if (!m.getMotEn()) {
      data[1] = 253;  // pause: 253
    } else if (!seeBall) {
      data[1] = 2;    // ball blind: 2
    } else {
      data[1] = ball < 0;
      // ball < 0: 0
      // ball >= 0: 1
    }
    data[2] = abs(ball);
    data[3] = ballWidth % 254;
    data[4] = ballWidth / 254;
    data[5] = us[0];
    data[6] = us[1];
    data[7] = us[2];
    data[8] = us[3];
    sendBluetooth(data, 9); // heartbeat
  }

  // bluetooth auslesen
  byte messageLength = receiveBluetooth();
  if (messageLength > 0) {
    debug("[" + String(millis()) + "]");
    for (int i = 0; i < messageLength; i++) {
      debug(String(cache[i]) + ".");
    }
    debugln();
    switch (cache[0]) {
      case 104: // heartbeat
        if (messageLength == 9) {
          heartbeatTimer = millis();
          if (cache[1] < 3) {
            start = true;
            if (cache[1] == 2) {
              seeBallMate = false;
            } else {
              seeBallMate = true;
              if (cache[2] != 255) {
                ballMate = -(cache[1] == 1) * cache[2];
              }
            }
          }
          if (cache[3] != 255 && cache[4] != 255) ballWidthMate = cache[3] + 254 * cache[4];
          if (cache[5] != 255) usMate[0] = cache[5];
          if (cache[6] != 255) usMate[1] = cache[6];
          if (cache[7] != 255) usMate[2] = cache[7];
          if (cache[8] != 255) usMate[3] = cache[8];
        }
        break;
      case 115: // start
        start = true;
        break;
      case 98: // brake
        start = false;
        if (digitalRead(SWITCH_KEEPER)) {
          m.brake(true);
        }
        break;
    }
  }

  rotaryEncoder.tick(); // erkenne Reglerdrehungen

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

  rotaryEncoder.tick(); // erkenne Reglerdrehungen
  //displayDebug = String(rotMulti) + "," + String(ballWidth);
  driveRot = ausrichten();

  rotaryEncoder.tick(); // erkenne Reglerdrehungen

  if ((lineDir >= 0 && millis() - lineTimer < 20) || millis() <= headstartTimer) {
    drivePwr = 255;
    if (millis() <= headstartTimer) {
      driveDir = 0;
      if (!headstartStraigth) {
         driveRot = 5;
      }
    }
  } else {
    //drivePwr = map(analogRead(POTI), 0, 1023, 0, 255) - abs(heading);
    if (seeBall && !(isConnected && seeBallMate && ballWidthMate > ballWidth)) {
      // merke Zeitpunkte, wenn sich Ball rechts oder links befindet
      if (ball > 50) {
        ballRightTimer = millis();
      }
      if (ball < -50 ) {
        ballLeftTimer = millis();
      }
      /*
        // gegensteuern, um zu verhindern, dass man am Ball vorbeidriftet
        if (millis() - ballRightTimer < 500 && ball < 10) {
        rotMulti = ROTATION_TOUCH;
        drivePwr = SPEED_AVOID_DRIFT;
        }
        if (millis() - ballLeftTimer < 500 && ball > 10) {
        rotMulti = ROTATION_TOUCH;
        drivePwr = SPEED_AVOID_DRIFT;
        }
      */
      // seitwärts bewegen, um Torsusrichtung aufrecht zu erhalten
      if (ball > 100) {
        // fahre seitwärts nach links
        driveDir = -ANGLE_SIDEWAY;
        drivePwr = SPEED_SIDEWAY;
      } else if (ball < -100) {
        // fahre seitwärts nach rechts
        driveDir = ANGLE_SIDEWAY;
        drivePwr = SPEED_SIDEWAY;
      } else {
        if (hasBall) {
          driveDir = constrain(map(goal, -X_CENTER, X_CENTER, 50, -50), 50, -50);
        } else {
          // fahre in Richtung des Balls
          driveDir = constrain(map(ball, -X_CENTER, X_CENTER, (float)rotMulti, -(float)rotMulti), -120, 120);
          if (-15 < ball && ball < 15) {
            // fahre geradeaus
            drivePwr = SPEED_BALL_IN_FRONT;
          } else {
            // drehe dich zum Ball
            drivePwr = SPEED;
          }
        }
      }
    } else {
      // fahre nach hinten
      driveDir = 180;
      drivePwr = SPEED_BACKWARDS;

      if (us[3] < 50 && us[3] > 0 && abs(heading) < 40) {
        drivePwr = SPEED_KEEPER;
        byte usRight = us[0];
        byte usLeft = us[2];
        if (usRight == 0 && usLeft == 0) {
          // beide Ultraschallsensoren kaputt
          stateFine = false;
        } else {
          if (usRight == 0) {
            usRight = COURT_WIDTH - usLeft; // ersetze kaputte US-Sensoren mit sinvollen Werten
          }
          if (usLeft == 0) {
            usLeft = COURT_WIDTH - usRight;  // ersetze kaputte US-Sensoren mit sinvollen Werten
          }
          if (millis() - lastKeeperToggle > 3000) {
            // toggle
            if (isKeeperLeft) {
              driveDir = -ANGLE_SIDEWAY;
              isKeeperLeft = false;
              lastKeeperToggle = millis();
            } else {
              driveDir = ANGLE_SIDEWAY;
              isKeeperLeft = true;
              lastKeeperToggle = millis();
            }
          } else if (millis() - lastKeeperToggle > 1500) {
            if (isKeeperLeft) {
              // wir fahren gerade nach links
              if (usLeft < COURT_GOAL_TO_BORDER) {
                driveDir = -ANGLE_SIDEWAY;
                isKeeperLeft = false;
                lastKeeperToggle = millis();
              }
            } else {
              // wir fahren gerade nach rechts
              if (usRight < COURT_GOAL_TO_BORDER) {
                driveDir = ANGLE_SIDEWAY;
                isKeeperLeft = true;
                lastKeeperToggle = millis();
              }
            }
          } else {
            if (isKeeperLeft) {
              driveDir = ANGLE_SIDEWAY;
            } else {
              driveDir = -ANGLE_SIDEWAY;
            }
          }
        }
      }

    }
    if (ballWidth > 32) {
      drivePwr /= 2;
    }
  }
  drivePwr = max(drivePwr - abs(driveRot), 0);

  rotaryEncoder.tick(); // erkenne Reglerdrehungen

  if (millis() - lineTimer > 50) {
    m.drive(driveDir, drivePwr, driveRot);
  }

  rotaryEncoder.tick(); // erkenne Reglerdrehungen

  //displayDebug = driveRot;
  // aktualisiere Bildschirm und LEDs
  if (millis() - lastDisplay > 40) {
    updateDisplay();
  }

  rotaryEncoder.tick(); // erkenne Reglerdrehungen
  //debugln();

}

//###################################################################################################

// Piloten konfigurieren
void setupMotor() {
  m.setAngle(70);

  m.setPins(0, FWD0, BWD0, PWM0);
  m.setPins(1, FWD1, BWD1, PWM1);
  m.setPins(2, FWD2, BWD2, PWM2);
  m.setPins(3, FWD3, BWD3, PWM3);
}

// Bildschirm konfigurieren und Startschriftzug zeigen
void setupDisplay() {
  d.begin(SSD1306_SWITCHCAPVCC, 0x3C);  //Initialisieren des Displays
  d.clearDisplay();     //leert das Display
  d.setTextSize(2);     //setzt Textgroesse
  d.setTextColor(WHITE);//setzt Textfarbe
  d.setCursor(0, 0);    //positioniert Cursor
  d.println("ICEBERG");  //schreibt Text auf das Display
  d.println("ROBOTS");
  d.display();          //wendet Aenderungen an
}

void displayMessage(String str) {
  messageLog += START_MARKER + String(str) + END_MARKER + "\n";
  d.setCursor(3, 46);
  d.fillRect(3, 46, 123, 16, false); // lösche das Textfeld
  d.setTextSize(2);     //setzt Textgroesse
  d.println(str.substring(0, 10));
  d.display();
}

// Infos auf dem Bildschirm anzeigen
void updateDisplay() {
  String myTime = "";
  int min = numberOfMinutes(millis());
  if (min < 10) {
    myTime += "0";
  }
  myTime += String(min) + ":";
  int sec = numberOfSeconds(millis());
  if (sec < 10) {
    myTime += "0";
  }
  myTime += String(sec);

  d.clearDisplay();
  d.setTextColor(WHITE);
  d.setTextSize(1);
  d.setCursor(3, 3);
  if (isTypeA) {
    d.println("IcebergRobotsA " + myTime);
  } else {
    d.println("IcebergRobotsB " + myTime);
  }
  if (heading < -135) { // zeige einen Punkt, der zum Tor zeigt
    d.drawRect(map(heading, -180, -134, 63 , 125), 61, 2, 2, WHITE); //unten (rechte Hälfte)
  } else if (heading < -45) {
    d.drawRect(125, map(heading, -135, -44, 61, 0), 2, 2, WHITE); //rechts
  } else if (heading < 45) {
    d.drawRect(map(heading, -45, 44, 125, 0), 0, 2, 2, WHITE); //oben
  } else if (heading < 135) {
    d.drawRect(0, map(heading, 45, 134, 0, 61), 2, 2, WHITE); //links
  } else if (heading < 180) {
    d.drawRect(map(heading, 135, 179, 0, 62), 61, 2, 2, WHITE); //unten (linke Hälfte)
  }
  d.setTextSize(2);
  d.setCursor(3, 14);
  if (seeBall) {
    d.println("Ball");
    d.setCursor(50, 20);
    d.setTextSize(1);
    d.println(ball);
    d.drawLine(91, 27, constrain(map(ball, -160, 160, 60, 123), 60, 123), 14, WHITE);
  } else {
    d.println("Ball:blind");
  }
  d.setTextSize(2);
  d.drawLine(3, 11, map(drivePwr, 0, 255, 3, 123), 11, WHITE);

  String name1 = "";
  String name2 = "";
  String value1 = "";
  String value2 = "";

  switch (rotaryPosition) {
    case 0:
      name1 = "Dir:";
      name2 = String(displayDebug);
      value1 = String(driveDir);
      break;
    case 1:
      name1 = "^";
      value1 = ">";
      name2 = "<";
      value2 = "v";
      d.setCursor(21, 30);
      d.print(us[1] + String("   ").substring(0, 3 - String(us[1]).length() ));
      d.print(String("    ").substring(0, 4 - String(us[0]).length()) + String(us[0]) );
      d.setCursor(21, 46);
      d.print(us[2] + String("   ").substring(0, 3 - String(us[2]).length() ));
      d.print(String("    ").substring(0, 4 - String(us[3]).length()) + String(us[3]));
      break;
    case 2:
      name1 = "dPwr:";
      value1 = intToStr(drivePwr);   // drive power
      name2 = "dRot:";
      value2 = intToStr(driveRot);   // drive rotation
      break;
    case 3:
      name1 = "rotMp:";
      value1 = intToStr(rotMulti);  // ratation multiplier
      name2 = "balX:";
      value2 = intToStr(ball);   // ball angle
      break;
    case 4:
      name1 = "t:";
      value1 = String(millis()); // ratation multiplier
      name2 = "headi:";
      value2 = intToStr(heading);  // heading
      break;
    case 5:
      name1 = "batVo:";
      value1 = String(batVol / 10) + "." + String(batVol % 10); // bluetooth command
      name2 = "start:";
      value2 = String(start);      // start
      break;
    case 6:
      name1 = "bWid:";
      value1 = String(ballWidth);  // ball box width
      name2 = "bSiz:";
      value2 = String(ballSize);  // ball box height*width
      break;
    case 7:
      name1 = "gWid";
      value1 = String(goalWidth);
      name2 = "gSiz";
      value2 = String(goalSize);
      break;
    case 8:
      name1 = "gX:";
      value1 = intToStr(goal);
      // 3 - 123
      int goalLeft;
      goalLeft = X_CENTER + goal - goalWidth / 2;
      goalLeft = constrain(map(goalLeft, PIXY_MIN_X, PIXY_MAX_X, 3, 123), 3, 123);
      d.fillRect(goalLeft, 46, constrain(map(goalWidth, 0, PIXY_MAX_X - PIXY_MIN_X, 0, 123), 0, 123), 32, true); // zeige die Torbreite
      break;
    case 9:
      name1 = "Mball:";
      if (seeBallMate) {
        value1 = intToStr(ballMate);
      } else {
        value1 = "blind";
      }
      name2 = "Mwid:";
      value2 = String(ballWidthMate);
      break;
    case 10:
      name1 = "^";
      value1 = ">";
      name2 = "<";
      value2 = "v";
      d.setCursor(21, 30);
      d.print(usMate[1] + String("   ").substring(0, 3 - String(usMate[1]).length() ));
      d.print(String("M   ").substring(0, 4 - String(usMate[0]).length()) + String(usMate[0]) );
      d.setCursor(21, 46);
      d.print(usMate[2] + String("   ").substring(0, 3 - String(usMate[2]).length() ));
      d.print(String("M   ").substring(0, 4 - String(usMate[3]).length()) + String(usMate[3]));
      break;
  }

  bool seeBallMate = false;
  int ballMate = 0;
  int ballWidthMate = 0;
  byte usMate[] = {0, 0, 0, 0};


  name1 += String("          ").substring(0, max(0, 10 - name1.length() - value1.length()));
  name1 = String(name1 + value1).substring(0, 10);
  name2 += String("          ").substring(0, max(0, 10 - name2.length() - value2.length()));
  name2 = String(name2 + value2).substring(0, 10);
  d.setCursor(3, 30);
  d.println(name1);
  d.setCursor(3, 46);
  d.println(name2);

  d.invertDisplay(m.getMotEn());
  d.display();      // aktualisiere Display
  matrix.show();    // aktualisiere Matrix-Leds
  info.show();  // aktualisiere Status-Leds
  lastDisplay = millis();
}

// Roboter mittels PID-Regler zum Tor ausrichten
int ausrichten() {
  // Misst die Kompassabweichung vom Tor [-180 bis 179]
  heading = getCompassHeading();

  rotaryEncoder.tick(); // erkenne Reglerdrehungen

  if (m.getMotEn()) {
    pidIn = (double) heading;

    double gap = abs(pidSetpoint - pidIn); //distance away from setpoint
    myPID.SetTunings(PID_FILTER_P, PID_FILTER_I, PID_FILTER_D);
    myPID.Compute();

    return -pidOut; // [-255 bis 255]
  }
}

// Pixy auslesen: sucht groesten Block in der Farbe des Balls
void readPixy() {
  pixy.setLED(0, 0, 0); // schalte die Front-LED aus
  int ballSizeMax = 0;  // Ballgröße, 0: blind, >0: Flächeninhalt
  int goalSizeMax = 0;  // Torgröße,  0: blind, >0: Flächeninhalt

  uint16_t blocks = pixy.getBlocks();  //lässt sich die Bloecke ausgeben

  for (byte i = 0; i < blocks; i++) { // geht alle erkannten Bloecke durch
    int height = pixy.blocks[i].height;
    int width = pixy.blocks[i].width;
    int x = pixy.blocks[i].x - X_CENTER;
    switch (pixy.blocks[i].signature) { // Was sehe ich?
      case SIGNATURE_BALL:
        ballSizeMax = max(ballSizeMax, height * width);
        ball = x;           // merke Ballwinkel
        ballWidth = width;  // merke Ballbreite
        break;
      case SIGNATURE_GOAL:
        goalSizeMax = max(goalSizeMax, height * width);
        goal = x;           // merke Torwinkel
        goalWidth = width;  // merke Torbreite
        break;
    }
  }

  pixyTimer = millis();
  if (ballSizeMax > 0) {
    ballSize = ballSizeMax;
    seeBallTimer = millis();
  }
  if (goalSizeMax > 0) {
    goalSize = goalSizeMax;
    seeGoalTimer = millis();
  }

}

void sendBluetooth(byte * data, byte numberOfElements) {
  BLUETOOTH_SERIAL.write(START_MARKER);
  for (byte i = 0; i < numberOfElements; i++) {
    BLUETOOTH_SERIAL.write(constrain(data[i], 0, 253));
  }
  BLUETOOTH_SERIAL.write(END_MARKER);
}

// Bluetooth auswerten
byte receiveBluetooth() {
  // returns length of incomming message
  while (BLUETOOTH_SERIAL.available() > 0) {
    byte b = BLUETOOTH_SERIAL.read();
    if (cacheIndex != 255) { // aktives Zuhören?
      if (b == START_MARKER) {
        cacheIndex = 0;  // aktiviere Zuhören
        for (byte i = 0; i < CACHE_SIZE; i++) {
          cache[i] = 255; // überschreibe den Cache
        }
      } else if (b == END_MARKER) {
        byte messageLength = cacheIndex;
        cacheIndex = 255; // deaktiviere Zuhören
        return messageLength; // Befehl empfangen!
      } else {
        if (cacheIndex >= CACHE_SIZE) {
          cacheIndex = 255; // deaktiviere Zuhören
        } else {
          cache[cacheIndex] = b;  // speichere in Cache
          cacheIndex += 1;  // speichere index
        }
      }
    } else {
      if (b == START_MARKER) {
        cacheIndex = 0; // aktiviere Zuhören
        for (byte i = 0; i < CACHE_SIZE; i++) {
          cache[i] = 255; // überschreibe den Cache
        }
      }
    }

  }
  return 0;
}

// Status-Led zeigt bool-Wert rot oder gruen an
void showLed(Adafruit_NeoPixel & board, byte pos, bool state, bool showRed) {
  board.setPixelColor(pos, bottom.Color((!showRed) * (!state) * PWR_LED, state * PWR_LED, 0));
}

void showLed(Adafruit_NeoPixel & board, byte pos, bool state) {
  showLed(board, pos, state, false);
}

bool getUs() {
  /*  erfragt beim Ultraschallsensor durch einen Interrupt die aktuellen Sensorwerte
      empfängt und speichern diese Werte im globalen Array us[]:
          1
         .--.
        /    \ 0
      2 \    /
         '--'
           3
      gibt zurück, ob Daten empfangen wurden
  */
  digitalWrite(INT_US, 1);  // sende eine Interrupt Aufforderung an den US-Arduino
  usTimer = millis();
  while (millis() - usTimer < 3) {  // warte max. 3ms auf eine Antwort
    if (US_SERIAL.available() >= 4) { // alle Sensorwerte wurden übertragen
      for (byte i = 0; i < 4; i++) {
        us[i] = US_SERIAL.read();
      }
      digitalWrite(INT_US, 0);  // beende das Interrupt Signal
      return true;
    }
  }
  digitalWrite(INT_US, 0);  // beende das Interrupt Signal
  return false; // keine Daten konnten emopfangen werden
}

void avoidLine() {
  digitalWrite(BUZZER_AKTIV, HIGH);
  while (BOTTOM_SERIAL.available() > 1) {
    BOTTOM_SERIAL.read();
  }
  if (BOTTOM_SERIAL.available() > 0 && millis() > lineTimer + 50) {
    lineDir = BOTTOM_SERIAL.read() * 90 + 90;
    driveDir = lineDir;
    m.drive(driveDir, 255, 0);
    lineTimer = millis();
  }
}

void kick() {
  if (millis() - kickTimer > 333 && digitalRead(SWITCH_SCHUSS)) {
    digitalWrite(SCHUSS, 1);
    kickTimer = millis();
  }
}

int getCompassHeading() {
  // kompasswert [-180 bis 180]
  rotaryEncoder.tick(); // erkenne Reglerdrehungen
  accel.getEvent(&accel_event);
  rotaryEncoder.tick(); // erkenne Reglerdrehungen

  mag.getEvent(&mag_event);

  rotaryEncoder.tick(); // erkenne Reglerdrehungen

  if (dof.magGetOrientation(SENSOR_AXIS_Z, &mag_event, &orientation)) {
    rotaryEncoder.tick(); // erkenne Reglerdrehungen
    return (((int)orientation.heading - startHeading + 720) % 360) - 180;
  } else {
    stateFine = false;
    displayMessage("E: Com:read");
  }
}

String intToStr(int number) {
  if (number < 0) {
    return String(number);
  } else {
    return "+" + String(number);
  }
}

void buzzerTone(int duration) {
  digitalWrite(BUZZER_AKTIV, 1);
  buzzerStopTimer = max(buzzerStopTimer, millis() + duration);
}

