#include "Config.h"
#include "Pin.h"
#include "Pilot.h"
#include "Utility.h"

int dire = 0;

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
