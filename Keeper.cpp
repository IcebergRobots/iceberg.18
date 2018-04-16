#include "Keeper.h"

// Implementierung: OBJEKTE
extern Display d;
extern Keeper keeper;
extern Led led;
extern Mate mate;
extern Pilot m;
extern Ultrasonic us;

Keeper::Keeper() {
}

void Keeper::set() {
  drivePower = SPEED_KEEPER;
  if (movingLeft) {
    driveState = "keeper <";
    driveDirection = ANGLE_SIDEWAY;
    driveOrientation = -constrain(map(us.left() - COURT_GOAL_TO_BORDER, 0, 30, ANGLE_KEEPER_MAX, 0), 0, ANGLE_KEEPER_MAX);
  } else {
    driveState = "keeper >";
    driveDirection = -ANGLE_SIDEWAY;
    driveOrientation = constrain(map(us.right() - COURT_GOAL_TO_BORDER, 0, 30, ANGLE_KEEPER_MAX, 0), 0, ANGLE_KEEPER_MAX);
  }
  if (us.back() < 15) driveDirection *= 0.8;
}

void Keeper::right() {
  movingLeft = false;
  toggleTimer = millis();
}

void Keeper::left() {
  movingLeft = true;
  toggleTimer = millis();
}

void Keeper::toggle() {
  movingLeft = !movingLeft;
  toggleTimer = millis();
}

bool Keeper::atGatepost() {
  // ersetze kaputte US-Sensoren mit sinvollen Werten

  if (movingLeft) {
    return us.left() < COURT_GOAL_TO_BORDER;
  } else {
    return us.right() < COURT_GOAL_TO_BORDER;
  }
}

unsigned long Keeper::lastToggle() {
  return millis() - toggleTimer;
}

