#ifndef Mate_h
#define Mate_h

#include "Config.h"

#define CACHE_SIZE 10             // Länge des Input Byte Buffers

class Mate
{
  public:
    Mate();

    byte receive();
    byte right();
    byte front();
    byte left();
    byte back();
    bool conn();
    void send(byte * data, byte numberOfElements);

    bool seeBall = false;
    bool role = 0;
    int ball = 0;
    unsigned int ballWidth = 0;

  private:
    byte fetch();

    byte cache[CACHE_SIZE]; // Zwischenspeicher für eingehende Bluetooth Nachrichten
    byte cacheIndex = 255;  // aktuelle Schreibposition im Zwischenspeicher
    byte distanceRight = 0;
    byte distanceFront = 0;
    byte distanceLeft = 0;
    byte distanceBack = 0;
    unsigned long responseTimer = 0;
};

#endif
