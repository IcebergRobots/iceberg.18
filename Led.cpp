#include "Led.h"

/*****************************************************
  Led zeigt rot, grün oder aus
  @param board: Led-Board
  @param pos: Nummer der Led im Board
  @param (optional) state: darzustellender Zustand
  0: rot/aus
  1: grün
  2: magenta
  3: rot
  @param hideRed: soll rot unsichtbar sein?
*****************************************************/
void Led::calculateStates() {
    // ermittle Statuswerte für Leds
  hasBall = digitalRead(SWITCH_SCHUSS) && analogRead(LIGHT_BARRIER) > LIGHT_BARRIER_TRIGGER_LEVEL;
  seeBall = millis() - seeBallTimer < 50;
  seeGoal = millis() - seeGoalTimer < 1200;
  isDrift = millis() - driftTimer < 100;
  isConnected = millis() - heartbeatTimer < 500;
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
}

/*****************************************************
  Led zeigt rot, grün oder aus
  @param board: Led-Board
  @param pos: Nummer der Led im Board
  @param (optional) state: darzustellender Zustand
  0: rot/aus
  1: grün
  2: magenta
  3: rot
  @param hideRed: soll rot unsichtbar sein?
*****************************************************/
void Led::showStates() {
    // zeige Statuswerte an
  showLed(info, 0, stateFine);
  showLed(info, 1, batState * (batState != 3 || millis() % 250 < 125), true);
  showLed(info, 2, millis() % 1000 < 200, true);

  showLed(matrix, 0, digitalRead(SWITCH_SCHUSS));
  showLed(matrix, 1, !digitalRead(SWITCH_MOTOR));
  showLed(matrix, 2, seeBall, true);
  showLed(matrix, 3, hasBall, true);
  showLed(matrix, 4, isConnected);
  //showLed(matrix, 5, Bodensensor verfügbar);
  //showLed(matrix, 6, lift);
  showLed(matrix, 7, pixyState, true);
  showLed(matrix, 8, !onLine);
  showLed(matrix, 9, seeGoal, true);
  showLed(matrix, 10, isHeadstart, true);
  showLed(matrix, 11, true);
}

/*****************************************************
  Led zeigt rot, grün oder aus
  @param board: Led-Board
  @param pos: Nummer der Led im Board
  @param (optional) state: darzustellender Zustand
  0: rot/aus
  1: grün
  2: magenta
  3: rot
  @param hideRed: soll rot unsichtbar sein?
*****************************************************/
void Led::showState(Adafruit_NeoPixel & board, byte pos, byte state, bool hideRed) {
  switch (state) {
    default:  //case: 0
      /*
         Information falsch (rot)
         hideRed=true Information nicht verfügbar (aus)
         hideRed=true Information nicht relevant (aus)
      */
      board.setPixelColor(pos, (!hideRed) * 255, 0, 0);
      break;
    case 1:
      /*
         Information wahr (grün)
      */
      board.setPixelColor(pos, 0, 255, 0);
      break;
    case 2:
      /*
         Wahrnung (magenta)
         Information ungewiss (magenta)
      */
      board.setPixelColor(pos, 255, 0, 255);
      break;
    case 3:
      /*
         Kritische Warnung (rot)
         Information falsch (rot)
      */
      board.setPixelColor(pos, 255, 0, 0);
      break;
  }
}
void Led::showState(Adafruit_NeoPixel & board, byte pos, byte state) {
  showState(board, pos, state, false);
}

/*****************************************************
  Zeige Farbkreislauf auf Board
  @param board: Led-Board
  @param boardLength: Leds im Board
  @param offset: Drehung des Farbkreises
*****************************************************/
void Led::wheelBoard(Adafruit_NeoPixel & board, int boardLength, int offset) {
  for (int i = 0; i < boardLength; i++) {
    board.setPixelColor(i, wheelToColor(board, offset + i * 256 / boardLength));
  }
  board.show();
}

/*****************************************************
  Schalte ein Board aus
  @param board: Led-Board
  @param boardLength: Leds im Board
*****************************************************/
void Led::turnOffBoard(Adafruit_NeoPixel & board, int boardLength) {
  for (int i = 0; i < boardLength; i++) {
    board.setPixelColor(i, 0);
  }
  board.show();
}

/*****************************************************
  wandle Zustand in Farbe eines Farbkreises um
  @param board: Led-Board
  @param pos: [0 bis 255]
  0:    rot
  85:   grün
  170:  blau
  255:  rot
*****************************************************/
uint32_t Led::wheelToColor(Adafruit_NeoPixel & board, byte pos) {
  pos = (pos % 256 + 256) % 256; // Eingabekorrektur
  if (pos < 85) {
    // rot bis grün
    return board.Color(255 - pos * 3, pos * 3, 0);
  } else if (pos < 170) {
    // grün bis blau
    pos -= 85;
    return board.Color(0, 255 - pos * 3, pos * 3);
  } else {
    // blau bis rot
    pos -= 170;
    return board.Color(pos * 3, 0, 255 - pos * 3);
  }
}
