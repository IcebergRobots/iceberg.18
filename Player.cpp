#include "Player.h"

// Implementierung: OBJEKTE
extern Display d;
//extern Player p;
extern Led led;
extern Mate mate;
extern Pilot m;
extern Ultrasonic us;

Player::Player() {
}

void Player::changeState() {
  // set state
  byte tempState = state;
  if (state >= 6) {
    // aktiv
    if (isKeeper() && !mate.timeout()) setState(0, "passive"); // werden passiv
    else if (!seeBall) setState(5, "blind"); // wir werden blind
  } else {
    // passiv
    if (seeBall && state != 3 && state != 4 && (isRusher() || mate.timeout())) setState(6, "active"); // wir werden aktiv
    // sehenBall &   keine Pfostendrehung   &  (Stürmer   oder  Singleplayer)
  }

  switch (state) {
    // Passivspiel
    case 0: // Nach hinten
      if (us.back() <= COURT_REARWARD_MAX) setState(1, "rearward<");
      else if (millis() - stateTimer > BACKWARD_MAX_DURATION) setState(4, "time>");
      break;

    case 1: // Torverteidigung
      if (!seeBall && millis() - stateTimer > SIDEWARD_MAX_DURATION) {
        if (us.back() > COURT_REARWARD_MAX) setState(0, "rearward>"); // fahre rückwärts
        else if (isKeeper()) setState(2, "time>,keeper");     // wechsle in Drehmodus
        else setDirection(TOGGLE, "time>,rusher");  // wechsle Fahrrichtung
      } else if (millis() - stateTimer > SIDEWARD_MIN_DURATION) {
        if (us.back() > COURT_REARWARD_MAX) setState(0, "rearward>"); // fahre rückwärts
        else if (millis() - lineTimer < 100) setDirection(TOGGLE, "line");
        else if (seeBall) {
          if (ball < -BALL_ANGLE_TRIGGER) {
            setDirection(LEFT, "ball<");
            stateTimer += 200 - SIDEWARD_MIN_DURATION;
          } else if (ball > BALL_ANGLE_TRIGGER) {
            setDirection(RIGHT, "ball>");
            stateTimer += 200 - SIDEWARD_MIN_DURATION;
          }
        } else if (atGatepost() && isKeeper()) setState(2, "gatepost");  // wechsle in Drehmodus
      }
      break;

    case 2: // Pfostendrehung hin
      if (seeBall) {
        if (stateLeft && ball > BALL_ANGLE_TRIGGER) setState(3, "ball>");
        else if (!stateLeft && ball < -BALL_ANGLE_TRIGGER) setState(3, "ball<");
      }
      else if (millis() - stateTimer > TURN_MAX_DURATION) setState(3, "time>");
      else if (stateLeft && heading < -ANGLE_TURN_MAX * 0.9) setState(3, "angle<");
      else if (!stateLeft && heading > ANGLE_TURN_MAX * 0.9) setState(3, "angle>");
      break;

    case 3: // Pfostendrehung zurück
      if (seeBall) {
        if (stateLeft) {
          if ((ball / 3 + heading) > -ANGLE_RETURN_MIN) setDirection(TOGGLE, "ball|");
          else if (ball < -BALL_ANGLE_TRIGGER) setState(2, "ball<");
        } else {
          if ((ball / 3 + heading) < ANGLE_RETURN_MIN) setDirection(TOGGLE, "ball|");
          else if (ball > BALL_ANGLE_TRIGGER) setState(2, "ball>");
        }
      }
      else if (millis() - stateTimer > RETURN_MAX_DURATION) setDirection(TOGGLE, "time>");
      else if (abs(heading) < ANGLE_RETURN_MIN) setDirection(TOGGLE, "angle|");
      break;

    case 4: // Befreiung
      if (millis() - stateTimer > GOAL_STUCK_DURATION) setState(0, "time>"); // fahre wieder rückwärts und warte erneut * Sekunden
      break;

    case 5: // Seitlich verloren
      if (millis() - stateTimer > AVOID_MATE_DURATION) setState(0, "time>");
      else if (seeBall && stateLeft && ball > BALL_ANGLE_TRIGGER) setDirection(RIGHT, "ball>");
      else if (seeBall && !stateLeft && ball < -BALL_ANGLE_TRIGGER) setDirection(LEFT, "ball<");
      break;


    // Aktivspiel
    case 6: // Ballverfolgung
      if (seeBall && ball > BALL_ANGLE_TRIGGER) stateLeft = RIGHT;
      else if (seeBall && ball < -BALL_ANGLE_TRIGGER) stateLeft = LEFT;

      if (closeBall && seeGoal) setState(7, "closeBall");
      break;

    case 7: // Torausrichtung
      if (!closeBall) setState(6, "farBall");
      else if (!seeGoal) setState(6, "!goal");
      else if (goal < -BALL_ANGLE_TRIGGER) stateLeft = LEFT;
      else if (goal > BALL_ANGLE_TRIGGER) stateLeft = RIGHT;
      else setState(8, "goal|");
      break;

    case 8: // Angriff
      if (seeBall && ball > BALL_ANGLE_TRIGGER) stateLeft = RIGHT;
      else if (seeBall && ball < -BALL_ANGLE_TRIGGER) stateLeft = LEFT;
    
      if (!closeBall) setState(6, "!closeBall");
      break;
  }
  if (tempState == 0 && state == 1) stateTimer = max(0, millis() - SIDEWARD_MIN_DURATION / 2);
  else if (tempState != state) stateTimer = millis();
}

