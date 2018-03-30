#ifndef Utility_h
#define Utility_h

#include "Config.h"

void shift(int &value, int min, int max);
void startSound();
void calculateStates();
void transmitHeartbeat();
void setupMotor();
bool readUltrasonic();
void avoidLine();
void kick();
int getCompassHeading();
void buzzerTone(int duration);
int ausrichten();
void readPixy();
#endif
