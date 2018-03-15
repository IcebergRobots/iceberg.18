#include "Mate.h"

/*****************************************************
  setze Baudrate für Bluetooth Modul
*****************************************************/
Mate::Mate() {
}

/*****************************************************
  übertrage Daten an den Patner
*****************************************************/
void Mate::sendBluetooth(byte * data, byte numberOfElements) {
  BLUETOOTH_SERIAL.write(START_MARKER);
  for (byte i = 0; i < numberOfElements; i++) {
    BLUETOOTH_SERIAL.write(constrain(data[i], 0, 253));
  }
  BLUETOOTH_SERIAL.write(END_MARKER);
}

/*****************************************************
  empfange Daten an des Patners
*****************************************************/
byte Mate::cache() {
  // returns length of incomming message
  while (BLUETOOTH_SERIAL.available() > 0) {
    byte b = BLUETOOTH_SERIAL.read();
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
  übertrage Daten an den Patner
*****************************************************/
byte Mate::receiveBluetooth() {
  // aktualisiere den Cache
  if (cache() == 9 && _chache[0] == 104) {
    // cache ist 9 zeilen lang und vom Typ Heartbeat
    motEn = _cache[1] < 3; // speichere Motorzustand
    if (motEn) {
      // Partner is aktiv
      if (_cache[1] == 2) {
        // Partner ist blind
        seeBall = false;
      } else {
        // Partner sieht den Ball
        seeBall = true;
        ball = -(cache[1] == 1) * cache[2]; // speichere die Ballabweichung
      }
    }
    ballWidth = _cache[3] + 254 * _cache[4];  // speichere die Ballbreite
    us[0] = _cache[5];
    us[1] = _cache[6];
    us[2] = _cache[7];
    us[3] = _cache[8];
  }
  return _cache[0];
}