void Player::play() {
  changeState();

  driveRotation = 255;
  switch (state) {
    case 0: // Nach hinten
      if (us.back() && us.back() < 80) {
        drivePower = SPEED_PENALTY;
        driveState = "v penalty";
      } else {
        drivePower = SPEED_BACKWARDS;
        driveState = "v backward";
      }
      // fahre rückwärts und lenke zur Mitte vor dem Tor
      if (us.left() && us.left() < COURT_BORDER_MIN) driveDirection = -constrain(map(COURT_BORDER_MIN - us.left(), 0, 30, 180, 180 - ANGLE_PASSIVE_MAX), 180 - ANGLE_PASSIVE_MAX, 180);
      else if (us.right() && us.right() < COURT_BORDER_MIN) driveDirection = constrain(map(COURT_BORDER_MIN - us.right(), 0, 30, 180, 180 - ANGLE_PASSIVE_MAX), 180 - ANGLE_PASSIVE_MAX, 180);
      else driveDirection = 180;
      driveOrientation = 0;
      break;

    case 1: // Torverteidigung
      // fahre seitlich vor dem Tor
      if (seeBall) drivePower = map(abs(ball), 0, BALL_ANGLE_TRIGGER, SPEED_KEEPER, 0.6 * SPEED_KEEPER);
      else drivePower = SPEED_KEEPER;
      driveOrientation = 0;
      if (stateLeft) {
        driveDirection = ANGLE_SIDEWAY;
        driveState = "< sideward";
        if (us.left() < COURT_BORDER_MIN) drivePower = SPEED_KEEPER * 0.7; // fahre langsamer am Spielfeldrand
      } else {
        driveDirection = -ANGLE_SIDEWAY;
        driveState = "> sideward";
        if (us.right() < COURT_BORDER_MIN) drivePower = SPEED_KEEPER * 0.7; // fahre langsamer am Spielfeldrand
      }
      if (us.back() < COURT_REARWARD_MIN) driveDirection *= map(us.back(), 0, COURT_REARWARD_MIN, 8, 10) / 10.0; // fahre leicht schräg nach vorne
      break;

    case 2: // Pfostendrehung hin
      drivePower = 0;
      driveDirection = 0;
      driveRotation = 200;
      if (seeBall && ball < BALL_ANGLE_TRIGGER) driveRotation = 160;
      if (stateLeft) {
        if (seeBall) driveOrientation = constrain(ball / 3 + heading, -ANGLE_TURN_MAX, 0);
        else driveOrientation = -ANGLE_TURN_MAX;
        driveState = "< turn";
      } else {
        if (seeBall) driveOrientation = constrain(ball / 3 + heading, 0, ANGLE_TURN_MAX);
        else driveOrientation = ANGLE_TURN_MAX;
        driveState = "> turn";
      }
      break;

    case 3: // Pfostendrehung zurück
      drivePower = 0;
      driveDirection = 0;
      driveRotation = 200;
      if (seeBall && ball < BALL_ANGLE_TRIGGER) driveRotation = 160;
      if (stateLeft) {
        if (seeBall) driveOrientation = constrain(ball / 3 + heading, -ANGLE_TURN_MAX, 0);
        else driveOrientation = 0;
        driveState = "< return";
      } else {
        if (seeBall) driveOrientation = constrain(ball / 3 + heading, 0, ANGLE_TURN_MAX);
        else driveOrientation = 0;
        driveState = "> return";
      }
      break;

    case 4: // Befreiung
      // fahre * Sekunden geradeaus, um nicht mehr am Tor hängenzubleiben
      drivePower = SPEED_FREE;
      driveDirection = 0;
      driveOrientation = 0;
      driveState = "^ free";
      break;

    case 5: // Seitlich verloren
      // fahre * Sekunden zur Seite, um den Ball wiederzufinden
      drivePower = SPEED_LOST;
      if (stateLeft) {
        driveDirection = ANGLE_SIDEWAY;
        driveState = "< lost";
        if (us.left() < 60) drivePower *= 0.7;  // fahre langsamer am Spielfeldrand
      } else {
        driveDirection = -ANGLE_SIDEWAY;
        driveState = "> lost";
        if (us.right() < 60) drivePower *= 0.7; // fahre langsamer am Spielfeldrand
      }
      driveOrientation = 0;
      break;

    case 6: // Ballverfolgung
      if (!seeBall) rotMulti = ROTATION_SIDEWAY;
      else if (ballWidth > 100) rotMulti = ROTATION_TOUCH;
      else if (ballWidth > 40) rotMulti = ROTATION_10CM;
      else if (ballWidth > 20) rotMulti = ROTATION_18CM;
      else rotMulti = ROTATION_AWAY;

      drivePower = SPEED_BALL;
      driveDirection = map(ball, -X_CENTER, X_CENTER, (float)rotMulti, -(float)rotMulti);
      if (driveDirection > 60) {
        // seitwärts bewegen, um Torsusrichtung aufrecht zu erhalten
        driveState = "> follow";
        driveDirection = 100;
        drivePower = SPEED_SIDEWAY;
      } else if (driveDirection < -60) {
        // seitwärts bewegen, um Torsusrichtung aufrecht zu erhalten
        driveState = "< follow";
        driveDirection = -100;
        drivePower = SPEED_SIDEWAY;
      } else {
        driveState = "^ follow";
      }
      driveOrientation = 0;
      break;

    case 7: // Torausrichtung
      // orientiere dich zum Ball
      // bringe Ball und Tor in eine Linie
      drivePower = SPEED_CLOSE;
      if (!stateLeft) {
        // Tor ist links
        driveDirection = ANGLE_SIDEWAY;
        driveState = "< close";
      } else {
        // Tor ist rechts
        driveDirection = -ANGLE_SIDEWAY;
        driveState = "> close";
      }
      driveOrientation = constrain(ball / 3 + heading, -ANGLE_GOAL_MAX, ANGLE_GOAL_MAX);
      break;

    case 8: // Angriff
      drivePower = SPEED_ATTACK;
      if (seeGoal) driveDirection = constrain(map(goal, -X_CENTER, X_CENTER, 50, -50), -50, 50);
      else driveDirection = 0;
      driveState = "^ attack";
      if (hasBall) kick();
      break;
  }
}

