#ifndef Ultrasonic_h
#define Ultrasonic_h

#include "Config.h"

#define CACHE_SIZE 4 // Länge des Input Byte Buffers

class Ultrasonic
{
  public:
    Ultrasonic();

    void receive();
    byte right();
    byte front();
    byte left();
    byte back();
    bool check();
    unsigned long timeout();

  private:
    byte fetch();

    byte cache[CACHE_SIZE];     // Zwischenspeicher für eingehende Bluetooth Nachrichten
    byte cacheIndex = 255;      // aktuelle Schreibposition im Zwischenspeicher
    byte distanceRight = 0;
    byte distanceFront = 0;
    byte distanceLeft = 0;
    byte distanceBack = 0;
    unsigned long responseTimer = 0;
};

#endif
