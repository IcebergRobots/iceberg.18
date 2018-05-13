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
    unsigned long lastRoleToggle();

    void setState();
    byte getState();
    void play();

    bool atGatepost();

  private:
    byte role = 0;                // Spielrolle: Torwart(0) / Stürmer(1)
    unsigned long roleTimer = 0;  // Zeitpunkt des letzten Rollenwechsels

    bool stateLeft = 0;           // Fahrrichtung: rechts(0) / links(1)
    byte state = 0;               // Verteidigerrolle: rückwärts(0) / seitwärtsfahren(1) / Pfostendrehung hin(2) / Pfostendrehung zurück(3) / vorwärts(4) / seitlich verloren(5)
                                  // Angriffsrolle: Kompassausrichtung(6) / Ballorientierung mit Torausrichtung(7) / Richtung gesperrt(8)
    unsigned long stateTimer = 0; // Zeitpunkt des letzten Statuswechsels
};

#endif