void Player::setState(byte s, String reason) {
  if (s != state) {
    if (DEBUG_STATE) debug(String(state) + "->" + String(s) + ":" + reason);
    state = s;
    stateTimer = millis();
  }
}

void Player::setDirection(byte dir, String reason) {
  if (dir > 1) dir = !stateLeft;
  if (dir != stateLeft) {
    stateLeft = dir;
    if (DEBUG_STATE && stateLeft)  debug(String(state) + "<:" + reason);
    if (DEBUG_STATE && !stateLeft) debug(String(state) + ">:" + reason);
    stateTimer = millis();
    setState(1, "toggle");
  }
}

void Player::setRusher(bool force) {
  if (isKeeper()) {
    if (force || millis() - roleTimer > ROLE_COOLDOWN) {
      role = 1;
      roleTimer = millis();
      ledTimer = millis() - 101;
    }
  }
}

void Player::setKeeper(bool force) {
  if (isRusher()) {
    if (force || millis() - roleTimer > ROLE_COOLDOWN) {
      role = 0;
      roleTimer = millis();
      ledTimer = millis() - 101;
    }
  }
}

bool Player::isRusher() {
  return role;
}

bool Player::isKeeper() {
  return !role;
}

unsigned long Player::lastRoleToggle() {
  return millis() - roleTimer;
}

byte Player::getState() {
  return state;
}

bool Player::atGatepost() {
  if (true || isPenaltyFree) {
    // benutze Abstand in Bewegungsrichtung
    if (stateLeft) return us.left() < COURT_BORDER_MIN;
    else           return us.right() < COURT_BORDER_MIN;
  } else {
    // benutze Abstand gegen Bewegungsrichtung
    if (stateLeft) return us.right() > COURT_POST_TO_BORDER;
    else           return us.left() > COURT_POST_TO_BORDER;
  }
}
