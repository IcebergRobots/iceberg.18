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
int rotMulti;
boolean start = false;
int drivePwr = 0;       // maximale Motorstärke [0 bis 255]
int driveRot = 0;       // korrigiere Kompass
int driveDir = 0;       // Zielrichtung
int lineDir = -1;       // Richtung, in der ein Bodensensor ausschlug
int lineTimer = 0;      // Zeitpunkt des Interrupts durch einen Bodensensor
boolean lastKeeperLeft = false; // deckten wir zuletzt das Tor mit einer Linksbewegung?
Pilot m;                // OBJEKTINITIALISIERUNG

// Einstellungen: KOMPASS
int heading = 0;                    // Wert des Kompass
int startHeading = 0;               // Startwert des Kompass
int rotation = 0;                   // rotationswert für die Motoren
Adafruit_9DOF                 dof   = Adafruit_9DOF();
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);

// Einstellungen: BLUETOOTH
String bluetoothBuffer = "";
String command = "";
bool  activeListening = false;
bool startLast = false;
unsigned long startTimer = 0; // Zeitpunkt des letzten Start Drückens
unsigned long bluetoothTimer = 0; // Zeitpunkt des letzten Sendens
unsigned long heartbeatTimer = 0; // Zeitpunkt des letzten empfangenen Heartbeat

// Einstellungen: WICHTUNG DER PID-REGLER
double pidSetpoint;       // Nulllevel [-180 bis 180]:Winkel des Tours
double pidIn;             // Kompasswert [-180 bis 180]
double pidOut;            // Rotationsstärke [-255 bis 255]
PID myPID(&pidIn, &pidOut, &pidSetpoint, PID_FILTER_P, PID_FILTER_I, PID_FILTER_D, DIRECT); // OBJEKTINITIALISIERUNG

// Einstellungen: PIXY
uint16_t blocks;              // hier werden die erkannten Bloecke gespeichert
int greatestBlock;

int ball;                     // Abweichung der Ball X-Koordinate
int ballWidth;                // Breite der Ballbox
boolean seeBall = false;      // sehen wir den ball?
unsigned long seeBallTimer = 0;   // Zeitpunkt des letzten Ball Sehens
unsigned long ballRightTimer = 0; // Zeitpunkt der letzten Ballsicht über 15°
unsigned long ballLeftTimer = 0;  // Zeitpunkt der letzten Ballsicht unter -15
unsigned long pixyTimer = 0;  // Zeitpunkt des letzten Auslesens der Pixy
Pixy pixy;                    // OBJEKTINITIALISIERUNG

// Einstellungen: US
byte us[] = {255, 255, 255, 255};  // Werte des US-Sensors
unsigned long usTimer = 0;        // wann wurde der Us zuletzt ausgelesen?

// Einstellungen: KICK
unsigned long kickTimer = 0;    // Zeitpunkt des letzten Schießens

// Einstellungen: DISPLAY
unsigned long lastDisplay = 0;
String displayDebug = "";     // unterste Zeile des Bildschirms;
Adafruit_SSD1306 d(PIN_4);    // OBJEKTINITIALISIERUNG

// Einstellungen: STATUS-LEDS & LED-MATRIX
boolean stateFine = true;     // liegt kein Fehler vor?
boolean hasBall = false;  // besitzen der Roboter den Ball?
boolean showBottom = true;    // sollen die Boden-Leds an sein?

// Einstellungen: BUZZER
unsigned long buzzerStopTimer = 0; // Zeitpunkt, wann der Buzzer ausgehen soll

// Einstellungen: ROTARY-ENCODER
RotaryEncoder rotaryEncoder(ROTARY_B, ROTARY_A);
int rotaryPosition = 0;

// DEBUG
boolean isTypeA;
String messageLog = "";
unsigned long moveTimer = 0;

Adafruit_NeoPixel bottom = Adafruit_NeoPixel(16, BODEN_LED, NEO_GRB + NEO_KHZ800); // OBJEKTINITIALISIERUNG (BODEN-LEDS)
Adafruit_NeoPixel matrix = Adafruit_NeoPixel(12, MATRIX_LED, NEO_GRB + NEO_KHZ800); // OBJEKTINITIALISIERUNG (LED-MATRIX)
Adafruit_NeoPixel info = Adafruit_NeoPixel(3, INFO_LED, NEO_GRB + NEO_KHZ800); // OBJEKTINITIALISIERUNG (STATUS-LEDS)

