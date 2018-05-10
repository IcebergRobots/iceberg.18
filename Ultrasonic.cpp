#include "Ultrasonic.h"

// Implementierung: OBJEKTE
extern Display d;
extern Player p;
extern Led led;
extern Mate mate;
extern Pilot m;
//extern Ultrasonic us;

Ultrasonic::Ultrasonic() {
}

/*****************************************************
  empfange Daten an des Patners
  - speichere diese im Cache
*****************************************************/
byte Ultrasonic::fetch() {
  // returns length of incomming message
  while (ULTRASONIC_SERIAL.available()) {
    byte b = ULTRASONIC_SERIAL.read();
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
  erfragt beim Ultraschallsensor durch einen Interrupt die aktuellen Sensorwerte
  empfängt und speichern diese Werte im globalen Array us[]:
    1
   .--.
  /    \ 0
  2 \    /
   '--'
     3
*****************************************************/
void Ultrasonic::receive() {
  debug("us");
  digitalWrite(INT_US, HIGH);  // sende eine Interrupt Aufforderung an den US-Arduino
  unsigned long timestamp = millis();
  byte messageLength = 0;
  while (millis() - timestamp < 3) {
    messageLength = fetch();
    if (messageLength == 4) break;
  }
  while (ULTRASONIC_SERIAL.available()) ULTRASONIC_SERIAL.read();
  digitalWrite(INT_US, LOW);  // beende das Interrupt Signal

  if (messageLength == 4) {
    distanceRight = cache[0];
    distanceFront = cache[1];
    distanceLeft = cache[2];
    distanceBack = cache[3];
    responseTimer = millis(); // merke Zeitpunkt
  }
  usTimer = millis(); // merke Zeitpunkt
}

byte Ultrasonic::right() {
  if (!distanceRight && distanceLeft > 0) return COURT_WIDTH - distanceLeft;
  else return distanceRight;
}

byte Ultrasonic::front() {
  return distanceFront;
}

byte Ultrasonic::left() {
  if (!distanceLeft && distanceRight > 0) return COURT_WIDTH - distanceRight;
  else return distanceLeft;
}

byte Ultrasonic::back() {
  if (p.isRusher()) return max(1, distanceBack - 40);
  else return distanceBack;
}

bool Ultrasonic::check() {
  return !timeout() && distanceRight && distanceLeft && distanceBack;
}

unsigned long Ultrasonic::timeout() {
  if (millis() - responseTimer < 500) return 0;
  else return millis() - responseTimer;
}
