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

void Player::toggleDirection() {
  stateLeft = !stateLeft;
  stateTimer = millis();
  state = 1;
}

unsigned long Player::lastRoleToggle() {
  return millis() - roleTimer;
}

void Player::setState() {
  // set state
  byte tempState = state;
  if (state < 6 && seeBall && isRusher()) state = 6; // wir werden aktiv
  if (state > 5 && !seeBall) state = 5; // wir werden blind
  if (state > 5 && isKeeper()) state = 0; // wir werden passiv
  if (tempState != state) stateTimer = millis();

  switch (state) {
    // Passivspiel
    case 0: // Nach hinten
      if (us.back() <= COURT_REARWARD_MAX) state = 1;
      else if (millis() - stateTimer > BACKWARD_MAX_DURATION) state = 4;
      break;

    case 1: // Torverteidigung
      if (!seeBall && millis() - stateTimer > SIDEWARD_MAX_DURATION) {
        if (us.back() > COURT_REARWARD_MAX) state = 0; // fahre rückwärts
        else if (isKeeper()) state = 2;     // wechsle in Drehmodus
        else toggleDirection();  // wechsle Fahrrichtung
      } else if (millis() - stateTimer > SIDEWARD_MIN_DURATION) {
        if (us.back() > COURT_REARWARD_MAX) state = 0; // fahre rückwärts
        else if (onLine) toggleDirection();
        else if (seeBall && ball < -BALL_ANGLE_TRIGGER) stateLeft = true;
        else if (seeBall && ball > BALL_ANGLE_TRIGGER) stateLeft = false;
        else if (atGatepost()) {
          if (isKeeper()) state = 2;  // wechsle in Drehmodus
          else toggleDirection();     // wechsle Fahrrichtung
        }
      }
      break;

    case 2: // Pfostendrehung hin
      if (seeBall && stateLeft && ball > BALL_ANGLE_TRIGGER) state = 3;
      else if (seeBall && !stateLeft && ball < -BALL_ANGLE_TRIGGER) state = 3;
      else if (millis() - stateTimer > TURN_MAX_DURATION) state = 3;
      else if (stateLeft && heading < -ANGLE_TURN_MAX * 0.9) state = 3;
      else if (!stateLeft && heading > ANGLE_TURN_MAX * 0.9) state = 3;
      break;

    case 3: // Pfostendrehung zurück
      if (seeBall && stateLeft && ball < -BALL_ANGLE_TRIGGER * 2) state = 2;
      else if (seeBall && !stateLeft && ball > BALL_ANGLE_TRIGGER * 2) state = 2;
      else if (millis() - stateTimer > TURN_BACK_MAX_DURATION) toggleDirection();
      else if (abs(heading) < 20) toggleDirection();
      break;

    case 4: // Befreiung
      if (millis() - stateTimer > GOAL_STUCK_DURATION) state = 0; // fahre wieder rückwärts und warte erneut * Sekunden
      break;

    case 5: // Seitlich verloren
      if (millis() - stateTimer > AVOID_MATE_DURATION) state = 0;
      else if (seeBall && stateLeft && ball > BALL_ANGLE_TRIGGER) stateLeft = false;
      else if (seeBall && !stateLeft && ball < -BALL_ANGLE_TRIGGER) stateLeft = true;
      break;


    // Aktivspiel
    case 6: // Ballverfolgung
      if (closeBall && seeGoal) state = 7;
      //if (ball < BALL_ANGLE_TRIGGER)
      break;

    case 7: // Torausrichtung
      if (!closeBall || !seeGoal) state = 6;
      else if (goal < -BALL_ANGLE_TRIGGER) stateLeft = true;
      else if (goal > BALL_ANGLE_TRIGGER) stateLeft = false;
      else state = 8;
      break;

    case 8: // Angriff
      if (!closeBall) state = 6;
      break;
  }
  if (tempState != state) stateTimer = millis();
}

void Player::play() {
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
      if (us.left() && us.left() < COURT_GOAL_TO_BORDER) driveDirection = -constrain(map(COURT_GOAL_TO_BORDER - us.left(), 0, 30, 180, 180 - ANGLE_PASSIVE_MAX), 180 - ANGLE_PASSIVE_MAX, 180);
      else if (us.right() && us.right() < COURT_GOAL_TO_BORDER) driveDirection = constrain(map(COURT_GOAL_TO_BORDER - us.right(), 0, 30, 180, 180 - ANGLE_PASSIVE_MAX), 180 - ANGLE_PASSIVE_MAX, 180);
      else driveDirection = 180;
      driveOrientation = 0;
      break;

    case 1: // Torverteidigung
      // fahre seitlich vor dem Tor
      drivePower = SPEED_KEEPER;
      if (seeBall && abs(ball) < BALL_ANGLE_TRIGGER * 2) drivePower *= 0.5;
      driveOrientation = 0;
      if (stateLeft) {
        driveDirection = ANGLE_SIDEWAY;
        driveState = "< sideward";
        if (us.left() < 60) drivePower *= 0.7;  // fahre langsamer am Spielfeldrand
      } else {
        driveDirection = -ANGLE_SIDEWAY;
        driveState = "> sideward";
        if (us.right() < 60) drivePower *= 0.7; // fahre langsamer am Spielfeldrand
      }
      if (us.back() < COURT_REARWARD_MIN) driveDirection *= 0.8;  // fahre leicht schräg nach vorne
      break;

    case 2: // Pfostendrehung hin
      drivePower = 0;
      driveDirection = 0;
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
      driveState = "^ free";
      break;

    case 7: // Torausrichtung
      // orientiere dich zum Ball
      // bringe Ball und Tor in eine Linie
      drivePower = SPEED_CLOSE;
      if (stateLeft) {
        driveDirection = ANGLE_SIDEWAY;
        driveState = "< close";
      } else {
        driveDirection = -ANGLE_SIDEWAY;
        driveState = "> close";
      }
      driveOrientation = constrain(ball / 3 + heading, -ANGLE_GOAL_MAX, ANGLE_GOAL_MAX);
      break;

    case 8: // Angriff
      drivePower = SPEED_ATTACK;
      if (seeGoal) driveDirection = constrain(map(goal, -X_CENTER, X_CENTER, 50, -50), -50, 50);
      else driveDirection = 0;
      driveState = "^ straight";
      if (hasBall) kick();
      break;
  }
}

byte Player::getState() {
  return state;
}

bool Player::atGatepost() {
  if (true || isPenaltyFree) {
    // benutze Abstand in Bewegungsrichtung
    if (stateLeft) return us.left() < COURT_GOAL_TO_BORDER;
    else           return us.right() < COURT_GOAL_TO_BORDER;
  } else {
    // benutze Abstand gegen Bewegungsrichtung
    if (stateLeft) return us.right() > COURT_POST_TO_BORDER;
    else           return us.left() > COURT_POST_TO_BORDER;
  }
}
