#include "Utility.h"

// Implementierung: OBJEKTE
extern Display d;
extern Pilot m;
extern Mate mate;
extern Led led;

int shift(int &value, int min, int max) {
  max -= min;
  value = (max + (value - min % max)) % max + min; // wandle Drehposition in Zustand von 0 bis ROTARY_RANGE um
  return value;
}

void startSound() {
  //Fiepen, welches Programstart signalisiert
  for (int i = 10; i < 2000; i += 10) {
    tone(BUZZER, i);
    delay(1);
  }
  noTone(BUZZER);
}

/*****************************************************
  Berechne alle Statuswerte und Zustände
*****************************************************/
void calculateStates() {
  isMotor = m.getMotEn();
  onLine = millis() <= lineTimer;
  isHeadstart = millis() - headstartTimer < HEADSTART_DURATION;
  isConnected = millis() - heartbeatTimer < 500;
  batVol = analogRead(BATT_VOLTAGE) * 0.1220703;  // SPANNUNG MAL 10!
  if (batVol > VOLTAGE_MIN) {
    batState = 1; // ok
    if (isMotor) {
      if (batVol < VOLTAGE_MOTOR_CRIT) {
        batState = 3; // kritisch
      } else if (batVol < VOLTAGE_MOTOR_LOW) {
        batState = 2; // gering
      }
    } else {
      if (batVol < VOLTAGE_CRIT) {
        batState = 3; // kritisch
      } else if (batVol < VOLTAGE_LOW) {
        batState = 2; // gering
      }
    }
  } else {
    batState = 0; // no battery
  }
  seeBall = millis() - seeBallTimer < 50;
  seeGoal = millis() - seeGoalTimer < 1200;
  isDrift = millis() - driftTimer < 100;
  if (pixyResponseTimer > 0 && millis() - pixyResponseTimer < PIXY_RESPONSE_DURATION) {
    // Kamera war in den letzen 30 Sekunden bereits aktiv
    pixyState = 1;
  } else if (pixyResponseTimer > 0) {
    // Kamera war seit dem letzten Neustart bereits aktiv
    pixyState = 2;
  } else {
    // Kamera nicht angeschlossen
    pixyState = 3;
  }
  usConnected = millis() - usResponseTimer < 500;
  usFine = usConnected && us[0] * us[2] * us[3] != 0;
  hasBall = analogRead(LIGHT_BARRIER) > LIGHT_BARRIER_TRIGGER_LEVEL;
}

/*****************************************************
  Sende einen Herzschlag mit Statusinformationen an den Partner
  Byte    Information   mögliche Zustände
  ---------------------------------------
  0       Pakettyps     Heartbeat(104)
  1       Rolle         Stürmer(2) / Torwart(1) / Aus(0)
  2       Ballzustand   blind(2) / links(1) / rechts(0)
  3       Ballwinkel    (0 bis 253)
  4,5     Ballbreite    je (0 bis 253)
  6,7,8,9 Ultraschall   je (0 bis 253)
*****************************************************/
void transmitHeartbeat() {
  byte data[10];
  data[0] = 'h';
  if(!m.getMotEn()||isLifted) data[1] = 0;
  else data[1] = !isRusher;
  if (!seeBall) data[2] = 2;
  else data[1] = ball < 0;
  data[2] = abs(ball);
  data[3] = ballWidth % 254;
  data[4] = ballWidth / 254;
  data[5] = us[0];
  data[6] = us[1];
  data[7] = us[2];
  data[8] = us[3];
  mate.send(data, 10); // heartbeat
  bluetoothTimer = millis();
}

/*****************************************************
  Piloten konfigurieren
*****************************************************/
void setupMotor() {
  m.setAngle(70);

  m.setPins(0, FWD0, BWD0, PWM0);
  m.setPins(1, FWD1, BWD1, PWM1);
  m.setPins(2, FWD2, BWD2, PWM2);
  m.setPins(3, FWD3, BWD3, PWM3);
}

