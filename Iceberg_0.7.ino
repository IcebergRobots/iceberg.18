#include "Pin.h"
#include "Pilot.h"

Pilot m;
int dire = 0;
unsigned long lastButton=0;

boolean battLow(){
  return (analogRead(BATT_VOLTAGE) * 0.01220703)>4 && (analogRead(BATT_VOLTAGE) * 0.01220703)<10.8;
}

void pinModes(){
  pinMode(KICK, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BIG_BUTTON, INPUT_PULLUP);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_KICK, INPUT_PULLUP);

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
  if(battLow()){
    tone(9, 900);
  }else{
    noTone(9);
  }
  if(!digitalRead(BIG_BUTTON)){
    for(int i=0;i<4;i++){
      m.steerMotor(i,255);
    }
    delay(2000);
    for(int i=0;i<4;i++){
      m.brake(1);
    }
  }
  if(!digitalRead(BUTTON_1)){
    delay(100);
    m.steerMotor(0,255);
    m.steerMotor(1,255);
    m.steerMotor(2,-255);
    m.steerMotor(3,-255);
    delay(3000);
    for(int i=0;i<4;i++){
      m.brake(1);
    }
  }
  if(!digitalRead(BUTTON_KICK)){
      digitalWrite(KICK,1);
      delay(40);
      digitalWrite(KICK,0);
      delay(1000);
  } 
}
