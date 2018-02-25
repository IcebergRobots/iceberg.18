#ifndef pin_h
#define pin_h

#define EXT_A2          A2  //  Pin ohne feste Funktion. Verbunden mit Platinensteckverbinder
#define EXT_A3          A3  //  Pin ohne feste Funktion. Verbunden mit Platinensteckverbinder
#define EXT_A4          A4  //  Pin ohne feste Funktion. Verbunden mit Platinensteckverbinder
#define EXT_A6          A6  //  Pin ohne feste Funktion. Verbunden mit Platinensteckverbinder
#define EXT_A7          A7  //  Pin ohne feste Funktion. Verbunden mit Platinensteckverbinder
#define EXT_D45         45  //  Pin ohne feste Funktion. Verbunden mit Platinensteckverbinder
#define EXT_D47         47  //  Pin ohne feste Funktion. Verbunden mit Platinensteckverbinder
#define EXT_D49         49  //  Pin ohne feste Funktion. Verbunden mit Platinensteckverbinder
#define EXT_D53         53  //  Pin ohne feste Funktion. Verbunden mit Platinensteckverbinder. Pin 53 MUSS als Output gesetzt sein, nutzen von Pin 53 könnte zu Problemen mit der Pixy führen (ISP und so)

#define LIGHT_BARRIER   A0  //  
#define BATT_VOLTAGE    A15 //  Hier liegt 2/5 der Akkuspannung an.

#define POTI            A1  //  hier ist das Potentiometer unten auf der Platine angeschlossen
#define TYPE            A10 //  high bei Roboter A und low bei Roboter B

#define BIG_BUTTON      A5  //  hier ist der große Button auf der Platine angeschlossen
#define SCHUSS_BUTTON   8   //  hier ist ein Button angeschlussen mit der Beschriftung: Schuss
#define BUTTON_1        24  //  Hier wird Pushbutton 1 ausgelesen
#define BUTTON_2        22  //  Hier wird Pushbutton 2 ausgelesen
#define BUTTON_3        23  //  Hier wird Pushbutton 3 ausgelesen

#define SWITCH_SCHUSS   31  //  Hier wird ausgelesen, ob der Schuss eingeschaltet ist.
#define SWITCH_KEEPER   33  //  Hier wird ausgelesen, ob der Roboter Keeper oder Angreifer ist.
#define SWITCH_BODENS 35  //  Hier wird ausgelesen, ob die Bodensensoren eingeschaltet sind.
#define SWITCH_DEBUG    37  //  Hier wird ausgelesen, ob die Debug-LEDs eingeschaltet sind.
#define SWITCH_MOTOR    39  //  Hier wird ausgelesen, ob die Motoren eingeschaltet sind.
#define SWITCH_B        27  //  Dieser Schalter ist vorerst unbelegt.
#define SWITCH_A        29  //  Dieser Schalter ist vorerst unbelegt.

#define ROTARY_BUTTON   25  //  Rotary Encoder Taster
#define ROTARY_A        26  //  Rotary Encoder Rotation A
#define ROTARY_B        28  //  Rotary Encoder Rotation B

#define INFO_LED       6   //  hier sind 3 RGB-LEDs angeschlossen: State, Battery, Heartbeat
#define BUZZER          9   //  zum Ausgeben von Signaltönen
#define MATRIX_LED      41  //  Reihenfolge: Schuss, Mot. EN, Balls., Ballb., Bluet., Bodens., lift, Pixy, line, undefined, undefined, Debug

#define J_A8            A8  //  hier können Funktionen durch das setzten eines Lötpunktes auf die SMD-Pads eingestellt werden. Hiermit könnten beide Roboter mit gleichem Programm voneinander unterschieden werden.
#define J_A9            A9  //  hier können Funktionen durch das setzten eines Lötpunktes auf die SMD-Pads eingestellt werden. Hiermit könnten beide Roboter mit gleichem Programm voneinander unterschieden werden.
#define J_A10           A10 //  hier können Funktionen durch das setzten eines Lötpunktes auf die SMD-Pads eingestellt werden. Hiermit könnten beide Roboter mit gleichem Programm voneinander unterschieden werden.

#define RX0             0   //  Dieser Pin ist frei, da bei Belegung dieses Pins keine Programme hochgeladen werden können.
#define TX0             1   //  Dieser Pin ist frei, da bei Belegung dieses Pins keine Programme hochgeladen werden können.

#define TX1             18  //  Serielles Senden zum Bluetooth HC-05
#define RX1             19  //  Serielles Empfangen vom Bluetooth HC-05

#define TX2             16  //  Serielles Senden zum Bodensensor Arduino Nano
#define RX2             17  //  Serielles Empfangen vom Bodensensor Arduino Nano

#define TX3             14  //  Serielles Senden zum Ultraschall Arduino Nano
#define RX3             15  //  Serielles Empfangen vom Ultraschall Arduino Nano
#define I2C_D           20  //  I²C Bus: Daten
#define I2C_C           21  //  I²C Bus: Clock

#define INT_BODENSENSOR 2   //  hier sendet der Nano des Bodensensors sein Interruptsignal
#define INT_US          3   //  hier kann der Mega einen Interrupt an den Arduino Nano mit den US-Sensoren senden

#define SCHUSS          5   //  Wenn dieser Pin auf HIGH gesetzt wir, fährt der Schuss aus
#define BODEN_LED       7   //  Hier können die RGB-LEDs des Bodensensors gesteuert werden

#define DRDY            43  //  Hier kann ausgelesen werden, ob der Kompass Werte zur Verfügung stellen kann. (glaube ich)
#define TILTSWITCH      46  //  Hier kann der TILTSWITCH ausgelesen werden.

#define PIN_4           4   //  

#define BLUET_A         48  //  

#define FWD0            34
#define BWD0            36
#define PWM0            13

#define FWD1            30
#define BWD1            32
#define PWM1            10

#define FWD2            44
#define BWD2            42
#define PWM2            11

#define FWD3            38
#define BWD3            40
#define PWM3            12

#define M0_CURR         A11 //  Pin zum messen des Motorstroms von M1
#define M1_CURR         A12 //  Pin zum messen des Motorstroms von M2
#define M2_CURR         A13 //  Pin zum messen des Motorstroms von M3
#define M3_CURR         A14 //  Pin zum messen des Motorstroms von M4

#endif
