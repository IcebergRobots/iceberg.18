#include "Pin.h"
#include "Pilot.h"

Pilot m;
int dire = 0;

void setup() {
  Serial.begin(9600);
  pinMode(A5,INPUT_PULLUP);
  
  m.setAngle(70);
  
  m.setPins(0, FWD1, BWD1, PWM1);
  m.setPins(1, FWD2, BWD2, PWM2);
  m.setPins(2, FWD3, BWD3, PWM3);
  m.setPins(3, FWD4, BWD4, PWM4);

  
  while(digitalRead(A5)){
    
  }
  m.steerMotor(0,30);
  m.steerMotor(1,30);
  m.steerMotor(2,30);
  m.steerMotor(3,30);
  while(1){}
}

void loop(){
  m.drive(dire % 360, 50);
  if(!digitalRead(A5)){
    dire += 45;
    while(!digitalRead(A5)){;}
  }
  delay(10);
}
