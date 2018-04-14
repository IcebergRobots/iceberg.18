#ifndef Ultrasonic_h
#define Ultrasonic_h

#include "Config.h"

#define CACHE_SIZE 4             // Länge des Input Byte Buffers

class Ultrasonic
{
  public:
    Ultrasonic();

    void send(byte * data, byte numberOfElements);
    byte cache();
    void receive();

  private:
    byte _cache[CACHE_SIZE]; // Zwischenspeicher für eingehende Bluetooth Nachrichten
    byte _cacheIndex = 255;  // aktuelle Schreibposition im Zwischenspeicher
};

#endif
