#include "Pilot.h"

// array auslesen ist schneller als berechnen
const int sinus[360] = {0,175,349,523,698,872,1045,1219,1392,1564,1736,1908,2079,2250,2419,2588,2756,2924,3090,3256,3420,3584,3746,3907,4067,4226,4384,4540,4695,4848,5000,5150,5299,5446,5592,5736,5878,6018,6157,6293,6428,6561,6691,6820,6947,7071,7193,7314,7431,7547,7660,7771,7880,7986,8090,8192,8290,8387,8480,8572,8660,8746,8829,8910,8988,9063,9135,9205,9272,9336,9397,9455,9511,9563,9613,9659,9703,9744,9781,9816,9848,9877,9903,9925,9945,9962,9976,9986,9994,9998,10000,9998,9994,9986,9976,9962,9945,9925,9903,9877,9848,9816,9781,9744,9703,9659,9613,9563,9511,9455,9397,9336,9272,9205,9135,9063,8988,8910,8829,8746,8660,8572,8480,8387,8290,8192,8090,7986,7880,7771,7660,7547,7431,7314,7193,7071,6947,6820,6691,6561,6428,6293,6157,6018,5878,5736,5592,5446,5299,5150,5000,4848,4695,4540,4384,4226,4067,3907,3746,3584,3420,3256,3090,2924,2756,2588,2419,2250,2079,1908,1736,1564,1392,1219,1045,872,698,523,349,175,0,-175,-349,-523,-698,-872,-1045,-1219,-1392,-1564,-1736,-1908,-2079,-2250,-2419,-2588,-2756,-2924,-3090,-3256,-3420,-3584,-3746,-3907,-4067,-4226,-4384,-4540,-4695,-4848,-5000,-5150,-5299,-5446,-5592,-5736,-5878,-6018,-6157,-6293,-6428,-6561,-6691,-6820,-6947,-7071,-7193,-7314,-7431,-7547,-7660,-7771,-7880,-7986,-8090,-8192,-8290,-8387,-8480,-8572,-8660,-8746,-8829,-8910,-8988,-9063,-9135,-9205,-9272,-9336,-9397,-9455,-9511,-9563,-9613,-9659,-9703,-9744,-9781,-9816,-9848,-9877,-9903,-9925,-9945,-9962,-9976,-9986,-9994,-9998,-10000,-9998,-9994,-9986,-9976,-9962,-9945,-9925,-9903,-9877,-9848,-9816,-9781,-9744,-9703,-9659,-9613,-9563,-9511,-9455,-9397,-9336,-9272,-9205,-9135,-9063,-8988,-8910,-8829,-8746,-8660,-8572,-8480,-8387,-8290,-8192,-8090,-7986,-7880,-7771,-7660,-7547,-7431,-7314,-7193,-7071,-6947,-6820,-6691,-6561,-6428,-6293,-6157,-6018,-5878,-5736,-5592,-5446,-5299,-5150,-5000,-4848,-4695,-4540,-4384,-4226,-4067,-3907,-3746,-3584,-3420,-3256,-3090,-2924,-2756,-2588,-2419,-2250,-2079,-1908,-1736,-1564,-1392,-1219,-1045,-872,-698,-523,-349,-175};

/*****************************************************
setze Achsenwinkel auf 70°
*****************************************************/
Pilot::Pilot(){
  _angle = 70;
}

/*****************************************************
setze Achsenwinkel
*****************************************************/
Pilot::Pilot(byte angle){
  _angle = angle;
}

/*****************************************************
setze Motor-Ansteuerungspins
*****************************************************/
void Pilot::setPins(byte id, byte fwd, byte bwd, byte pwm) {
  if(id < 0 || id > 3) {  // ungueltige Eingabe
    return;
  }
  
  _fwd[id] = fwd;         // speichere Pins
  _bwd[id] = bwd;
  _pwm[id] = pwm;
  
  pinMode(fwd, OUTPUT);   // definiere Pins als Output
  pinMode(bwd, OUTPUT);
  pinMode(pwm, OUTPUT);
}

/*****************************************************
setze den Winkel zwischen zwei Motoren einer Seite (Achsenwinkel)
*****************************************************/
void Pilot::setAngle(byte angle) {
  _angle = angle%180;
}

/*****************************************************
setze Ausgangssignale fuer einen Motor
*****************************************************/
void Pilot::steerMotor(byte id, int power) {
  
  if(id < 0 || id > 3) {      // ungueltige Eingabe
    return;
  }

  power = min(255,power);
  power = max(-255,power);

  digitalWrite(_fwd[id], power>0);
  digitalWrite(_bwd[id], power<=0);
  analogWrite(_pwm[id], abs(power));
  
  Serial.println((String)id + " -> "+(String)power);
}

/*****************************************************
steuere die Motoren an, um zu fahren
- keine Berechnungen
*****************************************************/
void Pilot::drive() {
  drive(_values);
}

/*****************************************************
steuere die Motoren an, um zu fahren
- nutze Berechnungen des Zwischenspeichers
*****************************************************/
void Pilot::drive(int values[]) {
  for(int i=0; i<4; i++) {
    steerMotor(i, values[i]);
  }
}

/*****************************************************
fahre mit Geschwindigkeit und Zielwinkel
*****************************************************/
void Pilot::drive(int angle, int power) {
  calculate(angle, power);
  drive();
}

/*****************************************************
fahre mit Geschwindigkeit, Zielwinkel und Eigenrotation
- Geschwindigkeit [0 bis 255]
- Zielwinkel [-180 bis 180]:
      0  
    .--.
90 /    \ -90
   \    /
- Eigenrotation [-255 bis 255]
-> Korrekturdrehung, um wieder zum Gegnertor ausgerichtet zu sein
*****************************************************/
 void Pilot::drive(int angle, int power, int rotation) {
  calculate(angle, power, rotation);
  drive();
}

/*****************************************************
berechne Zwischenspeicher für Motoransteuerung
*****************************************************/
void Pilot::calculate(int angle, int power) {
  calculate(angle, power, 0);
}

/*****************************************************
berechne Zwischenspeicher für Motoransteuerung
*****************************************************/
void Pilot::calculate(int angle, int power, int rotation) {
  if(power<0) {
    power = -power;                       // power absolut machen
    angle += 180;                         // hinten sind 180°
  }

  while(angle<0) {                        // zu kleine Eingabe korrigieren
    angle += 360;
  }
  angle %= 360;                           // zu große Eingabe korrigieren

  if(power + abs(rotation) > 255) {       // zu große Gesamtgeschwindigkeit korrigieren
    power -= (power + abs(rotation))-255;
  }
  
  int sinA13 = sinus[(((_angle/2)-angle)+360)%360]; // Achse für Motoren 1 und 3
  int sinA24 = sinus[(((_angle/2)+angle)+360)%360]; // Achse für Motoren 2 und 4
  
  int axis13 = power * (double)sinA13/10000;
  int axis24 = power * (double)sinA24/10000;

  
  _values[0] = /*Motor 1*/ axis13 - rotation;       // speichere die Werte
  _values[1] = /*Motor 2*/ axis24 - rotation;
  _values[2] = /*Motor 3*/ axis13 + rotation;
  _values[3] = /*Motor 4*/ axis24 + rotation;
}

/*****************************************************
bremse aktiv oder passiv alle Motoren
*****************************************************/
void Pilot::brake(bool activ) {
  for(byte i = 0; i<4; i++){
    digitalWrite(_fwd[i], activ);
    digitalWrite(_bwd[i], activ);
    analogWrite(_pwm[i], 255);
  }
}
