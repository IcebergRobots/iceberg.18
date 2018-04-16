#ifndef Ultrasonic_h
#define Ultrasonic_h

#include "Config.h"

#define CACHE_SIZE 4             // Länge des Input Byte Buffers

class Ultrasonic
{
  public:
    Ultrasonic();

    void receive();

    bool connected = false;

  private:
    byte fetch();

    byte cache[CACHE_SIZE]; // Zwischenspeicher für eingehende Bluetooth Nachrichten
    byte cacheIndex = 255;  // aktuelle Schreibposition im Zwischenspeicher
};

#endif
