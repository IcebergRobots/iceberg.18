#include "Config.h"
#include "Pin.h"
#include "Pilot.h"
#include "Utility.h"

#include <Wire.h>
#include "HMC6352.h"

#define PWR 50
  
Pilot m;            //Motorobjekt
HMC6352 c;  //Kompassobjekt

int heading;        //Wert des Kompass
int startHeading;   //Startwert des Kompass
int rotation;       //rotationswert für die Motoren

void setup() {
  //Start-fiep
  for(int i = 10; i<2000; i += 10){
    tone(BUZZER, i);
    delay(1);
  }
  
  Serial.begin(9600);   //Start der Seriellen Kommunikation
  Wire.begin();         //Start der I2C-Kommunikation
  
  pinModes();           //setzt die PinModes
  motorConfig();        //konfiguriert das Pilot-Objekt
  c.setOutputMode(0);   //keine Ahnung

  startHeading = c.getHeading();  //merkt sich den Startwert des Kompass
}

void loop(){
  heading = ((int)((c.getHeading()-startHeading)+360) % 360)-180;   //Misst die Kompassabweichung vom Startwert
  rotation = map(heading,-180,180,-PWR,PWR);                        //Berechnet die Rotation zu Kompasskorrektur
  
  m.drive(0, PWR-abs(rotation),rotation);                           //steuert die Motoren an
}

void motorConfig(){
  m.setAngle(70);
  
  m.setPins(0, FWD0, BWD0, PWM0);
  m.setPins(1, FWD1, BWD1, PWM1);
  m.setPins(2, FWD2, BWD2, PWM2);
  m.setPins(3, FWD3, BWD3, PWM3);
}
