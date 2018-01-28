#include "Config.h"
#include "Pin.h"
#include "Pilot.h"
#include "Utility.h"

#include <SPI.h>
#include <Pixy.h>
#include <Wire.h>
#include <HMC6352.h>
#include <PID_v1.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

// Einstellungen: FAHREN
int drivePwr = 0;       // maximale Motorstärke [0 bis 255]
int driveRot = 0;       // korrigiere Kompass
int driveDir = 0;       // Zielrichtung
int lineDir = -1;        // Richtung, in der ein Bodensensor ausschlug
int lineTimer = 0;      // Zeitpunkt des Interrupts durch einen Bodensensor
Pilot m;                // OBJEKTINITIALISIERUNG

// Einstellungen: KOMPASS
int heading = 0;                // Wert des Kompass
int startHeading;               // Startwert des Kompass
int rotation;                   // rotationswert für die Motoren
HMC6352 c;                      // OBJEKTINITIALISIERUNG

// Einstellungen: BLUETOOTH
String bluetoothBuffer = "";
String command = "";
bool  activeListening = false;
unsigned long bluetoothTimer = 0; // Zeitpunkt des letzten Sendens
unsigned long heartbeatTimer = 0; // Zeitpunkt des letzten empfangenen Heartbeat

// Einstellungen: WICHTUNG DER PID-REGLER
double pidSetpoint;       // Nulllevel [-180 bis 180]:Winkel des Tours
double pidIn;             // Kompasswert [-180 bis 180]
double pidOut;            // Rotationsstärke [-255 bis 255]
PID myPID(&pidIn, &pidOut, &pidSetpoint, PID_FILTER_P, PID_FILTER_I, PID_FILTER_D, DIRECT); // OBJEKTINITIALISIERUNG

// Einstellungen: PIXY
uint16_t blocks;              // hier werden die erkannten Bloecke gespeichert
int ball;                     // Abweichung der Ball X-Koordinate
boolean seeBall;            // ob wir den Ball sehen
unsigned long pixyTimer = 0;   // Zeitpunkt des letzten Auslesens der Pixy
Pixy pixy;                    // OBJEKTINITIALISIERUNG

// Einstellungen: US
int us[] = {255, 255, 255, 255};  // Werte des US-Sensors
unsigned long usTimer = 0;        // wann wurde der Us zuletzt ausgelesen?

// Einstellungen: DISPLAY
unsigned long lastDisplay = 0;
String displayDebug = "";     // unterste Zeile des Bildschirms;
Adafruit_SSD1306 d(PIN_4);    // OBJEKTINITIALISIERUNG

// Einstellungen: STATUS-LEDS & LED-MATRIX
boolean stateFine = true;     // liegt kein Fehler vor?
boolean hasBall = false;  // besitzen der Roboter den Ball?
boolean showBottom = true;    // sollen die Boden-Leds an sein?
Adafruit_NeoPixel bottom = Adafruit_NeoPixel(16, BODEN_LED, NEO_GRB + NEO_KHZ800); // OBJEKTINITIALISIERUNG (BODEN-LEDS)
Adafruit_NeoPixel matrix = Adafruit_NeoPixel(12, MATRIX_LED, NEO_GRB + NEO_KHZ800); // OBJEKTINITIALISIERUNG (LED-MATRIX)
Adafruit_NeoPixel info = Adafruit_NeoPixel(3, INFO_LED, NEO_GRB + NEO_KHZ800); // OBJEKTINITIALISIERUNG (STATUS-LEDS)

//###################################################################################################

