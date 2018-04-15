#include "Keeper.h"

Keeper::Keeper() {
}

void Keeper::set() {
  drivePwr = SPEED_KEEPER;
  driveState = "keeper";
  if (movingLeft) {
    driveDir = ANGLE_SIDEWAY;
  } else {
    driveDir = -ANGLE_SIDEWAY;
  }
  if (us[3] < 15) driveDir *= 0.8;
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
  byte usLeft = us[2];
  byte usRight = us[0];
  if (movingLeft) {
    if (usLeft == 0)  usLeft = COURT_WIDTH - usRight;
    return usLeft < COURT_GOAL_TO_BORDER;
  } else {
    if (usRight == 0) usRight = COURT_WIDTH - usLeft;
    return usRight < COURT_GOAL_TO_BORDER;
  }
}

unsigned long Keeper::lastToggle() {
  return millis() - toggleTimer;
}

