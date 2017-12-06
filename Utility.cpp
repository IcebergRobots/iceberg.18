#include "Utility.h"

boolean battLow(){
  return (analogRead(BATT_VOLTAGE) * 0.01220703)>4 && (analogRead(BATT_VOLTAGE) * 0.01220703)<10.8;
}
