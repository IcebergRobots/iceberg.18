#include "Config.h"

void pinModes(){
  pinMode(BUZZER, OUTPUT);
  pinMode(BIG_BUTTON, INPUT_PULLUP);

  pinMode(BATT_VOLTAGE, INPUT);
}
