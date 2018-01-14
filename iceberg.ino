#include "Config.h"
#include "Pin.h"
#include "Pilot.h"
#include "Utility.h"

#include <Wire.h>
#include <HMC6352.h>

#include <PID_v1.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Adafruit_NeoPixel.h>

#define PWR 60      //maximale Motorstärke
#define ROT_MULTI 0.35
#define ROT_MAX 0.5 //der maximale Wert der Rotation

#define PWR_LED 30
  
Pilot m;            //Motorobjekt
HMC6352 c;          //Kompassobjekt
Adafruit_SSD1306 d(PIN_4); //Display-Objekt


boolean stateFine = true;
boolean ballsicht = false;
boolean ballbesitz = false;

int heading;        //Wert des Kompass
int startHeading;   //Startwert des Kompass
int rotation;       //rotationswert für die Motoren
unsigned long turningTimer = 0;

Adafruit_NeoPixel matrix = Adafruit_NeoPixel(12, MATRIX_LED, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stateLed = Adafruit_NeoPixel(3, STATE_LED, NEO_GRB + NEO_KHZ800);

// Wichtungseinstellungen des PID-Reglers
double pidFilterP=0.32;  // p:proportional
double pidFilterI=0.03;  // i:vorausschauend
double pidFilterD=0.03;  // d:Schwung herausnehmen (nicht zu weit drehen)
double pidSetpoint;  // Nulllevel [-180 bis 180]:Winkel des Tours
double pidIn;        // Kompasswert [-180 bis 180]
double pidOut;       // Rotationsstärke [-255 bis 255]
PID myPID(&pidIn, &pidOut, &pidSetpoint, pidFilterP, pidFilterI, pidFilterD, DIRECT);

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
  pidSetpoint = 0;
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(-255,255);

  matrix.begin(); // This initializes the NeoPixel library.
  stateLed.begin();
}


void loop(){
  stateLed.setPixelColor(0, stateLed.Color(PWR_LED*!stateFine,PWR_LED*stateFine,0));
  stateLed.setPixelColor(1, stateLed.Color(PWR_LED*battLow(),PWR_LED*!battLow(),0));
  
  if(millis() % 1000 < 200){
    stateLed.setPixelColor(2, stateLed.Color(0,PWR_LED,0));
  }else{
    stateLed.setPixelColor(2, stateLed.Color(0,0,0));
  }

  

  m.setMotEn(!digitalRead(SWITCH_MOTOR));
  matrix.setPixelColor(1, matrix.Color(!m.getMotEn()*PWR_LED, m.getMotEn()*PWR_LED,0));

  ausrichten();
  pidFilterI = analogRead(POTI)/10000.0;

  
  delay(1);  

  //fahre geradeaus (zum Tor)
  
  
  d.clearDisplay();
  d.setTextSize(2);
  d.setTextColor(WHITE);
  d.setCursor(0,0);
  d.println("Komp: "+ String(heading));
  d.println("MotE: "+ String(!digitalRead(SWITCH_MOTOR)));
  d.println("OUTP: "+ String(round(pidOut)));
  
  d.println("I: "+ String(pidFilterI));
  d.display();

  matrix.show();
  stateLed.show();
  
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

  pidIn = (double) heading;
  
  double gap = abs(pidSetpoint-pidIn); //distance away from setpoint
  myPID.SetTunings(pidFilterP, pidFilterI, pidFilterD);
  myPID.Compute();
  
  m.drive(0, 0, -pidOut);

}


