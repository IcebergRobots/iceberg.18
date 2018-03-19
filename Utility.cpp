#include "Utility.h"

void startSound() {
  //Fiepen, welches Programstart signalisiert
  for (int i = 10; i < 2000; i += 10) {
    tone(BUZZER, i);
    delay(1);
  }
  noTone(BUZZER);
}
