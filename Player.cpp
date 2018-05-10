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
  if (m.getMotEn() && penalty) {
    if (force) {
      penalty = false;
      roleTimer = millis();
      ledTimer = millis() - 101;
    } else {
      if (millis() - roleTimer > ROLE_COOLDOWN) {
        penalty = false;
        roleTimer = millis();
        ledTimer = millis() - 101;
      }
    }
  }
}

void Player::setKeeper(bool force) {
  if (m.getMotEn() && !penalty) {
    if (force) {
      penalty = true;
      roleTimer = millis();
      ledTimer = millis() - 101;
    } else {
      if (millis() - roleTimer > ROLE_COOLDOWN) {
        penalty = true;
        roleTimer = millis();
        ledTimer = millis() - 101;
      }
    }
  }
}

bool Player::isRusher() {
  return m.getMotEn() && penalty == false;
}

bool Player::isKeeper() {
  return m.getMotEn() && penalty == true;
}

byte Player::getRole() {
  if (!m.getMotEn()) return 0;
  else return 1 + !penalty;
}

unsigned long Player::lastRoleToggle() {
  return millis() - roleTimer;
}

void Player::blind() {
  // set state
  switch (state) {
    case 0: // fahre rückwärts
      if (us.back() <= BACK_IDEAL) setState(1);
      else if (millis() - stateTimer > BACKWARD_MAX_DURATION) setState(4);
      break;

    case 1: // fahre seitwärts
      if ( millis() - stateTimer > SIDEWARD_MAX_DURATION) {
        if (us.back() > BACK_IDEAL) setState(0); // fahre rückwärts
        else if (penalty) setState(2);     // wechsle in Drehmodus
        else toggleStateDirection();  // wechsle Fahrrichtung
      } else if ( millis() - stateTimer > SIDEWARD_MIN_DURATION) {
        if (us.back() > BACK_IDEAL) setState(0); // fahre rückwärts
        else if (onLine) toggleStateDirection();
        else if (atGatepost()) {
          if (penalty) setState(2);     // wechsle in Drehmodus
          else toggleStateDirection();  // wechsle Fahrrichtung
        }
      }
      break;

    case 2: // Pfostendrehung hin
      if ( millis() - stateTimer > TURN_MAX_DURATION) setState(3);
      else if (stateLeft && heading < -ANGLE_TURN_MAX) setState(3);
      else if (!stateLeft && heading > ANGLE_TURN_MAX) setState(3);
      break;

    case 3: // Pfostendrehung zurück
      if ( millis() - stateTimer > TURN_BACK_MAX_DURATION) toggleStateDirection();
      else if (abs(heading) < 20) toggleStateDirection();
      break;

    case 4: // befreie dich vom Tor
      if (millis() - stateTimer > GOAL_STUCK_DURATION) setState(0); // fahre wieder rückwärts und warte erneut * Sekunden
      break;
  }

  // set pilot values
  switch (state) {
    case 0: // fahre rückwärts
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
      
    case 1: // fahre seitwärts
      drivePower = SPEED_KEEPER;
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
      if (us.back() < 15) driveDirection *= 0.8;  // fahre leicht schräg nach vorne
      break;

    case 2: // Pfostendrehung hin
      drivePower = SPEED_FREE;
      driveDirection = 0;
      if (stateLeft) {
        driveOrientation = ANGLE_TURN_MAX;
        driveState = "< turn";
      } else {
        driveOrientation = -ANGLE_TURN_MAX;
        driveState = "> turn";
      }
      break;

    case 3: // Pfostendrehung zurück
      drivePower = 0;
      driveDirection = 0;
      driveOrientation = 0;
      if (stateLeft) driveState = "< return";
      else driveState = "> return";
      break;

    case 4: // befreie dich vom Tor
      // fahre * Sekunden geradeaus, um nicht mehr am Tor hängenzubleiben
      drivePower = SPEED_FREE;
      driveDirection = 0;
      driveOrientation = 0;
      driveState = "^ free";
      break;
  }
}

void Player::setState(byte s) {
  if (s != state) {
    state = s;
    stateTimer = millis();
  }
}

void Player::toggleStateDirection() {
  state = 1;
  stateTimer = millis();
  stateLeft = !stateLeft;
}

bool Player::atGatepost() {
  if (millis() - lastGoalFree < 200) {
    // benutze Abstand in Bewegungsrichtung
    if (stateLeft) return us.left() < COURT_GOAL_TO_BORDER;
    else           return us.right() < COURT_GOAL_TO_BORDER;
  } else {
    // benutze Abstand gegen Bewegungsrichtung
    if (stateLeft) return us.right() > COURT_POST_TO_BORDER;
    else           return us.left() > COURT_POST_TO_BORDER;
  }
}

byte Player::getState() {
  return state;
}

