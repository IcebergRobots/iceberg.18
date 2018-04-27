#include "Led.h"

// Implementierung: OBJEKTE
extern Display d;
extern Keeper keeper;
//extern Led led;
extern Mate mate;
extern Pilot m;
extern Ultrasonic us;

Led::Led() {}

/*****************************************************
  Aktualisiere alle Leds bzw. zeige die Animation
*****************************************************/
void Led::led() {
  if (!animationPosition) {
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
  } else if (animationPosition == 1) {
    animationTimer = millis();
    animationPosition++;
    bottom.setBrightness(255);
    matrix.setBrightness(255);
    info.setBrightness(255);
    setBoard(bottom, BOTTOM_LENGTH, bottom.Color(255, 255, 255));
    setBoard(matrix, BOTTOM_LENGTH, matrix.Color(255, 255, 255));
    setBoard(info, BOTTOM_LENGTH, info.Color(255, 255, 255));
  } else if (millis() - animationTimer > ANIMATION_DURATION) {
    // beende die Animation
    if (!digitalRead(SWITCH_BOTTOM) || isLifted) setBoard(bottom, BOTTOM_LENGTH, 0);
    setBoard(matrix, MATRIX_LENGTH, 0);
    setBoard(info, INFO_LENGTH, 0);
    animationPosition = 0;
  } else {
    // setze den Farbkreis
    wheelBoard(bottom, BOTTOM_LENGTH, animationPosition);
    setBoard(matrix, MATRIX_LENGTH, wheelToColor(matrix, animationPosition));
    setBoard(info, INFO_LENGTH, wheelToColor(info, animationPosition));

    // erhöhe die Position in der Animation
    animationPosition += 1 + animationPosition * ANIMATION_SPEED;
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
  showState(matrix, 4, !mate.timeout());
  //showState(matrix, 5, Bodensensor verfügbar);
  showState(matrix, 6, isLifted * 3, true);
  showState(matrix, 7, pixyState, true);
  showState(matrix, 8, !onLine);
  showState(matrix, 9, seeGoal, true);
  showState(matrix, 10, !us.timeout() * (2 - us.check()));
  showState(matrix, 11, us.right() + us.left() >= COURT_WIDTH_FREE);
}

/*****************************************************
  Starte die Animation
*****************************************************/
void Led::animation() {
  if (ANIMATION && !silent) animationPosition = 1;
}

/*****************************************************
  Läuft die Animation gerade?
*****************************************************/
bool Led::isAnimation() {
  return animationPosition;
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
         Information falsch (weiß)
         hideRed=true Information nicht verfügbar (aus)
         hideRed=true Information nicht relevant (aus)
      */
      board.setPixelColor(pos, (!hideRed) * 150, 0, (!hideRed) * 150);
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
      board.setPixelColor(pos, 255, 0, 150);
      break;
    case 3:
      /*
         Kritische Warnung (weiß)
         Information falsch (weiß)
      */
      board.setPixelColor(pos, 0, 180, 120);
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
