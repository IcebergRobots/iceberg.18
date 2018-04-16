#include "Keeper.h"

Keeper::Keeper() {
}

void Keeper::set() {
  drivePower = SPEED_KEEPER;
  if (movingLeft) {
    driveState = "keeper <";
    driveDirection = ANGLE_SIDEWAY;
    driveOrientation = -constrain(map(usLeft - COURT_GOAL_TO_BORDER, 0, 30, ANGLE_KEEPER_MAX, 0), 0, ANGLE_KEEPER_MAX);
  } else {
    driveState = "keeper >";
    driveDirection = -ANGLE_SIDEWAY;
    driveOrientation = constrain(map(usRight - COURT_GOAL_TO_BORDER, 0, 30, ANGLE_KEEPER_MAX, 0), 0, ANGLE_KEEPER_MAX);
  }
  if (usBack < 15) driveDirection *= 0.8;
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
    return usLeft < COURT_GOAL_TO_BORDER;
  } else {
    return usRight < COURT_GOAL_TO_BORDER;
  }
}

unsigned long Keeper::lastToggle() {
  return millis() - toggleTimer;
}

