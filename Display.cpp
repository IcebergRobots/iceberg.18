#include "Display.h"

// Implementierung: OBJEKTE
extern Pilot m;
extern Mate mate;
extern Led led;

void Display::init() {
  begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialisiere das Displays
  clearDisplay(); // leere den Bildschirm
  drawBitmap(0, 0, logo, 114, 64, WHITE); // zeige das Logo
  display();  //wendet Aenderungen an
}

void Display::setupMessage(byte pos, String title, String description) {
  fillRect(47, 0, 81, 31, BLACK); // lösche das Textfeld
  drawRect(46, 29, map(pos, 0, SETUP_MESSAGE_RANGE, 0, 82), 2, WHITE);
  setTextColor(WHITE);

  setTextSize(2);
  setCursor(47, 0);
  print(title.substring(0, 6));

  setTextSize(1);
  setCursor(47, 17);
  print(description.substring(0, 13));

  display();
}

// Infos auf dem Bildschirm anzeigen
void Display::update() {
  String myTime = "";
  int min = numberOfMinutes(millis());
  if (min < 10) {
    myTime += "0";
  }
  myTime += String(min) + ":";
  int sec = numberOfSeconds(millis());
  if (sec < 10) {
    myTime += "0";
  }
  myTime += String(sec);

  clearDisplay();
  setTextColor(WHITE);
  setTextSize(1);
  setCursor(3, 3);
  if (isTypeA) {
    println("IcebergRobotsA " + myTime);
  } else {
    println("IcebergRobotsB " + myTime);
  }
  if (heading < -135) { // zeige einen Punkt, der zum Tor zeigt
    drawRect(map(heading, -180, -134, 63 , 125), 61, 2, 2, WHITE); //unten (rechte Hälfte)
  } else if (heading < -45) {
    drawRect(125, map(heading, -135, -44, 61, 0), 2, 2, WHITE); //rechts
  } else if (heading < 45) {
    drawRect(map(heading, -45, 44, 125, 0), 0, 2, 2, WHITE); //oben
  } else if (heading < 135) {
    drawRect(0, map(heading, 45, 134, 0, 61), 2, 2, WHITE); //links
  } else if (heading < 180) {
    drawRect(map(heading, 135, 179, 0, 62), 61, 2, 2, WHITE); //unten (linke Hälfte)
  }
  setTextSize(2);
  setCursor(3, 14);
  if (seeBall) {
    println("Ball");
    setCursor(50, 20);
    setTextSize(1);
    println(ball);
    drawLine(91, 27, constrain(map(ball, -160, 160, 60, 123), 60, 123), 14, WHITE);
  } else {
    println("Ball:blind");
  }
  setTextSize(2);
  drawLine(3, 11, map(drivePwr, 0, 255, 3, 123), 11, WHITE);

  String name1 = "";
  String name2 = "";
  String value1 = "";
  String value2 = "";

  switch (rotaryPosition) {
    case 0:
      name1 = "Dir:";
      value1 = String(driveDir);
      if (batState == 2) {
        name2 = "lowVoltage";
      } else {
        name2 = String(displayDebug);
      }
      break;
    case 1:
      name1 = "^";
      value1 = ">";
      name2 = "<";
      value2 = "v";
      setCursor(21, 30);
      print(us[1] + String("   ").substring(0, 3 - String(us[1]).length() ));
      print(String("    ").substring(0, 4 - String(us[0]).length()) + String(us[0]) );
      setCursor(21, 46);
      print(us[2] + String("   ").substring(0, 3 - String(us[2]).length() ));
      print(String("    ").substring(0, 4 - String(us[3]).length()) + String(us[3]));
      break;
    case 2:
      name1 = "dPwr:";
      value1 = intToStr(drivePwr);   // drive power
      name2 = "dRot:";
      value2 = intToStr(driveRot);   // drive rotation
      break;
    case 3:
      name1 = "rotMp:";
      value1 = intToStr(rotMulti);  // ratation multiplier
      name2 = "balX:";
      value2 = intToStr(ball);   // ball angle
      break;
    case 4:
      name1 = "t:";
      value1 = String(millis()); // ratation multiplier
      name2 = "headi:";
      value2 = intToStr(heading);  // heading
      break;
    case 5:
      name1 = "batVo:";
      value1 = String(batVol / 10) + "." + String(batVol % 10); // bluetooth command
      name2 = "start:";
      value2 = String(start);      // start
      break;
    case 6:
      name1 = "bWid:";
      value1 = String(ballWidth);  // ball box width
      name2 = "bSiz:";
      value2 = String(ballSize);  // ball box height*width
      break;
    case 7:
      name1 = "gWid";
      value1 = String(goalWidth);
      name2 = "gSiz";
      value2 = String(goalSize);
      break;
    case 8:
      name1 = "gX:";
      value1 = intToStr(goal);
      // 3 - 123
      int goalLeft;
      goalLeft = X_CENTER + goal - goalWidth / 2;
      goalLeft = constrain(map(goalLeft, PIXY_MIN_X, PIXY_MAX_X, 3, 123), 3, 123);
      fillRect(goalLeft, 46, constrain(map(goalWidth, 0, PIXY_MAX_X - PIXY_MIN_X, 0, 123), 0, 123), 32, true); // zeige die Torbreite
      break;
    case 9:
      name1 = "Mball:";
      if (mate.seeBall) {
        value1 = intToStr(mate.ball);
      } else {
        value1 = "blind";
      }
      name2 = "Mwid:";
      value2 = String(mate.ballWidth);
      break;
    case 10:
      name1 = "^";
      value1 = ">";
      name2 = "<";
      value2 = "v";
      setCursor(21, 30);
      print(mate.us[1] + String("   ").substring(0, 3 - String(mate.us[1]).length() ));
      print(String("M   ").substring(0, 4 - String(mate.us[0]).length()) + String(mate.us[0]) );
      setCursor(21, 46);
      print(mate.us[2] + String("   ").substring(0, 3 - String(mate.us[2]).length() ));
      print(String("M   ").substring(0, 4 - String(mate.us[3]).length()) + String(mate.us[3]));
      break;
  }

  name1 += String("          ").substring(0, max(0, 10 - name1.length() - value1.length()));
  name1 = String(name1 + value1).substring(0, 10);
  name2 += String("          ").substring(0, max(0, 10 - name2.length() - value2.length()));
  name2 = String(name2 + value2).substring(0, 10);
  if (batState == 3) {
    if (255 * (millis() % 250 < 170)) {
      name2 = "critVoltag";
    } else {
      name2 = "";
    }
  }
  setCursor(3, 30);
  print(name1);
  setCursor(3, 46);
  print(name2);

  invertDisplay(m.getMotEn());
  display();      // aktualisiere Display
  
  lastDisplay = millis(); // merke Zeitpunkt
}

String Display::intToStr(int number) {
  if (number < 0) {
    return String(number);
  } else {
    return "+" + String(number);
  }
}

