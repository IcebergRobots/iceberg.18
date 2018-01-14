#include "Config.h"
#include "Pin.h"
#include "Pilot.h"
#include "Utility.h"

#include <Wire.h>
#include <HMC6352.h>

#include <PID_v1.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define PWR 60      //maximale Motorstärke
#define ROT_MULTI 0.35
#define ROT_MAX 0.5 //der maximale Wert der Rotation
  
Pilot m;            //Motorobjekt
HMC6352 c;          //Kompassobjekt
Adafruit_SSD1306 d(PIN_4); //Display-Objekt


int heading;        //Wert des Kompass
int startHeading;   //Startwert des Kompass
int rotation;       //rotationswert für die Motoren
unsigned long turningTimer = 0;

double Setpoint, Input, Output;

double consKp=0.32, consKi=0.03, consKd=0.03;

PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, DIRECT);


void setup() {
  //startSound();     // Fiepen, welches Programstart signalisiert

  //Initialisierungen
  Serial.begin(9600);   //Start der Seriellen Kommunikation
  Wire.begin();         //Start der I2C-Kommunikation

  setupDisplay();       //initialisiere Display mit Iceberg Schriftzug
  pinModes();           //setzt die PinModes
  setupMotor();         //setzt Pins und Winkel des Pilot Objekts

  delay(1000);

  //Torrichtung [-180 bis 179] merken
  startHeading = c.getHeading()-180;  //merkt sich Startwert des Kompass

  m.setMotEn(true);

  if(!digitalRead(SWITCH_A)){
    m.drive(0,0,8);
    c.calibration();
  }
  m.brake(true);
  c.setOutputMode(0);   //Kompass initialisieren
  Setpoint = 0;
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(-255,255);
}


void loop(){
  //Batteriestand prüfen
  /*if(battLow()){
    tone(BUZZER, 900);
  }else{
    noTone(BUZZER);
  }*/

  m.setMotEn(!digitalRead(SWITCH_MOTOR));

  ausrichten();
  consKi = analogRead(POTI)/10000.0;

  
  delay(1);  

  //fahre geradeaus (zum Tor)
  
  
  d.clearDisplay();
  d.setTextSize(2);
  d.setTextColor(WHITE);
  d.setCursor(0,0);
  d.println("Komp: "+ String(heading));
  d.println("MotE: "+ String(!digitalRead(SWITCH_MOTOR)));
  d.println("OUTP: "+ String(round(Output)));
  
  d.println("I: "+ String(consKi));
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
  d.println("ICEBERG ROBOTS");  //schreibt Text auf das Display
  d.display();          //wendet Aenderungen an
}

void ausrichten() {
  heading = ((int)((c.getHeading()/*[0 bis 359]*/-startHeading/*[-359 bis 359]*/)+360) % 360)/*[0 bis 359]*/-180/*[-180 bis 179]*/; //Misst die Kompassabweichung vom Tor

  Input = (double) heading;
  
  double gap = abs(Setpoint-Input); //distance away from setpoint
  myPID.SetTunings(consKp, consKi, consKd);
  myPID.Compute();
  
  m.drive(0, 0, -Output);

}