void avoidLine() {
  buzzerTone(100);
  while (BOTTOM_SERIAL.available() > 1) {
    BOTTOM_SERIAL.read();
  }
  if (BOTTOM_SERIAL.available() > 0) {
    lineDir = BOTTOM_SERIAL.read() * 90 + 90;
    driveDirection = lineDir;
    m.drive(driveDirection, SPEED_LINE, 0);
    lineTimer = millis();
    headstartTimer = 0;
    if (drivePower > 200) {
      lineTimer = millis() + (2 * LINE_DURATION);
    } else if (drivePower > 100) {
      lineTimer = millis() + (1.5 * LINE_DURATION);
    } else {
      lineTimer = millis() + LINE_DURATION;
    }
    displayDebug = driveDirection;
  }


}

void kick() {
  if (millis() - kickTimer > 333 && digitalRead(SWITCH_SCHUSS)) {
    digitalWrite(SCHUSS, 1);
    kickTimer = millis();
  }
}

void readCompass() {
  // kompasswert [-180 bis 180]
  accel.getEvent(&accel_event);
  mag.getEvent(&mag_event);

  if (dof.magGetOrientation(SENSOR_AXIS_Z, &mag_event, &orientation)) {
    heading = (((int)orientation.heading - startHeading + 720) % 360) - 180;
  } else {
    stateFine = false;
  }
}

void buzzerTone(int duration) {
  if (!SILENT) {
    analogWrite(BUZZER, 127);
    buzzerStopTimer = max(buzzerStopTimer, millis() + duration);
  }
}


// Roboter mittels PID-Regler zum Tor ausrichten
int ausrichten(int orientation) {
  pidSetpoint = shift(orientation, -179, 180);
  if (isLifted || onLine) {
    return 0;
  } else {
    // Misst die Kompassabweichung vom Tor [-180 bis 179]
    if (m.getMotEn()) {
      pidIn = (double) heading;

      double gap = abs(pidSetpoint - pidIn); //distance away from setpoint
      myPID.Compute();

      return -pidOut; // [-255 bis 255]
    }
  }
}

/*****************************************************
  Pixy auslesen: sucht groesten Block in der Farbe des Balls

  SPI-Protokoll:

  INPUT getBlocks():
  Bytes    16-bit words   Description
  ----------------------------------------------------------------
  0, 1     0              sync (0xaa55)
  2, 3     1              checksum (sum of all 16-bit words 2-6)
  4, 5     2              signature number
  6, 7     3              x center of object
  8, 9     4              y center of object
  10, 11   5              width of object
  12, 13   6              height of object

  OUTPUT setServos(servo 0, servo 1):
  Bytes    16-bit words   Description
  ----------------------------------------------------------------
  0, 1     0             sync (0xff00)
  2, 3     1             servo 0 (pan) position, between 0 and 1000
  4, 5     2             servo 1 (tilt) position, between 0 and 1000

  OUTPUT setBrightness(brightness)
  Bytes    16-bit words   Description
  ----------------------------------------------------------------
  0, 1     0             sync (0xfe00)
  2        1             brightness

  OUTPUT setLed(red, green, blue):
  Bytes    16-bit words   Description
  ----------------------------------------------------------------
  0, 1     0              sync (0xfd00)
  2, 3     1              red, green
  4        2              blue
*****************************************************/
void readPixy() {
  pixy.setLED(0, 0, 0); // schalte die Front-LED aus
  int ballSizeMax = 0;  // Ballgröße, 0: blind, >0: Flächeninhalt
  int goalSizeMax = 0;  // Torgröße,  0: blind, >0: Flächeninhalt

  uint16_t blockCount = pixy.getBlocks();
  // Liest alle Blöcke aus und zählt diese
  // Sendet "cs error" über USB bei Fehler in Prüfsumme eines empfangenen Objekts

  for (byte i = 0; i < blockCount; i++) { // geht alle erkannten Bloecke durch
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
    pixyResponseTimer = millis();
  }

  if (ballSizeMax > 0) {
    ballSize = ballSizeMax;
    seeBallTimer = millis();
  }
  if (goalSizeMax > 0) {
    goalSize = goalSizeMax;
    seeGoalTimer = millis();
  }

  pixyTimer = millis(); // merke Zeitpunkt
}
