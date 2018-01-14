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

int heading = 0;        //Wert des Kompass
int startHeading;   //Startwert des Kompass
int rotation;       //rotationswert für die Motoren
unsigned long turningTimer = 0;

Adafruit_NeoPixel matrix = Adafruit_NeoPixel(12, MATRIX_LED, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stateLed = Adafruit_NeoPixel(3, STATE_LED, NEO_GRB + NEO_KHZ800);

// Wichtungseinstellungen des PID-Reglers
double pidFilterP=0.32;  // p:proportional
double pidFilterI=0.04;  // i:vorausschauend
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

  Serial.println("setup done");
}


void loop(){
  m.setMotEn(!digitalRead(SWITCH_MOTOR));
  showState(0, stateFine);
  showState(1, battLow());
  showState(2, millis() % 1000 < 200);

  showMatrix(1, m.getMotEn());
  showMatrix(2, ballsicht);
  showMatrix(3, ballbesitz);

  ausrichten();

  delay(1);  
  
  d.clearDisplay();
  d.setTextSize(2);
  d.setTextColor(WHITE);
  
  d.setCursor(0,0);
  d.println("MotE: " + String(m.getMotEn()));
  d.println("Komp: " + String(heading));
  
  d.display();
  delay(1);
  
  matrix.show();
  stateLed.show();

  
  delay(1);

  debugln("loop");
  
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

void showMatrix(byte pos, boolean state){
  matrix.setPixelColor(pos, matrix.Color((!state)*PWR_LED,state*PWR_LED,0));
}

void showState(byte pos, boolean state){
  stateLed.setPixelColor(pos, stateLed.Color((!state)*PWR_LED,state*PWR_LED,0));
}


