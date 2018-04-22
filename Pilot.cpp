#include "Pilot.h"

// Implementierung: OBJEKTE
extern Display d;
extern Keeper keeper;
extern Led led;
extern Mate mate;
extern Pilot m;
extern Ultrasonic us;

// array auslesen ist schneller als berechnen
const int sinus[360] = {0, 175, 349, 523, 698, 872, 1045, 1219, 1392, 1564, 1736, 1908, 2079, 2250, 2419, 2588, 2756, 2924, 3090, 3256, 3420, 3584, 3746, 3907, 4067, 4226, 4384, 4540, 4695, 4848, 5000, 5150, 5299, 5446, 5592, 5736, 5878, 6018, 6157, 6293, 6428, 6561, 6691, 6820, 6947, 7071, 7193, 7314, 7431, 7547, 7660, 7771, 7880, 7986, 8090, 8192, 8290, 8387, 8480, 8572, 8660, 8746, 8829, 8910, 8988, 9063, 9135, 9205, 9272, 9336, 9397, 9455, 9511, 9563, 9613, 9659, 9703, 9744, 9781, 9816, 9848, 9877, 9903, 9925, 9945, 9962, 9976, 9986, 9994, 9998, 10000, 9998, 9994, 9986, 9976, 9962, 9945, 9925, 9903, 9877, 9848, 9816, 9781, 9744, 9703, 9659, 9613, 9563, 9511, 9455, 9397, 9336, 9272, 9205, 9135, 9063, 8988, 8910, 8829, 8746, 8660, 8572, 8480, 8387, 8290, 8192, 8090, 7986, 7880, 7771, 7660, 7547, 7431, 7314, 7193, 7071, 6947, 6820, 6691, 6561, 6428, 6293, 6157, 6018, 5878, 5736, 5592, 5446, 5299, 5150, 5000, 4848, 4695, 4540, 4384, 4226, 4067, 3907, 3746, 3584, 3420, 3256, 3090, 2924, 2756, 2588, 2419, 2250, 2079, 1908, 1736, 1564, 1392, 1219, 1045, 872, 698, 523, 349, 175, 0, -175, -349, -523, -698, -872, -1045, -1219, -1392, -1564, -1736, -1908, -2079, -2250, -2419, -2588, -2756, -2924, -3090, -3256, -3420, -3584, -3746, -3907, -4067, -4226, -4384, -4540, -4695, -4848, -5000, -5150, -5299, -5446, -5592, -5736, -5878, -6018, -6157, -6293, -6428, -6561, -6691, -6820, -6947, -7071, -7193, -7314, -7431, -7547, -7660, -7771, -7880, -7986, -8090, -8192, -8290, -8387, -8480, -8572, -8660, -8746, -8829, -8910, -8988, -9063, -9135, -9205, -9272, -9336, -9397, -9455, -9511, -9563, -9613, -9659, -9703, -9744, -9781, -9816, -9848, -9877, -9903, -9925, -9945, -9962, -9976, -9986, -9994, -9998, -10000, -9998, -9994, -9986, -9976, -9962, -9945, -9925, -9903, -9877, -9848, -9816, -9781, -9744, -9703, -9659, -9613, -9563, -9511, -9455, -9397, -9336, -9272, -9205, -9135, -9063, -8988, -8910, -8829, -8746, -8660, -8572, -8480, -8387, -8290, -8192, -8090, -7986, -7880, -7771, -7660, -7547, -7431, -7314, -7193, -7071, -6947, -6820, -6691, -6561, -6428, -6293, -6157, -6018, -5878, -5736, -5592, -5446, -5299, -5150, -5000, -4848, -4695, -4540, -4384, -4226, -4067, -3907, -3746, -3584, -3420, -3256, -3090, -2924, -2756, -2588, -2419, -2250, -2079, -1908, -1736, -1564, -1392, -1219, -1045, -872, -698, -523, -349, -175};

/*****************************************************
  setze Achsenwinkel auf 70°
*****************************************************/
Pilot::Pilot() {
  _angle = 70;
  _motEn = false;
}

/*****************************************************
  setze Achsenwinkel
  @param angle: Achsenwinkel
*****************************************************/
Pilot::Pilot(byte angle) {
  _angle = angle;
  _motEn = false;
}

