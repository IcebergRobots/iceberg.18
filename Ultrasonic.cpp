#include "Ultrasonic.h"

// Implementierung: OBJEKTE
extern Mate mate;

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
  debug("us ");
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
    usRight = cache[0];
    usFront = cache[1];
    usLeft = cache[2];
    usBack = cache[3];
    us[0] = usRight;
    us[1] = usFront;
    us[2] = usLeft;
    us[3] = usBack;
    if (usLeft == 0)  usLeft = COURT_WIDTH - usRight;
    if (usRight == 0) usRight = COURT_WIDTH - usLeft;
    if (isConnected && mate.us[3] < 80) usBack = max(0, usBack - 80);
    usResponseTimer = millis(); // merke Zeitpunkt
  }
  usTimer = millis(); // merke Zeitpunkt
}

