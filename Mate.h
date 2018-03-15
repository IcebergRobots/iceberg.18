#ifndef Mate_h
#define Mate_h

#include "Arduino.h"
#include "Config.h"
#include "Pin.h"
#include "Pilot.h"
//#include "Mate.h"
#include "Utility.h"

#define CACHE_SIZE 10

class Mate
{
  public:
    Mate();
  
    void sendBluetooth(byte * data, byte numberOfElements);
    byte receiveBluetooth();
    bool motEn;
    bool seeBall = false;
    int ball = 0;
    unsigned int ballWidth = 0;
    byte us[] = {0, 0, 0, 0};

  private:
    byte _cache[CACHE_SIZE]; // Zwischenspeicher für eingehende Bluetooth Nachrichten
    byte _cacheIndex = 255;  // aktuelle Schreibposition im Zwischenspeicher
};

#endif
