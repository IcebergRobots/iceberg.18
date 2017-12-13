#include "Config.h"
#include "Pin.h"
#include "Pilot.h"
#include "Utility.h"

#include <Wire.h>
#include "HMC6352.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define PWR 255      //
  
Pilot m;            //Motorobjekt
HMC6352 c;          //Kompassobjekt
Adafruit_SSD1306 d(PIN_4); //Display-Objekt

int heading;        //Wert des Kompass
int startHeading;   //Startwert des Kompass
int rotation;       //rotationswert für die Motoren
unsigned long turningTimer = 0;

void setup() {
  //Start-fiep
  for(int i = 10; i<2000; i += 10){
    tone(BUZZER, i);
    delay(1);
  }
  noTone(BUZZER);
  
  Serial.begin(9600);   //Start der Seriellen Kommunikation
  Wire.begin();         //Start der I2C-Kommunikation
  d.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  pinModes();           //setzt die PinModes
  motorConfig();        //konfiguriert das Pilot-Objekt
  c.setOutputMode(0);   //keine Ahnung

  d.clearDisplay();     //leert das Display
  d.setTextSize(2);     //setzt Textgroesse
  d.setTextColor(WHITE);//setzt Textfarbe
  d.setCursor(0,0);     //positioniert Cursor
  d.println("Iceberg Robots");  //schreibt Text auf das Display
  d.display();          //wendet Aenderungen an

  startHeading = c.getHeading()-180;  //merkt sich den Startwert des Kompass
}

void loop(){
  if(battLow()){
    tone(BUZZER, 900);
  }else{
    noTone(BUZZER);
  }

  
  heading = ((int)((c.getHeading()-startHeading)+360) % 360)-180;   //Misst die Kompassabweichung vom Startwert
  rotation = max(min(map(heading,-220,220,-PWR,PWR),PWR),-PWR);   //Berechnet die Rotation zu Kompasskorrektur
  
  delay(1);  
  
  m.drive(0, PWR-abs(rotation),rotation);                           //steuert die Motoren an
  
  d.clearDisplay();
  d.setTextSize(2);
  d.setTextColor(WHITE);
  d.setCursor(0,0);
  d.println("Iceberg");
  d.println("Robots");
  d.setTextSize(4);
  d.println(rotation);
  d.display();
  
  delay(1);
  
}

void motorConfig(){
  m.setAngle(70);
  
  m.setPins(0, FWD0, BWD0, PWM0);
  m.setPins(1, FWD1, BWD1, PWM1);
  m.setPins(2, FWD2, BWD2, PWM2);
  m.setPins(3, FWD3, BWD3, PWM3);
}
