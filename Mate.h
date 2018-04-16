#ifndef Mate_h
#define Mate_h

#include "Config.h"

#define CACHE_SIZE 10             // Länge des Input Byte Buffers

class Mate
{
  public:
    Mate();

    void send(byte * data, byte numberOfElements);
    byte receive();
    bool role = 0;
    bool seeBall = false;
    int ball = 0;
    unsigned int ballWidth = 0;
    byte us[4] = {0, 0, 0, 0};

  private:
    byte fetch();

    byte cache[CACHE_SIZE]; // Zwischenspeicher für eingehende Bluetooth Nachrichten
    byte cacheIndex = 255;  // aktuelle Schreibposition im Zwischenspeicher
};

#endif
