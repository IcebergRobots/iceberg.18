#include "Led.h"

Led::Led() {}

/*****************************************************
  Aktualisiere alle Leds bzw. zeige die Animation
*****************************************************/
void Led::led() {
  if (animationPos == 0 || !ANIMATION) {
    // setze Helligkeit zurück
    bottom.setBrightness(BOTTOM_BRIGHTNESS);
    matrix.setBrightness(MATRIX_BRIGHTNESS);
    info.setBrightness(INFO_BRIGHTNESS);

    // setze Boden-Leds
    if (!digitalRead(SWITCH_BOTTOM) || isLifted) {
      setBoard(bottom, BOTTOM_LENGTH, bottom.Color(0, 0, 0));
    } else if (!digitalRead(SWITCH_A)) {
      setBoard(bottom, BOTTOM_LENGTH, bottom.Color(255, 0, 0));
    } else {
      setBoard(bottom, BOTTOM_LENGTH, bottom.Color(255, 255, 255));
    }
  } else {
    if (animationPos == 1) {
      animationTimer = millis();
    }

    // setze Helligkeit maximal
    bottom.setBrightness(255);
    matrix.setBrightness(255);
    info.setBrightness(255);

    // setze den Farbkreis
    wheelBoard(bottom, BOTTOM_LENGTH, animationPos);
    setBoard(matrix, MATRIX_LENGTH, wheelToColor(matrix, animationPos));
    setBoard(info, INFO_LENGTH, wheelToColor(info, animationPos));

    // erhöhe die Position in der Animation
    animationPos += 1 + animationPos * ANIMATION_SPEED;

    // beende die Animation
    if (millis() - animationTimer > ANIMATION_DURATION) {
      if (!digitalRead(SWITCH_BOTTOM) || isLifted) setBoard(bottom, BOTTOM_LENGTH, 0);
      setBoard(matrix, MATRIX_LENGTH, 0);
      setBoard(info, INFO_LENGTH, 0);
      animationPos = 0;
    }
  }
  // übernehme alle Änderungen
  bottom.show();
  matrix.show();
  info.show();
  ledTimer = millis();
}

/*****************************************************
  Lege Leds auf Statusinformation fest
*****************************************************/
void Led::set() {
  // zeige Statuswerte an
  showState(info, 0, stateFine);
  showState(info, 1, batState * (batState != 3 || millis() % 250 < 125), true);
  showState(info, 2, millis() % 1000 < 200, true);

  showState(matrix, 0, digitalRead(SWITCH_SCHUSS));
  showState(matrix, 1, !digitalRead(SWITCH_MOTOR));
  showState(matrix, 2, seeBall, true);
  showState(matrix, 3, hasBall, true);
  showState(matrix, 4, mate.connected);
  //showState(matrix, 5, Bodensensor verfügbar);
  showState(matrix, 6, isLifted * 3, true);
  showState(matrix, 7, pixyState, true);
  showState(matrix, 8, !onLine);
  showState(matrix, 9, seeGoal, true);
  showState(matrix, 10, usFine);
  showState(matrix, 11, DEBUG * 3, true);
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
void Led::setBoard(Adafruit_NeoPixel & board, int boardLength, uint32_t color) {
  for (int i = 0; i < boardLength; i++) {
    board.setPixelColor(i, color);
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
