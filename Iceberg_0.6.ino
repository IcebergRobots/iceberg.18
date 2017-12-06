#include "Pin.h"
#include "Pilot.h"

Pilot m;
int dire = 0;

boolean battLow(){
  return (analogRead(BATT_VOLTAGE) * 0.01220703)>4 && (analogRead(BATT_VOLTAGE) * 0.01220703)<10.8;
}

void pinModes(){
  pinMode(BUZZER, OUTPUT);
  pinMode(BIG_BUTTON, INPUT_PULLUP);

  pinMode(BATT_VOLTAGE, INPUT);
}

void motorConfig(){
  m.setAngle(70);
  
  m.setPins(0, FWD1, BWD1, PWM1);
  m.setPins(1, FWD2, BWD2, PWM2);
  m.setPins(2, FWD3, BWD3, PWM3);
  m.setPins(3, FWD4, BWD4, PWM4);
}

void setup() {
  Serial.begin(9600);
  pinModes();
  motorConfig();
}

void loop(){
  m.steerMotor(0,30);
  m.steerMotor(1,30);
  m.steerMotor(2,30);
  m.steerMotor(3,30);
  
  if(battLow()){
    tone(9, 900);
  }else{
    noTone(9);
  }
}
