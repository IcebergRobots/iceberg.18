#include "Config.h"

void pinModes(){
  pinMode(BUZZER, OUTPUT);
  pinMode(BIG_BUTTON, INPUT_PULLUP);

  pinMode(BATT_VOLTAGE, INPUT);
}

void motorConfig(){
  m.setAngle(70);
  
  m.setPins(0, FWD0, BWD0, PWM0);
  m.setPins(1, FWD1, BWD1, PWM1);
  m.setPins(2, FWD2, BWD2, PWM2);
  m.setPins(3, FWD3, BWD3, PWM3);
}

