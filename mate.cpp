#include "Mate.h"

/*****************************************************
setze Achsenwinkel auf 70°
*****************************************************/
Mate::Mate(){
}

/*****************************************************
setze Achsenwinkel
*****************************************************/
void Mate::send(byte * data, byte numberOfElements) {
  BLUETOOTH_SERIAL.write(START_MARKER);
  for (byte i = 0; i < numberOfElements; i++) {
    BLUETOOTH_SERIAL.write(constrain(data[i], 0, 253));
  }
  BLUETOOTH_SERIAL.write(END_MARKER);
}

/*****************************************************
setze Motor-Ansteuerungspins
*****************************************************/
// Bluetooth auswerten
byte Mate::receive() {
  // returns length of incomming message
  while (BLUETOOTH_SERIAL.available() > 0) {
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
