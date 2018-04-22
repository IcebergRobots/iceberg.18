#include "Display.h"

// Implementierung: OBJEKTE
extern Display d;
extern Keeper keeper;
extern Led led;
extern Mate mate;
extern Pilot m;
extern Ultrasonic us;

void Display::init() {
  begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialisiere das Displays
  clearDisplay(); // leere den Bildschirm
  drawBitmap(0, 0, LOGO, 114, 64, WHITE); // zeige das Logo
  display();  //wendet Aenderungen an
}

void Display::setupMessage(byte pos, String title, String description) {
  fillRect(47, 0, 81, 31, BLACK); // lösche das Textfeld
  drawRect(0, 29, map(pos, 0, SETUP_MESSAGE_RANGE, 0, 128), 2, WHITE);
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
  set();

  clearDisplay();
  setTextColor(WHITE);

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
  if (level == 0) {
    drawLine(map(page, 0, PAGE_RANGE, 3, 123), 11, map(page, -1, PAGE_RANGE - 1, 3, 123), 11, WHITE);
  } else if (level == 1) {
    drawLine(3, 11, map(subpage, 0, SUBPAGE_RANGE[page], 3, 123), 11, WHITE);
    drawLine(map(subpage, -1, SUBPAGE_RANGE[page] - 1, 3, 123), 11, 123, 11, WHITE);
  }

  // Pixy Boxen
  if (page == 3) {
    for (byte i = 0; i < blockCount; i++) { // geht alle erkannten Bloecke durch
      int width = map(pixy.blocks[i].width, 0, PIXY_MAX_X - PIXY_MIN_X + 1, 0, 121);
      int height = map(pixy.blocks[i].height, 0, PIXY_MAX_Y - PIXY_MIN_Y + 1, 0, 47);
      int x = map(pixy.blocks[i].x, PIXY_MIN_X, PIXY_MAX_X, 3, 123);
      int y = map(pixy.blocks[i].y , PIXY_MIN_Y, PIXY_MAX_Y, 13, 59);
      switch (pixy.blocks[i].signature) { // Was sehe ich?
        case SIGNATURE_BALL:
          if (subpage < 2) fillRect(x, y, width, height, WHITE);
          break;
        case SIGNATURE_GOAL:
          if (subpage == 0) drawRect(x, y, width, height, WHITE);
          else if (subpage == 2) fillRect(x, y, width, height, WHITE);
          break;
      }
    }
  }

  setTextSize(1);
  setCursor(3, 3);
  print(title.substring(0, 14) + String("               ").substring(0, max(1, 15 - title.length())) + runtime);

  setTextSize(2);
  setCursor(3, 14);
  print(line0.substring(0, 10));

  setCursor(3, 30);
  print(line1.substring(0, 10));

  setCursor(3, 46);
  print(line2.substring(0, 10));

  invertDisplay(m.getMotEn());
  display();      // aktualisiere Display

  lastDisplay = millis(); // merke Zeitpunkt
}

void Display::select() {
  if (level < 1) {
    level++;
    update();
  }
}

void Display::back() {
  if (level == 0) {
    asm ("jmp 0");   // starte den Arduino neu
  } else {
    level--;
    update();
  }
}

void Display::change(int change) {
  if (level == 0) {
    page += change;
    shift(page, 0, PAGE_RANGE);
    subpage = 0;
  } else if (level == 1) {
    subpage += change;
    shift(subpage, 0, SUBPAGE_RANGE[page]);
  }
  update();
}