/*****************************************************
  setze Motor-Ansteuerungspins
  @param id: Motor-ID
  @param fwd: Pin für Vorwärtsdrehung
  @param bwd: Pin für Rückwärtsdrehung
  @param pwm: Pin für Geschwindigkeit
*****************************************************/
void Pilot::setPins(byte id, byte fwd, byte bwd, byte pwm) {
  if (id < 0 || id > 3) { // ungueltige Eingabe
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
  @param angle: Achsenwinkel
*****************************************************/
void Pilot::setAngle(byte angle) {
  _angle = angle % 180;
}

/*****************************************************
  setze Ausgangssignale fuer einen Motor
  @param id [0 bis 3]: Motor-ID
  @param power [-255 bis 255]: Gescwindigkeit

  IDs:
     .--.
  0 /    \ 3
  1 \    / 2
     '--'
*****************************************************/
void Pilot::steerMotor(byte id, int power) {
  if (_motEn) {
    if (id < 0 || id > 3) {     //Eingabeueberpruefung
      return;
    }

    power = min(255, power);    //Eingabekorrektur
    power = max(-255, power);

    digitalWrite(_fwd[id], power > 0);  //drehe Motor vorwarts
    digitalWrite(_bwd[id], power <= 0); //drehe Motor rueckwaerts
    analogWrite(_pwm[id], abs(power));  //drehe Motor mit Geschwindigkeit
  }
}

/*****************************************************
  fahre mit Geschwindigkeit, Zielwinkel und Eigenrotation
  @param (optional) angle [180 bis 180]: Zielwinkel
  @param (optional) power [-255 bis 255]: Geschwindigkeit
  @param (optional) rotation [-255 bis 255]: Eigenrotation -> Korrekturdrehung, um wieder zum Gegnertor ausgerichtet zu sein

  Winkel:0
      .--.
  90 /    \ -90
     \    /
      '--'
*****************************************************/
void Pilot::drive(int angle, int power, int rotation) {
  calculate(angle, power, rotation);
  drive();
}
void Pilot::drive(int angle, int power) {
  calculate(angle, power);
  drive();
}
void Pilot::drive() {
  drive(_values);
}

/*****************************************************
  steuere die Motoren an, um zu fahren
  @param values: Zwischenspeicher
  - nutze Berechnungen des Zwischenspeichers
*****************************************************/
void Pilot::drive(int values[]) {
  for (int i = 0; i < 4; i++) {
    steerMotor(i, values[i]);
  }
}


/*****************************************************
  berechne Zwischenspeicher für Motoransteuerung
  @param angle [180 bis 180]: Zielwinkel
  @param power [-255 bis 255]: Geschwindigkeit
  @param (optional) rotation [-255 bis 255]: Eigenrotation -> Korrekturdrehung, um wieder zum Gegnertor ausgerichtet zu sein
*****************************************************/
void Pilot::calculate(int angle, int power, int rotation) {
  if (power < 0) {      //bei negativen Geschwindigkeiten,
    power = -power;     //positive Geschwindigkeit
    angle += 180;       //bei 180° Drehung verwenden
  }

  while (angle < 0) {   //Eingabekorrektur
    angle += 360;       //
  }                     //
  angle %= 360;         //

  if (power + abs(rotation) > 255) {        //Wenn die Gesamtgeschwindigkeit zu groß ist,
    power -= (power + abs(rotation)) - 255; //wird die Geschwindigkeit ausreichend reduziert
  }

  //                                                      IDs:  .--.
  int sinA02 = sinus[(((_angle / 2) - angle) + 360) % 360]; //berechne Zwischenwert für Achse der Motoren 1 und 3      3 /    \ 0
  int sinA13 = sinus[(((_angle / 2) + angle) + 360) % 360]; //berechne Zwischenwert für Achse der Motoren 2 und 4      2 \    / 1
  //                                                            '--'
  int axis02 = power * (double)sinA02 / 10000; //berechne Motorstärken für Achse 1&3
  int axis13 = power * (double)sinA13 / 10000; //berechne Motorstärken für Achse 2&4


  _values[0] = axis02 - rotation;       //erstelle Zwischenspeicher für alle Motorstärken
  _values[1] = axis13 - rotation;
  _values[2] = axis02 + rotation;
  _values[3] = axis13 + rotation;
}
void Pilot::calculate(int angle, int power) {
  calculate(angle, power, 0);
}



/*****************************************************
  bremse aktiv oder passiv alle Motoren
  @param activ: aktives Bremsen?
*****************************************************/
void Pilot::brake(bool activ) {
  for (byte i = 0; i < 4; i++) {
    digitalWrite(_fwd[i], activ);
    digitalWrite(_bwd[i], activ);
    analogWrite(_pwm[i], 255);
  }
}

void Pilot::setMotEn(bool motEn) {
  if (_motEn != motEn) {
    _motEn = motEn;
    if (motEn) {
      if (!mate.timeout() && mate.role == 1) {
        _role = 2;
        _roleTimer = millis();
      } else {
        _role = 1;
      }
    } else {
      _role = 0;
      brake(true);
    }
  }
}

void Pilot::switchMotEn() {
  setMotEn(!_motEn);
}

bool Pilot::getMotEn() {
  return _motEn;
}

bool Pilot::setRusher() {
  if (_role != 2 && millis() - _roleTimer > ROLE_COOLDOWN) {
    _role = 2;
    _roleTimer = millis();
  }
}

bool Pilot::setKeeper() {
  if (_role != 1 && millis() - _roleTimer > ROLE_COOLDOWN) {
    _role = 1;
    _roleTimer = millis();
  }
}

bool Pilot::isRusher() {
  return _role == 2;
}

bool Pilot::isKeeper() {
  return _role == 1;
}

byte Pilot::getRole() {
  return _role;
}