sensors_event_t accel_event;
sensors_event_t mag_event;
sensors_vec_t   orientation;

//###################################################################################################

void setup() {
  DEBUG_SERIAL.begin(115200);   // Start der Seriellen Kommunikation
  BLUETOOTH_SERIAL.begin(115200); // 38400
  US_SERIAL.begin(38400); // 115200
  BOTTOM_SERIAL.begin(115200);
  Wire.begin();         // Start der I2C-Kommunikation

  //  attachInterrupt(digitalPinToInterrupt(INT_BODENSENSOR), avoidLine, RISING);     //erstellt den Interrupt -> wenn das Signal am Interruptpin ansteigt, dann wird die Methode usAusgeben ausgeführt

  setupDisplay();       // initialisiere Display mit Iceberg Schriftzug
  displayMessage("1/9 Pins");
  pinModes();           // setzt die PinModes
  isTypeA = digitalRead(TYPE);
  displayMessage("2/9 Motoren");
  setupMotor();         // setzt Pins und Winkel des Pilot Objekts
  displayMessage("3/9 Pixy");
  pixy.init();          // initialisiere Kamera
  pixy.setLED(0, 0, 0);
  displayMessage("4/9 EEPROM");
  startHeading = -EEPROM.read(0) * EEPROM.read(1);

  displayMessage("5/9 Accel");
  if (!accel.begin()) {
    stateFine = false;
    displayMessage("5/9 Accel failed");
  }

  displayMessage("6/9 Mag");
  if (!mag.begin()) {
    stateFine = false;
    displayMessage("6/9 Mag failed");
  }

  delay(1);

  displayMessage("7/9 Kompass");
  mag.enableAutoRange(true);
  heading = getCompassHeading();

  displayMessage("8/9 Tor");
  // merke Torrichtung
  m.brake(true);        // Roboter bremst aktiv
  pidSetpoint = 0;
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(-255, 255);

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
  debugln("loop");
  rotaryEncoder.tick(); // update rotary encoder

  // remote start when keeper aktiviert
  if (!digitalRead(SWITCH_MOTOR)) {
    m.setMotEn(!digitalRead(SWITCH_KEEPER) || start);
  } else {
    m.setMotEn(false);
    start = false;
  }

  // schuß wieder aus machen
  if (millis() - kickTimer > 30) {
    digitalWrite(SCHUSS, 0);
  }

  // buzzer anschalten bzw. wieder ausschalten
  digitalWrite(BUZZER_AKTIV, millis() <= buzzerStopTimer);

  // read rotary encoder
  rotaryEncoder.tick(); // update rotary encoder
  if (!digitalRead(ROTARY_BUTTON)) {
    rotaryEncoder.setPosition(0);
    buzzerTone(50);
  }
  rotaryPosition = (ROTARY_RANGE + (rotaryEncoder.getPosition() % ROTARY_RANGE)) % ROTARY_RANGE;
  displayDebug = rotaryPosition;

  // kompass kalibrieren
  if (!digitalRead(BUTTON_2)) {
    //Torrichtung [-180 bis 179] merken
    startHeading = 0;
    startHeading = getCompassHeading(); //merkt sich Startwert des Kompass
    EEPROM.write(0, startHeading < 0); // Vorzeichen
    EEPROM.write(1, abs(startHeading)); // Winkel
    heading = 0;
    buzzerTone(200);
  }

  rotaryEncoder.tick(); // update rotary encoder

  hasBall = analogRead(LIGHT_BARRIER) > LIGHT_BARRIER_TRIGGER_LEVEL;
  seeBall = millis() - seeBallTimer < 50;

  showLed(info, 0, stateFine);
  showLed(info, 1, !battLow());
  showLed(info, 2, millis() % 1000 < 200, true);

  showLed(matrix, 1, m.getMotEn());
  showLed(matrix, 2, seeBall);
  showLed(matrix, 3, hasBall);
  showLed(matrix, 4, millis() - heartbeatTimer < 500);
  showLed(matrix, 11, ballWidth > 15);

  // schieße
  if (hasBall || !digitalRead(SCHUSS_BUTTON)) {
    kick();
  }

  // prüfe, ob Boden-Leds an sein sollen
  for (int i = 0; i < 16; i++) {
    if (!digitalRead(SWITCH_BODENS)) {
      bottom.setPixelColor(i, 0, 0, 0);
    } else if (!digitalRead(SWITCH_A)) {
      bottom.setPixelColor(i, 255, 0, 0);
    } else {
      bottom.setPixelColor(i, 255, 255, 255);
    }
  }
  bottom.show();

  rotaryEncoder.tick(); // update rotary encoder

  // aktualisiere Pixywerte (max. alle 30ms)
  if (millis() - pixyTimer > 50) {
    readPixy();
  }

  rotaryEncoder.tick(); // update rotary encoder

  // lese die US maximal alle 30ms aus
  if (millis() - usTimer > 100) {
    getUs();
    usTimer = millis();
  }

  rotaryEncoder.tick(); // update rotary encoder

  // remote start
  if (!digitalRead(BIG_BUTTON)) {
    if (!startLast || millis() - startTimer < 100) {
      bluetooth('s');
      start = true;
    } else if (millis() - startTimer > 1000) {
      bluetooth('b');
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
    char data[7];
    data[0] = 'h';
    if (!start) {
      data[1] = 255;  // pause: 255
    } else if (!seeBall) {
      data[1] = 2;    // ball blind: 2
    } else {
      data[1] = ball < 0;
      // ball < 0: 0
      // ball >= 0: 1
    }
    data[2] = constrain(abs(ball), 0, 255); // 0: not negativ, 1: negativ
    data[3] = ballWidth % 256;
    data[4] = ballWidth / 256;
    data[3] = us[0];
    data[4] = us[1];
    data[5] = us[2];
    data[6] = us[3];
    bluetooth(data); // heartbeat
  }

  // bluetooth auslesen
  command = receiveBluetooth();
  if (command != "") {
    switch (command.charAt(0)) {
      case 'h': // heartbeat
        heartbeatTimer = millis();
        break;
      case 's': // start
        start = true;
        break;
      case 'b': // brake
        m.brake(true);
        start = false;
        break;
      case 'd': // brake
        for (int i = 0; i < 4; i++) {
          us[i] = (unsigned char) command.charAt(i + 1);
        }
        break;
    }
  }

  rotaryEncoder.tick(); // update rotary encoder

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

  rotaryEncoder.tick(); // update rotary encoder
  //displayDebug = String(rotMulti) + "," + String(ballWidth);
  driveRot = ausrichten();

  rotaryEncoder.tick(); // update rotary encoder

  if (m.getMotEn() || true) {
    if (lineDir >= 0 && millis() - lineTimer < 20) { // anfangs ist lineDir negativ, beim einem Interrupt immer positiv
      drivePwr = 255;
    } else {
      //drivePwr = map(analogRead(POTI), 0, 1023, 0, 255) - abs(heading);
      if (seeBall) {
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
      } else {
        // fahre nach hinten
        driveDir = 180;
        drivePwr = SPEED_BACKWARDS;

        if (us[3] < 50 && us[3] > 0 && abs(heading) < 40) {
          drivePwr = SPEED_KEEPER;
          if (lastKeeperLeft) {
            if (us[2] > COURT_GOAL_TO_BORDER) {
              driveDir = ANGLE_SIDEWAY;
            } else {
              driveDir = -ANGLE_SIDEWAY;
              lastKeeperLeft = false;
            }
          } else {
            if (us[0] > COURT_GOAL_TO_BORDER) {
              driveDir = -ANGLE_SIDEWAY;
            } else {
              driveDir = ANGLE_SIDEWAY;
              lastKeeperLeft = true;
            }
          }
        }

      }
      if (ballWidth > 32) {
        drivePwr /= 2;
      }
    }
    drivePwr = max(drivePwr - abs(driveRot), 0);

    rotaryEncoder.tick(); // update rotary encoder

    if (millis() - lineTimer > 50) {
      m.drive(driveDir, drivePwr, driveRot);
    }
  }

  rotaryEncoder.tick(); // update rotary encoder

  //displayDebug = driveRot;
  // aktualisiere Bildschirm und LEDs
  if (millis() - lastDisplay > 40) {
    updateDisplay();
  }

  rotaryEncoder.tick(); // update rotary encoder

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
      d.print(us[1] + String("    ").substring(0, 4 - String(us[1]).length() ));
      d.print(String("   ").substring(0, 3 - String(us[0]).length()) + String(us[0]) );
      d.setCursor(21, 46);
      d.print(us[2] + String("    ").substring(0, 4 - String(us[2]).length() ));
      d.print(String("   ").substring(0, 3 - String(us[3]).length()) + String(us[3]));
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
      name2 = "ball:";
      value2 = intToStr(ball);   // ball angle
      break;
    case 4:
      name1 = "t:";
      value1 = String(millis()); // ratation multiplier
      name2 = "headi:";
      value2 = intToStr(heading);  // heading
      break;
    case 5:
      name1 = "bluet-c:";
      value1 = String(command);  // bluetooth command
      name2 = "start:";
      value2 = String(start);      // start
      break;
    case 6:
      name1 = "bWid:";
      value1 = String(ballWidth);  // ball box width
      name2 = "bSiz:";
      value2 = String(greatestBlock);  // ball box height*width
      break;
  }
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

  rotaryEncoder.tick(); // update rotary encoder

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
  pixy.setLED(0, 0, 0);
  greatestBlock = 0; //hier wird die Groeße des groeßten Blocks gespeichert
  int highX = 0;             //Position des Balls (X)
  int highY = 0;             //Position des Balls (Y)
  int blockAnzahl = 0;       //Anzahl der Bloecke
  ballWidth = 0;

  blocks = pixy.getBlocks();  //lässt sich die Bloecke ausgeben

  for (int j = 0; j < blocks; j++) {                                  //geht alle erkannten Bloecke durch
    if (pixy.blocks[j].signature == PIXY_BALL_NUMMER) {               //Überprueft, ob es sich bei dem Block um den Ball handelt
      if (pixy.blocks[j].height * pixy.blocks[j].width > greatestBlock) { //Wenn der Block der aktuell groesste ist
        greatestBlock = pixy.blocks[j].height * pixy.blocks[j].width;
        highX = pixy.blocks[j].x;
        highY = pixy.blocks[j].y;
        ball = highX - X_CENTER;    // neue Ballposition wird gesetzt
        ballWidth = pixy.blocks[j].width;
      }
      blockAnzahl++;        //Anzahl wird hochgezaehlt
    }
  }

  pixyTimer = millis();         //Timer wird gesetzt, da Pixy nur alle 25ms ausgelesen werden darf

  if (blockAnzahl > 0) { //wenn Bloecke in der Farbe des Balls erkannt wurden, dann sehen wir den Ball
    seeBallTimer = millis();
  }
}