void Display::set() {
  runtime = "";
  int min = numberOfMinutes(millis());
  if (min < 10) {
    runtime += "0";
  }
  runtime += String(min) + ":";
  int sec = numberOfSeconds(millis());
  if (sec < 10) {
    runtime += "0";
  }
  runtime += String(sec);

  title = "";
  line0 = "";
  line1 = "";
  line2 = "";

  switch (page) {
    case 0:
      if (isTypeA) {
        title = "IcebergRobotsA";
      } else {
        title = "IcebergRobotsB";
      }

      if (seeBall) {
        if (m.isRusher())      setLine(0, "rush", ball, true);
        else if (m.isKeeper()) setLine(0, "keep", ball, true);
        else                  setLine(0, "off", ball, true);
      } else {
        if (m.isRusher())      setLine(0, "rush", "blind");
        else if (m.isKeeper()) setLine(0, "keep", "blind");
        else                  setLine(0, "off", "blind");
      }

      setLine(1, driveState.substring(0, 6), driveDirection, true);
      if (batState == 2) {
        setLine(2, "lowVoltage");
      } else {
        setLine(2, displayDebug);
      }
      break;
    case 1:
      title = "Sensor";
      setLine(0, "Ball:", ball, true);
      if (!us.timeout()) {
        setLine(1, "^" + String(us.front()), String(us.right()) + ">");
        setLine(2, "<" + String(us.left()), String(us.back()) + "v");
      } else {
        setLine(1, "^", ">");
        setLine(2, "<", "v");
      }
      setLine(3, "Barr:", analogRead(LIGHT_BARRIER));
      setLine(4, "Volt:", String(batVol / 10) + "." + String(batVol % 10)); // battery voltage
      if (onLine) {
        setLine(5, "Line:", lineDir, true);
      } else {
        setLine(5, "Line:");
      }
      setLine(6, "Head:", heading, true);
      setLine(7, "Time:", millis() / 1000);
      setLine(8, "acc.X:", accel_event.acceleration.x, true);
      setLine(9, "acc.Y:", accel_event.acceleration.y, true);
      setLine(10, "acc.Z:", accel_event.acceleration.z, true);
      break;
    case 2:
      title = "Debug";
      break;
    case 3:
      char ball;
      if (!blockCountBall && seeBall) {
        ball = '*';
      } else {
        ball = String(blockCountBall).charAt(0);
      }
      char goal;
      if (!blockCountGoal && seeGoal) {
        goal = '*';
      } else {
        goal = String(blockCountGoal).charAt(0);
      }

      if (subpage == 0) {
        title = "Pixy Both " + String(ball) + "+" + String(goal);
      } else if (subpage == 1) {
        title = "Pixy Ball " + String(ball);
      } else if (subpage == 2) {
        title = "Pixy Goal   " + String(goal);
      }
      break;
    case 4:
      title = "Pixy";
      if (seeBall) {
        setLine(0, "B.ang:", ball, true);
      } else {
        setLine(0, "B.ang:");
      }
      setLine(1, "B.wid:", ballWidth);
      setLine(2, "B.siz:", ballSize);
      setLine(3, "B.tim:", (millis() - seeBallTimer) / 1000);
      if (seeGoal) {
        setLine(4, "G.ang:", goal, true);
      } else {
        setLine(4, "G.ang:");
      }
      setLine(5, "G.wid:", goalWidth);
      setLine(6, "G.siz:", goalSize);
      setLine(7, "G.tim", (millis() - seeGoalTimer) / 1000);
      break;
    case 5:
      title = "Driving";
      setLine(0, "Dir:", driveDirection, true);
      setLine(1, "Rot:", driveRotation, true);
      setLine(2, "Pwr:", drivePower, true);
      setLine(3, driveState);
      setLine(4, "Ori:", pidSetpoint, true);
      setLine(5, "Line:", onLine);
      setLine(6, "Head:", isHeadstart);
      setLine(7, "K.tim:", (millis() - lastKeeperToggle) / 1000);
      setLine(8, "H.tim:", (millis() - headstartTimer) / 1000);
      setLine(9, "F.tim:", (millis() - flatTimer) / 1000);
      setLine(10, "R.tim:", (millis() - m._roleTimer) / 1000);
      break;
    case 6:
      title = "Mate";
      setLine(0, "Conn:", mate.timeout() / 1000);
      setLine(1, "^" + String(mate.front()), String(mate.right()) + ">");
      setLine(2, "<" + String(mate.left()), String(mate.back()) + "v");
      /*if (mate.role == 0) setLine(3, "Role:", "off");
        if (mate.role == 1) setLine(3, "Role:", "keeper");
        if (mate.role == 2)*/ setLine(3, "Role:", mate.role);
      if (mate.seeBall) {
        setLine(4, "B.dif:", ball - mate.ball, true);
        setLine(5, "B.ang:", mate.ball, true);
      } else {
        setLine(4, "B.dif:");
        setLine(5, "B.ang:");
      }
      setLine(6, "B.wid:", mate.ballWidth);
      break;
  }
  if (batState == 3) {
    if (255 * (millis() % 250 < 170)) {
      line2 = "critVoltag";
    } else {
      line2 = "";
    }
  }
}

void Display::setLine(int line, String title, String value) {
  title += String("          ").substring(0, max(0, 10 - title.length() - value.length()));
  title = String(title + value).substring(0, 10);
  line -= subpage;
  shift(line, 0, SUBPAGE_RANGE[page]);
  if (line == 0) {
    line0 = title;
  } else if (line == 1) {
    line1 = title;
  } else if (line == 2) {
    line2 = title;
  }
}
void Display::setLine(int line, String title, long value, bool showPlus) {
  if (showPlus) {
    setLine(line, title, intToStr(value));
  } else {
    setLine(line, title, String(value));
  }
}
void Display::setLine(int line, String title, long value) {
  setLine(line, title, String(value));
}
void Display::setLine(int line, String title) {
  setLine(line, title, "");
}
void Display::setLine(int line) {
  setLine(line, "", "");
}

String Display::intToStr(int number) {
  if (number < 0) {
    return String(number);
  } else {
    return "+" + String(number);
  }
}

byte Display::getPage() {
  return page;
}