void setup() {
  DEBUG_SERIAL.begin(9600);   // Start der Seriellen Kommunikation
  BLUETOOTH_SERIAL.begin(38400);
  US_SERIAL.begin(115200);
  BOTTOM_SERIAL.begin(38400);
  Wire.begin();         // Start der I2C-Kommunikation

  attachInterrupt(digitalPinToInterrupt(INT_BODENSENSOR), avoidLine, RISING);     //erstellt den Interrupt -> wenn das Signal am Interruptpin ansteigt, dann wird die Methode usAusgeben ausgeführt

  setupDisplay();       // initialisiere Display mit Iceberg Schriftzug
  pinModes();           // setzt die PinModes
  setupMotor();         // setzt Pins und Winkel des Pilot Objekts
  pixy.init();          // initialisiere Kamera

  delay(1000);

  //Torrichtung [-180 bis 179] merken
  startHeading = c.getHeading() - 180; //merkt sich Startwert des Kompass

  m.setMotEn(true);     // aktiviere die Motoren

  // Kalibriere Kompass: Drehe und messe kontinuierlich Kompasswerte
  if (!digitalRead(SWITCH_A)) {
    m.drive(0, 0, 8);   // Roboter drehr sich um eigene Achse
    c.calibration();
  }

  // merke Torrichtung
  m.brake(true);        // Roboter bremst aktiv
  c.setOutputMode(0);   // Kompass initialisieren
  pidSetpoint = 0;
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(-255, 255);

  bottom.begin();   // BODEN-LEDS initialisieren
  matrix.begin();   // MATRIX-LEDS initialisieren
  info.begin(); // STATUS-LEDS initialisieren

  debugln("setup done");
}

//###################################################################################################

void loop() {
  m.setMotEn(!digitalRead(SWITCH_MOTOR));

  hasBall = analogRead(LIGHT_BARRIER) > LIGHT_BARRIER_TRIGGER_LEVEL;

  showLed(info, 0, stateFine);
  showLed(info, 1, !battLow());
  showLed(info, 2, millis() % 1000 < 200, false, true);

  showLed(matrix, 1, m.getMotEn());
  showLed(matrix, 2, seeBall);
  showLed(matrix, 3, hasBall);
  showLed(matrix, 4, millis() - heartbeatTimer < 500);

  showBottom = !digitalRead(SWITCH_B);
  for (int i = 0; i < 16; i++) {
    showLed(bottom, i, showBottom, true, false);
  }
  bottom.show();

  // lese die Pixy maximal alle 30ms aus
  if (millis() - pixyTimer > 30) {
    readPixy();
  }

  // lese die US maximal alle 30ms aus
  if (!digitalRead(BUTTON_1) && millis() - usTimer > 500) {
    if (getUs()) {
      displayDebug = String(us[0]) + "," + String(us[1]) + "," + String(us[2]) + "," + String(us[3]);
    }
    usTimer = millis();
  }

  // bluetooth senden
  if (millis() - bluetoothTimer > 100) {
    bluetoothTimer = millis();
    bluetooth("h"); // heartbeat
  }

  // bluetooth auslesen
  command = receiveBluetooth();
  if (command != "") {
    switch (command.charAt(0)) {
      case 'h': // heartbeat
        heartbeatTimer = millis();
        break;
    }
  }

  // Fahre
  driveRot = ausrichten();
  if (lineDir>=0&&millis() - lineTimer < 20) {  // anfangs ist lineDir negativ, beim einem Interrupt immer positiv
    drivePwr = 255;
  } else {
    drivePwr = map(analogRead(POTI), 0, 1023, 0, 255) - abs(heading);
    if (seeBall) {
      if (-20 < ball && ball < 20) {
        // fahre geradeaus
        driveDir = 0;
      } else {
        // drehe dich zum Ball
        driveDir = map(ball, -100, 100, ROT_MULTI, -ROT_MULTI);
      }
    } else {
      // fahre nach hinten
      driveDir = 180;
    }
  }

  m.drive(driveDir, drivePwr, driveRot);

  // aktualisiere Bildschirm und LEDs
  if (millis() - lastDisplay > 40) {
    updateDisplay();
  }

  delay(1);

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
  d.setTextSize(1);     //setzt Textgroesse
  d.println();
  d.setTextSize(2);     //setzt Textgroesse
  d.println("      2018");
  d.display();          //wendet Aenderungen an
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
  d.println("Iceberg Robots " + myTime);
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
    d.println("Ball:");
    d.drawLine(91, 27, constrain(map(ball, -160, 160, 60, 123), 60, 123), 14, WHITE);
  } else {
    d.println("Ball:blind");
  }
  d.drawLine(3, 11, map(drivePwr, 0, 255, 3, 123), 11, WHITE);
  d.setCursor(3, 30);
  d.println("Dir: " + String(driveDir));
  d.setCursor(3, 46);
  d.println(String(displayDebug));

  d.display();      // aktualisiere Display
  matrix.show();    // aktualisiere Matrix-Leds
  info.show();  // aktualisiere Status-Leds
  lastDisplay = millis();
}

