#ifndef Player_h
#define Player_h

#include "Config.h"

class Player
{
#define TOGGLE 2
#define LEFT 1
#define RIGHT 0

  public:
    Player();

    bool isKeeper();
    bool isRusher();
    void setKeeper(bool force);
    void setRusher(bool force);
    unsigned long lastRoleToggle();

    byte getState();
    void play();

    bool atGatepost();

  private:
    void setState(byte s, String reason);
    void changeState();
    void setDirection(byte dir, String reason);

    byte role = 0;                // Spielrolle: Torwart(0) / Stürmer(1)
    unsigned long roleTimer = 0;  // Zeitpunkt des letzten Rollenwechsels

    bool stateLeft = false;           // Fahrrichtung: rechts(0) / links(1)
    byte state = 0;               // Verteidigerrolle: Nach hinten(0) / Torverteidigung(1) / Pfostendrehung hin(2) / Pfostendrehung zurück(3) / Befreiung(4) / Seitlich verloren(5)
    // Angriffsrolle: Ballverfolgung(6) / Torausrichtung(7) / Angriff(8) / Ausweichen(9)
    unsigned long stateTimer = 0; // Zeitpunkt des letzten Statuswechsels
};

#endif
