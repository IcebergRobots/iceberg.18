#include "Utility.h"

// Implementierung: OBJEKTE
extern Display d;
extern Pilot m;
extern Mate mate;
extern Led led;

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
  usFine = us[0] * us[1] * us[2] * us[3] != 0;
  hasBall = analogRead(LIGHT_BARRIER) > LIGHT_BARRIER_TRIGGER_LEVEL;
}

/*****************************************************
  Sende einen Herzschlag mit Statusinformationen an den Partner
*****************************************************/
void transmitHeartbeat() {
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
  mate.send(data, 9); // heartbeat
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

bool readUltrasonic() {
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
  usTimer = millis(); // merke Zeitpunkt
  rotaryEncoder.tick(); // erkenne Reglerdrehungen

  digitalWrite(INT_US, 1);  // sende eine Interrupt Aufforderung an den US-Arduino
  usTimer = millis();
  while (millis() - usTimer < 3) {  // warte max. 3ms auf eine Antwort
    if (US_SERIAL.available() >= 4) { // alle Sensorwerte wurden übertragen
      while (US_SERIAL.available() > 4) {
        US_SERIAL.read();

      }
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
  if (BOTTOM_SERIAL.available() > 0) {
    lineDir = BOTTOM_SERIAL.read() * 90 + 90;
    driveDir = lineDir;
    m.drive(driveDir, SPEED_LINE, 0);
    lineTimer = millis();
    headstartTimer = 0;
    if (drivePwr > 200) {
      lineTimer = millis() + (2 * LINE_DURATION);
    } else if (drivePwr > 100) {
      lineTimer = millis() + (1.5 * LINE_DURATION);
    } else {
      lineTimer = millis() + LINE_DURATION;
    }
    displayDebug = driveDir;
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
  }
}

void buzzerTone(int duration) {
  digitalWrite(BUZZER_AKTIV, 1);
  buzzerStopTimer = max(buzzerStopTimer, millis() + duration);
}
