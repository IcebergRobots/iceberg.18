#include "Utility.h"

// Implementierung: OBJEKTE
extern Display d;
extern Player p;
extern Led led;
extern Mate mate;
extern Pilot m;
extern Ultrasonic us;

void reset() {
  asm ("jmp 0");   // starte den Arduino neu
}

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
  isLifted = millis() - flatTimer > 300;
  onLine = millis() <= lineTimer;
  isHeadstart = millis() - headstartTimer < HEADSTART_DURATION;
  batVol = analogRead(BATT_VOLTAGE) * 0.1220703;  // SPANNUNG MAL 10!
  if (batVol > VOLTAGE_MIN) {
    batState = 1; // ok
    if (m.getMotEn()) {
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
  silent = !digitalRead(SWITCH_DEBUG);

  seeBall = !isLifted && millis() - seeBallTimer < 100;
  seeGoal = !isLifted && millis() - seeGoalTimer < 1200;
  seeEast = !isLifted && millis() - seeEastTimer < 500;
  seeWest = !isLifted && millis() - seeWestTimer < 500;
  closeBall = millis() - closeBallTimer < 500;
  isDrift = millis() - driftTimer < 100;
  isHeadstart = millis() - headstartTimer < HEADSTART_DURATION;
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
  hasBall = analogRead(LIGHT_BARRIER) > lightBarrierTriggerLevel;

  if (us.right() + us.left() >= COURT_WIDTH_FREE) lastGoalFree = millis();

  // erkenne Hochheben
  if (accel_event.acceleration.z >= 8) flatTimer = millis();
}

/*****************************************************
  Sende einen Herzschlag mit Statusinformationen an den Partner

  Byte    Information   mögliche Zustände
  -----------------------------------------------------
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
  if(!m.getMotEn()) data[1] = p.getState();
  else if(p.isKeeper()) data[1] = p.getState() + 10;
  else if(p.isRusher()) data[1] = p.getState() + 20;
  if (!m.getMotEn() || !seeBall) data[2] = 2;
  else data[2] = ball < 0;
  data[3] = abs(ball);
  data[4] = ballWidth % 254;
  data[5] = ballWidth / 254;
  data[6] = us.right();
  data[7] = us.front();
  data[8] = us.left();
  data[9] = us.back();
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
    lineDir = (BOTTOM_SERIAL.read() * 22.5) - 180;
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
  if (millis() - kickTimer > 333 && (digitalRead(SWITCH_SCHUSS) || !digitalRead(SCHUSS_BUTTON))) {
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
  if (!silent) {
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
  if (silent) pixy.setLED(0, 0, 0); // schalte die Front-LED aus
  int ballAreaMax = 0;  // Ballgröße, 0: blind, >0: Flächeninhalt
  int goalAreaMax = 0;  // Torgröße,  0: blind, >0: Flächeninhalt
  int eastHeightMax = 0;  // Farbmarkierungsgröße,  0: blind, >0: Flächeninhalt
  int westHeightMax = 0;  // Farbmarkierungsgröße,  0: blind, >0: Flächeninhalt

  blockCount = pixy.getBlocks();
  blockCountBall = 0;
  blockCountGoal = 0;
  blockCountEast = 0;
  blockCountWest = 0;
  // Liest alle Blöcke aus und zählt diese
  // Sendet "cs error" über USB bei Fehler in Prüfsumme eines empfangenen Objekts

  for (byte i = 0; i < blockCount; i++) { // geht alle erkannten Bloecke durch
    int height = pixy.blocks[i].height;
    int width = pixy.blocks[i].width;
    int x = pixy.blocks[i].x - X_CENTER;
    int signature = pixy.blocks[i].signature;
    int angle = pixy.blocks[i].angle;
    int area = height * width;
    switch (signature) { // Was sehe ich?
      case SIGNATURE_BALL:
        blockCountBall++;
        if (area > ballAreaMax) {
          ballAreaMax = area;
          ball = x;         // merke Ballwinkel
          ballWidth = width;  // merke Ballbreite
          seeBallTimer = millis();
          if (ballWidth > BALL_WIDTH_TRIGGER) closeBallTimer = millis();
        }
        break;
      case SIGNATURE_GOAL:
        blockCountGoal++;
        if (area > ballAreaMax) {
          ballAreaMax = area;
          goal = x;          // merke Torwinkel
          goalWidth = width;  // merke Torbreite
          seeGoalTimer = millis();
        }
        break;
      case SIGNATURE_CC:
        if (angle < 0) {
          blockCountEast++;
          if (height > eastHeightMax) {
            eastHeightMax = height;
            east = x;
            eastHeight = height;
            seeEastTimer = millis();
          }
        } else {
          blockCountWest++;
          if (height > westHeightMax) {
            westHeightMax = height;
            west = x;
            westHeight = height;
            seeWestTimer = millis();
          }
        }
        break;
    }
    pixyResponseTimer = millis();
  }

  pixyTimer = millis(); // merke Zeitpunkt
}

/*****************************************************
  sende Text zum PC
*****************************************************/
void debug(String str) {
  if (DEBUG && !silent) DEBUG_SERIAL.print(str + " ");
}
void debug(long num) {
  debug(String(num));
}

/*****************************************************
  sende Text zum PC
*****************************************************/
void debugln(String str) {
  if (DEBUG && !silent) DEBUG_SERIAL.println(str);
}
void debugln(long num) {
  debugln(String(num));
}
void debugln() {
  debugln("");
}
