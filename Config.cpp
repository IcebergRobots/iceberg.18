#include "Config.h"

void pinModes(){
  pinMode(BUZZER, OUTPUT);
  pinMode(BIG_BUTTON, INPUT_PULLUP);

  pinMode(BATT_VOLTAGE, INPUT);

  pinMode(SWITCH_MOTOR, INPUT_PULLUP);
}
