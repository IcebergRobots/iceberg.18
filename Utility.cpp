#include "Utility.h"

boolean battLow(){
  return (analogRead(BATT_VOLTAGE) * 0.01220703)>4 && (analogRead(BATT_VOLTAGE) * 0.01220703)<10.8;
}

void startSound() {
  //Fiepen, welches Programstart signalisiert
  for(int i = 10; i<2000; i += 10){
    tone(BUZZER, i);
    delay(1);
  }
  noTone(BUZZER);
}



