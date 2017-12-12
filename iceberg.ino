#include "Config.h"
#include "Pin.h"
#include "Pilot.h"
#include "Utility.h"

#include <Wire.h>
#include "HMC6352.h"

#define PWR 50

Pilot m;
HMC6352 myCompass;

float heading;

void setup() {
  for(int i = 10; i<2000; i += 10){
    tone(BUZZER, i);
    delay(1);
  }
  
  Serial.begin(9600);
  Wire.begin();
  pinModes();
  motorConfig();
  myCompass.setOutputMode(0);
}

void loop(){
  heading = map(myCompass.getHeading()-180,-180,180,-PWR,PWR);
  m.drive(0, PWR-abs(heading),heading);
}

void motorConfig(){
  m.setAngle(70);
  
  m.setPins(0, FWD0, BWD0, PWM0);
  m.setPins(1, FWD1, BWD1, PWM1);
  m.setPins(2, FWD2, BWD2, PWM2);
  m.setPins(3, FWD3, BWD3, PWM3);
}
