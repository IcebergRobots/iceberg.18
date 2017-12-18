#include "Config.h"
#include "Pin.h"
#include "Pilot.h"
#include "Utility.h"

#include <Wire.h>
#include "HMC6352.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define PWR 255     //maximale Motorstärke
  
Pilot m;            //Motorobjekt
HMC6352 c;          //Kompassobjekt
Adafruit_SSD1306 d(PIN_4); //Display-Objekt

int heading;        //Wert des Kompass
int startHeading;   //Startwert des Kompass
int rotation;       //rotationswert für die Motoren
unsigned long turningTimer = 0;

void setup() {
  startSound();     // Fiepen, welches Programstart signalisiert

  //Initialisierungen
  Serial.begin(9600);   //Start der Seriellen Kommunikation
  Wire.begin();         //Start der I2C-Kommunikation
  pinModes();           //setzt die PinModes
  setupMotor();         //setzt Pins und Winkel des Pilot Objekts
  c.setOutputMode(0);   //Kompass initialisieren
  setupDisplay();       //initialisiere Display mit Iceberg Schriftzug

  //Torrichtung [-180 bis 179] merken
  startHeading = c.getHeading()-180;  //merkt sich Startwert des Kompass
}

void loop(){
  //Batteriestand prüfen
  if(battLow()){
    tone(BUZZER, 900);
  }else{
    noTone(BUZZER);
  }

  //Winkel [-180 bis 179] zum Tor berechnen
  heading = ((int)((c.getHeading()/*[0 bis 359]*/-startHeading/*[-180 bis 180]*/)+360) % 360)/*[0 bis 359]*/-180; //Misst die Kompassabweichung vom Tor
  
  //Rotationsstärke [-PWR bis PWR] für Torausrichtung berechnen
  rotation = max(min(map(heading,-220,220,-PWR,PWR),PWR),-PWR);   //Je größer der Torwinkel, desto groeßer die Rotation
  
  delay(1);  

  //fahre geradeaus (zum Tor)
  m.drive(0, PWR-abs(rotation),rotation);
  
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

void setupMotor(){
  m.setAngle(70);
  
  m.setPins(0, FWD0, BWD0, PWM0);
  m.setPins(1, FWD1, BWD1, PWM1);
  m.setPins(2, FWD2, BWD2, PWM2);
  m.setPins(3, FWD3, BWD3, PWM3);
}

void setupDisplay() {
  d.begin(SSD1306_SWITCHCAPVCC, 0x3C);  //Initialisieren des Displays
  d.clearDisplay();     //leert das Display
  d.setTextSize(2);     //setzt Textgroesse
  d.setTextColor(WHITE);//setzt Textfarbe
  d.setCursor(0,0);     //positioniert Cursor
  d.println("Iceberg Robots");  //schreibt Text auf das Display
  d.display();          //wendet Aenderungen an
}

