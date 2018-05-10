#ifndef Player_h
#define Player_h

#include "Config.h"

class Player
{
  public:
    Player();

    bool isKeeper();
    bool isRusher();
    void setKeeper(bool force);
    void setRusher(bool force);
    byte getRole();
    unsigned long lastRoleToggle();

    void ball();
    void blind();

    bool atGatepost();
    void setState(byte s);
    void toggleStateDirection();

  private:
    void nextState(); // nächste Torwartrolle annehmen

    bool penalty = true;  // Spielrolle: Stürmer(0) / Torwart(1)
    unsigned long roleTimer = 0;

    bool stateLeft = 0; // Fahrrichtung: rechts(0) / links(1)
    byte state = 0;  // Torwartrolle: rückwärts(0) / seitwärtsfahren(1) / Pfostendrehung hin(2) / Pfostendrehung zurück(3)
    unsigned long stateTimer = 0; // Zeitpunkt des letzten Statuswechsels
};

#endif
