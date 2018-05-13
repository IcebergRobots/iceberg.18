#include "Mate.h"

// Implementierung: OBJEKTE
extern Display d;
extern Player p;
extern Led led;
//extern Mate mate;
extern Pilot m;
extern Ultrasonic us;

Mate::Mate() {
}

/*****************************************************
  übertrage Daten an den Patner
  @param data: Datenarray
  @param numberOfElements: Länge des Arrays
*****************************************************/
void Mate::send(byte * data, byte numberOfElements) {
  if (BLUETOOTH) {
    BLUETOOTH_SERIAL.write(START_MARKER);
    for (byte i = 0; i < numberOfElements; i++) {
      BLUETOOTH_SERIAL.write(constrain(data[i], 0, 253));
    }
    BLUETOOTH_SERIAL.write(END_MARKER);
  }
}

/*****************************************************
  empfange Daten an des Patners
  - speichere diese im Cache
*****************************************************/
byte Mate::fetch() {
  // returns length of incomming message
  while (BLUETOOTH_SERIAL.available()) {
    byte b = BLUETOOTH_SERIAL.read();
    if (cacheIndex != 255) { // aktives Zuhören?
      if (b == START_MARKER) {
        cacheIndex = 0;  // aktiviere Zuhören
        for (byte i = 0; i < CACHE_SIZE; i++) {
          cache[i] = 255; // überschreibe den Cache
        }
      } else if (b == END_MARKER) {
        byte messageLength = cacheIndex;
        cacheIndex = 255; // deaktiviere Zuhören
        return messageLength; // Befehl empfangen!
      } else {
        if (cacheIndex >= CACHE_SIZE) {
          cacheIndex = 255; // deaktiviere Zuhören
        } else {
          cache[cacheIndex] = b;  // speichere in Cache
          cacheIndex += 1;  // speichere index
        }
      }
    } else {
      if (b == START_MARKER) {
        cacheIndex = 0; // aktiviere Zuhören
        for (byte i = 0; i < CACHE_SIZE; i++) {
          cache[i] = 255; // überschreibe den Cache
        }
      }
    }

  }
  return 0;
}

/*****************************************************
  lese Nachrichten aus dem Cache aus
*****************************************************/
byte Mate::receive() {
  byte messageLength = fetch(); // aktualisiere den Cache
  if (messageLength == 10 && cache[0] == 'h') {
    /*Byte    Information   mögliche Zustände
      -----------------------------------------------------
      0       Pakettyps     Heartbeat(104)
      1       Rolle         Stürmer(2) / Torwart(1) / Aus(0)
      2       Ballzustand   blind(2) / links(1) / rechts(0)
      3       Ballwinkel    (0 bis 253)
      4,5     Ballbreite    je (0 bis 253)
      6,7,8,9 Ultraschall   je (0 bis 253)
    */
    role = cache[1] / 10;
    state = cache[1] % 10;
    seeBall = cache[2] < 2;
    if (cache[2] == 1) ball = -cache[3];
    else ball = cache[3];
    ballWidth = cache[4] + 254 * cache[5];  // speichere die Ballbreite
    distanceRight = cache[6];
    distanceFront = cache[7];
    distanceLeft = cache[8];
    distanceBack = cache[9];
    responseTimer = millis();
  }
  if (messageLength > 0) {
    return cache[0];
  } else {
    return 255;
  }
}

bool Mate::getMotEn() {
  return role > 0;
}

bool Mate::isKeeper() {
  return role == 1;
}

bool Mate::isRusher() {
  return role == 2;
}

byte Mate::getState() {
  return state;
}

byte Mate::right() {
  return distanceRight;
}

byte Mate::front() {
  return distanceFront;
}

byte Mate::left() {
  return distanceLeft;
}

byte Mate::back() {
  return distanceBack;
}

unsigned long Mate::timeout() {
  if(millis() - responseTimer < 500) return 0;
  else return millis() - responseTimer;
}
