#ifndef Pilot_h
#define Pilot_h

#include "Config.h"

class Pilot
{
  public:
    Pilot();
    Pilot(byte angle);

    void setPins(byte id, byte fwd, byte bwd, byte pwm);
    void setAngle(byte angle);

    void steerMotor(byte id, int power);

    void drive();
    void drive(int values[]);
    void drive(int angle, int power);
    void drive(int angle, int power, int rotation);

    void calculate(int angle, int power);
    void calculate(int angle, int power, int rotation);

    void brake(bool activ);

    void setMotEn(bool motEn);
    bool getMotEn();
    void switchMotEn();

    bool setRusher();
    bool setKeeper();
    bool isRusher();
    bool isKeeper();
    byte getRole();

  private:
    byte _fwd[4];    // digitaler PIN fuer Vorwaertsrotation
    byte _bwd[4];    // digitaler PIN fuer Rueckwaertsrotation
    byte _pwm[4];    // analoger PIN fuer Geschwindigkeit

    int _values[4];  // Zwischenspeicher für Outputsignale

    byte _angle;
    byte _role = 0; // Spielrolle: Stürmer(2) / Torwart(1) / Aus(0)
    bool _motEn;    // gibt an, ob die Motoren an sind
    unsigned long _roleTimer = 0; // Zeitpunkt des letzten Rollenwechsels
};

#endif
