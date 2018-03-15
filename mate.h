#ifndef Mate_h
#define Mate_h

#include "Arduino.h"
#include "Config.h"
#include "Utility.h"
#include "Pin.h"

class Mate
{
  public:
    Mate();
  
    void send(byte * data, byte numberOfElements);
    
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

  private:
    byte _fwd[4];    // digitaler PIN fuer Vorwaertsrotation
    byte _bwd[4];    // digitaler PIN fuer Rueckwaertsrotation
    byte _pwm[4];    // analoger PIN fuer Geschwindigkeit
    
    int _values[4];  // Zwischenspeicher für Outputsignale

    byte _angle;
    bool _motEn;    //gibt an, ob die Motoren an sind
};

#endif
