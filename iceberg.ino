#include "Config.h"
#include "Pin.h"
#include "Pilot.h"
#include "Utility.h"

#include <SPI.h>
#include <Pixy.h>
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

Pilot m;            // Motorobjekt
HMC6352 c;          // Kompassobjekt
Adafruit_SSD1306 d(PIN_4); // Displayobjekt
Pixy pixy;          // Kameraobjekt

// Led-Matrix
boolean stateFine = true;
boolean canSeeBall = false;
boolean isOnTheBall = false;

int heading = 0;        //Wert des Kompass
int startHeading;   //Startwert des Kompass
int rotation;       //rotationswert für die Motoren
unsigned long turningTimer = 0;

Adafruit_NeoPixel matrix = Adafruit_NeoPixel(12, MATRIX_LED, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stateLed = Adafruit_NeoPixel(3, STATE_LED, NEO_GRB + NEO_KHZ800);

//Pixyeinstellungen
#define SPEED          100  //Geschwindigkeit des Roboters in %
#define PIXY_BALL_NUMMER  1 //Pixy-Signature des Balls
#define X_CENTER ((PIXY_MAX_X-PIXY_MIN_X)/2) //Die Mitte des Bildes der Pixy (in Pixeln)
uint16_t blocks;            //hier werden die erkannten Bloecke gespeichert
unsigned long lastPixy = 0; //Timer zum Auslesen der Pixy
byte blockAnzahl = 0;       //Anzahl der erkannten Bloecke
int highX;                  //Position des Balls (x-Koordinate)
int highY;                  //Position des Balls (y-Koordinate)
int xAbw = 0;               //die Abweichung des Balls von der Mitte des Pixybildes
boolean ballSicht;          //ob wir den Ball sehen

//Wichtungseinstellungen des PID-Reglers
double pidFilterP = 0.32; // p:proportional
double pidFilterI = 0.03; // i:vorausschauend
double pidFilterD = 0.03; // d:Schwung herausnehmen (nicht zu weit drehen)
double pidSetpoint;  // Nulllevel [-180 bis 180]:Winkel des Tours
double pidIn;        // Kompasswert [-180 bis 180]
double pidOut;       // Rotationsstärke [-255 bis 255]
PID myPID(&pidIn, &pidOut, &pidSetpoint, pidFilterP, pidFilterI, pidFilterD, DIRECT);

void setup() {
  //startSound();     // Fiepen, welches Programstart signalisiert

  //Initialisierungen
  Serial.begin(9600);   // Start der Seriellen Kommunikation
  Wire.begin();         // Start der I2C-Kommunikation

  setupDisplay();       // initialisiere Display mit Iceberg Schriftzug
  pinModes();           // setzt die PinModes
  setupMotor();         // setzt Pins und Winkel des Pilot Objekts
  pixy.init();          // initialisiere Kamera

  delay(1000);

  //Torrichtung [-180 bis 179] merken
  startHeading = c.getHeading() - 180; //merkt sich Startwert des Kompass

  m.setMotEn(true);

  if (!digitalRead(SWITCH_A)) {
    m.drive(0, 0, 8);
    c.calibration();
  }
  m.brake(true);
  c.setOutputMode(0);   //Kompass initialisieren
  pidSetpoint = 0;
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(-255, 255);

  matrix.begin(); // This initializes the NeoPixel library.
  stateLed.begin();

  Serial.println("setup done");
}


void loop() {
  m.setMotEn(!digitalRead(SWITCH_MOTOR));
  
  showState(0, stateFine);
  showState(1, battLow());
  showState(2, millis() % 1000 < 200);

  showMatrix(1, m.getMotEn());
  showMatrix(2, ballsicht);
  showMatrix(3, ballbesitz);


  //wenn 25ms seit derm letzten Auslesen vergangen sind, wird die Pixy erneut ausgelesen
  if (millis() - lastPixy > 25) {
    readPixy();
  }

  ausrichten();

  delay(1);

  d.clearDisplay();
  d.setTextSize(2);
  d.setTextColor(WHITE);
  
  d.setCursor(0,0);
  d.println("MotE: " + String(m.getMotEn()));
  d.println("Komp: " + String(heading));
  delay(1);
  
  matrix.show();
  stateLed.show();
  
  delay(1);
  
}

void setupMotor() {
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
  d.setCursor(0, 0);    //positioniert Cursor
  d.println("ICEBERG ROBOTS");  //schreibt Text auf das Display
  d.display();          //wendet Aenderungen an
}

void ausrichten() {
  heading = ((int)((c.getHeading()/*[0 bis 359]*/ - startHeading/*[-359 bis 359]*/) + 360) % 360)/*[0 bis 359]*/ - 180/*[-180 bis 179]*/; //Misst die Kompassabweichung vom Tor

  pidIn = (double) heading;

  double gap = abs(pidSetpoint - pidIn); //distance away from setpoint
  myPID.SetTunings(pidFilterP, pidFilterI, pidFilterD);
  myPID.Compute();

  m.drive(0, 0, -pidOut);

}

void showBool(Adafruit_NeoPixel nMatrix, byte pos, boolean state){
  nMatrix.setPixelColor(1, nMatrix.Color(!state*PWR_LED,state*PWR_LED,0));
}

//Methode zum Auslesen der Pixy; Diese Methode sucht nach dem groesten Block in der Farbe des Balls
void readPixy() {
  int greatestBlock = 0; //hier wird die Groeße des groeßten Blocks gespeichert
  highX = 0;             //Position des Balls (X)
  highY = 0;             //Position des Balls (Y)
  blockAnzahl = 0;       //Anzahl der Bloecke

  blocks = pixy.getBlocks();  //lässt sich die Bloecke ausgeben

  for (int j = 0; j < blocks; j++) {                                  //geht alle erkannten Bloecke durch
    if (pixy.blocks[j].signature == PIXY_BALL_NUMMER) {               //Überprueft, ob es sich bei dem Block um den Ball handelt
      if (pixy.blocks[j].height * pixy.blocks[j].width > greatestBlock) { //Wenn der Block der aktuell groesste ist
        greatestBlock = pixy.blocks[j].height * pixy.blocks[j].width;
        highX = pixy.blocks[j].x;                                     //Position des Blocks wird uebernommen
        highY = pixy.blocks[j].y;
      }
      blockAnzahl++;        //Anzahl wird hochgezaehlt
    }
  }

  lastPixy = millis();         //Timer wird gesetzt, da Pixy nur alle 25ms ausgelesen werden darf

  ballSicht = blockAnzahl != 0; //wenn Bloecke in der Farbe des Balls erkannt wurden, dann sehen wir den Ball
}

void showMatrix(byte pos, boolean state){
  matrix.setPixelColor(pos, matrix.Color((!state)*PWR_LED,state*PWR_LED,0));
}
