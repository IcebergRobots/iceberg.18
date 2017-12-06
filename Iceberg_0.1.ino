#include "pin.h"
#include "Pilot.h"

Pilot motors;
int dire = 0;

void setup() {
  Serial.begin(9600);
  
  motors.setAngle(70);
  
  motors.setPins(0, FWD1, BWD1, PWM1);
  motors.setPins(1, FWD2, BWD2, PWM2);
  motors.setPins(2, FWD3, BWD3, PWM3);
  motors.setPins(3, FWD4, BWD4, PWM4);
}

void loop() {
  Serial.println("Direction: "+(String) dire+"°");
  motors.drive(dire, 255);
  Serial.println();
  dire += 5;
  dire %= 360;
  delay(1000);
} 
