#include "Config.h"
#include "Pin.h"
#include "Pilot.h"
#include "Utility.h"

Pilot m;

void setup() {
  Serial.begin(9600);
  pinModes();
  motorConfig();
}

void loop(){
  m.steerMotor(1,30);
  if(battLow()){
    tone(9, 900);
  }else{
    noTone(9);
  }
}

void motorConfig(){
  m.setAngle(70);
  
  m.setPins(0, FWD0, BWD0, PWM0);
  m.setPins(1, FWD1, BWD1, PWM1);
  m.setPins(2, FWD2, BWD2, PWM2);
  m.setPins(3, FWD3, BWD3, PWM3);
}