// Roboter mittels PID-Regler zum Tor ausrichten
int ausrichten() {
  // Misst die Kompassabweichung vom Tor [-180 bis 179]
  heading = ((int)((c.getHeading()/*[0 bis 359]*/ - startHeading/*[-359 bis 359]*/) + 360) % 360)/*[0 bis 359]*/ - 180;

  pidIn = (double) heading;

  double gap = abs(pidSetpoint - pidIn); //distance away from setpoint
  myPID.SetTunings(PID_FILTER_P, PID_FILTER_I, PID_FILTER_D);
  myPID.Compute();

  return -pidOut; // [-255 bis 255]
}

// Pixy auslesen: sucht groesten Block in der Farbe des Balls
void readPixy() {
  int greatestBlock = 0; //hier wird die Groeße des groeßten Blocks gespeichert
  int highX = 0;             //Position des Balls (X)
  int highY = 0;             //Position des Balls (Y)
  int blockAnzahl = 0;       //Anzahl der Bloecke

  blocks = pixy.getBlocks();  //lässt sich die Bloecke ausgeben

  for (int j = 0; j < blocks; j++) {                                  //geht alle erkannten Bloecke durch
    if (pixy.blocks[j].signature == PIXY_BALL_NUMMER) {               //Überprueft, ob es sich bei dem Block um den Ball handelt
      if (pixy.blocks[j].height * pixy.blocks[j].width > greatestBlock) { //Wenn der Block der aktuell groesste ist
        greatestBlock = pixy.blocks[j].height * pixy.blocks[j].width;
        highX = pixy.blocks[j].x;
        highY = pixy.blocks[j].y;
        ball = highX - X_CENTER;    // neue Ballposition wird gesetzt
      }
      blockAnzahl++;        //Anzahl wird hochgezaehlt
    }
  }

  pixyTimer = millis();         //Timer wird gesetzt, da Pixy nur alle 25ms ausgelesen werden darf

  seeBall = (blockAnzahl > 0); //wenn Bloecke in der Farbe des Balls erkannt wurden, dann sehen wir den Ball
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
void showLed(Adafruit_NeoPixel &board, byte pos, boolean state, boolean showRed, boolean showGreen) {
  board.setPixelColor(pos, bottom.Color(showRed * (!state) * PWR_LED, showGreen * state * PWR_LED, 0));
}

void showLed(Adafruit_NeoPixel &board, byte pos, boolean state) {
  showLed(board, pos, state, true, true);
}

boolean getUs() {
  digitalWrite(INT_US, 1);
  usTimer = millis();
  while (millis() - usTimer < 3) {
    if (US_SERIAL.available() >= 4) {
      debugln("available");
      for (int i = 0; i < 4; i++) {
        us[i] = US_SERIAL.read();
      }
      digitalWrite(INT_US, 0);
      return true;
    }
  }
  digitalWrite(INT_US, 0);
  return false;
}

void avoidLine() {
  if (BOTTOM_SERIAL.available() > 0) {
    lineDir = BOTTOM_SERIAL.read() + 180;
    driveDir = lineDir;
    m.drive(driveDir, 255, 0);
    lineTimer = millis();
  }
}

