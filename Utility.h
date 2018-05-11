#ifndef Utility_h
#define Utility_h

#include "Config.h"

int shift(int &value, int min, int max);
void startSound();
void calculateStates();
void transmitHeartbeat();
void setupMotor();
void avoidLine();
void kick();
void readCompass();
void buzzerTone(int duration);
int ausrichten(int orientation);
void readPixy();
void debug(String str);
void debug(long num);
void debugln(String str);
void debugln(long num);
void debugln();
#endif
