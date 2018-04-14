#include "Ultrasonic.h"

Ultrasonic::Ultrasonic() {
}

/*****************************************************
  empfange Daten an des Patners
  - speichere diese im Cache
*****************************************************/
byte Ultrasonic::cache() {
  // returns length of incomming message
  while (ULTRASONIC_SERIAL.available()) {
    byte b = ULTRASONIC_SERIAL.read();
    if (_cacheIndex != 255) { // aktives Zuhören?
      if (b == START_MARKER) {
        _cacheIndex = 0;  // aktiviere Zuhören
        for (byte i = 0; i < CACHE_SIZE; i++) {
          _cache[i] = 255; // überschreibe den Cache
        }
      } else if (b == END_MARKER) {
        byte messageLength = _cacheIndex;
        _cacheIndex = 255; // deaktiviere Zuhören
        return messageLength; // Befehl empfangen!
      } else {
        if (_cacheIndex >= CACHE_SIZE) {
          _cacheIndex = 255; // deaktiviere Zuhören
        } else {
          _cache[_cacheIndex] = b;  // speichere in Cache
          _cacheIndex += 1;  // speichere index
        }
      }
    } else {
      if (b == START_MARKER) {
        _cacheIndex = 0; // aktiviere Zuhören
        for (byte i = 0; i < CACHE_SIZE; i++) {
          _cache[i] = 255; // überschreibe den Cache
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
    messageLength = cache();
    if (messageLength == 4) break;
  }
  while (ULTRASONIC_SERIAL.available()) ULTRASONIC_SERIAL.read();
  digitalWrite(INT_US, LOW);  // beende das Interrupt Signal

  if (messageLength == 4) {
    us[0] = _cache[0];
    us[1] = _cache[1];
    us[2] = _cache[2];
    us[3] = _cache[3];
    usResponseTimer = millis(); // merke Zeitpunkt
  }
  usTimer = millis(); // merke Zeitpunkt
}

