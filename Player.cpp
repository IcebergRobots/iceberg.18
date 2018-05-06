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