// Bluetooth auswerten
String receiveBluetooth() {
  if (BLUETOOTH_SERIAL.available() > 0) {
    char c = BLUETOOTH_SERIAL.read();
    if ( activeListening) {
      if (c == START_MARKER) {
        bluetoothBuffer = "";
      } else if (c == END_MARKER) {
        activeListening = false;
        return (bluetoothBuffer);
      } else {
        bluetoothBuffer += c;
      }
    } else {
      if (c == START_MARKER) {
        bluetoothBuffer = "";
        activeListening = true;
      }
    }

  }
  return ("");
}

// Status-Led zeigt Boolean-Wert rot oder gruen an
void showLed(Adafruit_NeoPixel &board, byte pos, boolean state, boolean showRed) {
  board.setPixelColor(pos, bottom.Color((!showRed) * (!state) * PWR_LED, state * PWR_LED, 0));
}

void showLed(Adafruit_NeoPixel &board, byte pos, boolean state) {
  showLed(board, pos, state, false);
}

boolean getUs() {
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
  /*digitalWrite(INT_US, 1);  // sende eine Interrupt Aufforderung an den US-Arduino
    usTimer = millis();
    while (millis() - usTimer < 3) {  // warte max. 3ms auf eine Antwort
    if (US_SERIAL.available() >= 4) { // alle Sensorwerte wurden übertragen
      for (int i = 0; i < 4; i++) {
        us[i] = US_SERIAL.read();
      }
      digitalWrite(INT_US, 0);  // beende das Interrupt Signal
      return true;
    }
    }
    digitalWrite(INT_US, 0);  // beende das Interrupt Signal
    return false; // keine Daten konnten emopfangen werden*/
}

void avoidLine() {
  buzzerTone(50);
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
  rotaryEncoder.tick(); // update rotary encoder
  accel.getEvent(&accel_event);
  rotaryEncoder.tick(); // update rotary encoder

  mag.getEvent(&mag_event);

  rotaryEncoder.tick(); // update rotary encoder

  if (dof.magGetOrientation(SENSOR_AXIS_Z, &mag_event, &orientation)) {
    rotaryEncoder.tick(); // update rotary encoder
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

