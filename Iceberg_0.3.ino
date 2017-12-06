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
}

void loop(){
  m.drive(dire % 360, 50);
  dire ++;
  delay(10);
  Serial.println();
}
