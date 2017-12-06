#include "Pin.h"
#include "Pilot.h"

Pilot m;
int dire = 0;

void setup() {
  Serial.begin(9600);
  
  m.setAngle(70);
  
  m.setPins(0, FWD1, BWD1, PWM1);
  m.setPins(1, FWD2, BWD2, PWM2);
  m.setPins(2, FWD3, BWD3, PWM3);
  m.setPins(3, FWD4, BWD4, PWM4);
}

void loop() {
  for(int i = -255; i< 255; i++) {
    m.drive(0,i);
    delay(10);
  }
  for(int i = 255; i> -255; i--) {
    m.drive(0,i);
    delay(10);
  }
}
