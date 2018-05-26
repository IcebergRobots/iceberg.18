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
  if (BLUETOOTH && digitalRead(SWITCH_KEEPER)) {
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
  if (BLUETOOTH && digitalRead(SWITCH_KEEPER)) {
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
  }
  return 0;
}

/*****************************************************
  lese Nachrichten aus dem Cache aus
*****************************************************/
byte Mate::receive() {
  byte messageLength = fetch(); // aktualisiere den Cache
  if (messageLength == 3 && cache[0] == 'h') {
/***************************************************** 
  Sende einen Herzschlag mit Statusinformationen an den Partner 
 
  Byte    Information   mögliche Zustände 
  ----------------------------------------------------- 
  0       Pakettyps     Heartbeat(104) 
  1       Status+Rolle  Aus(0+Status) / Torwart(1+Status) / Stürmer(2+Status) 
  2       Score         Blind(0) / Bewertung(...) 
*****************************************************/ 
    role = cache[1] / 10;
    state = cache[1] % 10;
    score = cache[2];
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

byte Mate::getScore() { 
  return score; 
}

unsigned long Mate::timeout() {
  if (millis() - responseTimer < 500) return 0;
  else return millis() - responseTimer;
}
