#ifndef Pilot_h
#define Pilot_h

#include "Arduino.h"
#include "Config.h"
#include "Utility.h"
#include "Pin.h"

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

  private:
    byte _fwd[4];    // digitaler PIN fuer Vorwaertsrotation
    byte _bwd[4];    // digitaler PIN fuer Rueckwaertsrotation
    byte _pwm[4];    // analoger PIN fuer Geschwindigkeit
    
    int _values[4];  // Zwischenspeicher für Outputsignale

    byte _angle;
};

#endif
