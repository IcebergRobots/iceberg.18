#include "Display.h"

// Implementierung: OBJEKTE
//extern Display d;
extern Player p;
extern Led led;
extern Mate mate;
extern Pilot m;
extern Ultrasonic us;

void Display::init() {
  begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialisiere das Displays
  clearDisplay(); // leere den Bildschirm
  if (!silent) drawBitmap(0, 0, LOGO, 114, 64, WHITE); // zeige das Logo
  display();  //wendet Aenderungen an
}

void Display::setupMessage(byte pos, String title, String description) {
  if (!silent) {
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
}

// Infos auf dem Bildschirm anzeigen
void Display::update() {
  if(set() == false) {
    debug("reload");
    set();
  }

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
    drawLine(3, 11, map(subpage, 0, subpageRange[page], 3, 123), 11, WHITE);
    drawLine(map(subpage, -1, subpageRange[page] - 1, 3, 123), 11, 123, 11, WHITE);
  }

  // Pixy Boxen
  if (page == 3) {
    for (byte i = 0; i < blockCount; i++) { // geht alle erkannten Bloecke durch
      int width = map(pixy.blocks[i].width, 0, PIXY_MAX_X - PIXY_MIN_X + 1, 0, 121);
      int height = map(pixy.blocks[i].height, 0, PIXY_MAX_Y - PIXY_MIN_Y + 1, 0, 47);
      int x = map(pixy.blocks[i].x - width / 2, PIXY_MIN_X, PIXY_MAX_X, 3, 123);
      int y = map(pixy.blocks[i].y - height / 2, PIXY_MIN_Y, PIXY_MAX_Y, 13, 59);
      int angle = pixy.blocks[i].angle;
      switch (pixy.blocks[i].signature) { // Was sehe ich?
        case SIGNATURE_BALL:
          if (subpage < 2) fillRect(x, y, width, height, WHITE);
          break;
        case SIGNATURE_GOAL:
          if (subpage == 0) drawRect(x, y, width, height, WHITE);
          else if (subpage == 2) fillRect(x, y, width, height, WHITE);
          break;
        case SIGNATURE_CC:
          DEBUG_SERIAL.println(angle);
          if (subpage == 0 || subpage == 3) {
            if (angle < 0) drawLine(x + width, y, x, y + height, WHITE);
            else drawLine(x, y, x + width, y + height, WHITE);
          }
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
    reset();
  } else {
    level--;
    update();
  }
}

void Display::toggle() {
  if(level == 0) select();
  else back();
}

void Display::change(int change) {
  if (level == 0) {
    page += change;
    shift(page, 0, PAGE_RANGE);
    subpage = 0;
  } else if (level == 1) {
    subpage += change;
    shift(subpage, 0, subpageRange[page]);
  }
  update();
}

bool Display::set() {
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
  lineIndex = 0;

  switch (page) {
    case 0:
      if (isTypeA) title = "IcebergRobotsA";
      else title = "IcebergRobotsB";

      if (seeBall) {
        if (p.isRusher())      addLine("rush", ball, true);
        else if (p.isKeeper()) addLine("keep", ball, true);
        else                   addLine("off", ball, true);
      } else {
        if (p.isRusher())      addLine("rush", "blind");
        else if (p.isKeeper()) addLine("keep", "blind");
        else                   addLine("off", "blind");
      }
      addLine(driveState.substring(0, 6), driveDirection, true);
      if (batState == 2) addLine("lowVoltage");
      else addLine(displayDebug);
      addLine("==========");
      break;
    case 1:
      title = "Sensor";
      addLine("Ball:", ball, true);
      if (!us.timeout()) {
        addLine("^" + String(us.front()), String(us.right()) + ">");
        addLine("<" + String(us.left()), String(us.back()) + "v");
      }
      addLine("Barr:", analogRead(LIGHT_BARRIER));
      addLine("Volt:", String(batVol / 10) + "." + String(batVol % 10)); // battery voltage
      if (onLine) {
        addLine("Line:", lineDir, true);
      } else {
        addLine("Line:");
      }
      addLine("Head:", heading, true);
      addLine("Time:", millis() / 1000);
      addLine("acc.X:", accel_event.acceleration.x, true);
      addLine("acc.Y:", accel_event.acceleration.y, true);
      addLine("acc.Z:", accel_event.acceleration.z, true);
      addLine("==========");
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
      char east;
      if (!blockCountEast && seeEast) {
        east = '*';
      } else {
        east = String(blockCountEast).charAt(0);
      }
      char west;
      if (!blockCountWest && seeWest) {
        west = '*';
      } else {
        west = String(blockCountWest).charAt(0);
      }

      if (subpage == 0) {
        title = "Pixy " + String(ball) + "+" + String(goal) + "+" + String(west) + "+" + String(east);
      } else if (subpage == 1) {
        title = "Pixy Ball " + String(ball);
      } else if (subpage == 2) {
        title = "Pixy Goal   " + String(goal);
      } else if (subpage == 3) {
        title = "Pixy CC " + String(west) + "+" + String(east);
      }
      addLine();
      addLine();
      addLine();
      addLine();
      break;
    case 4:
      title = "Pixy";
      if (seeBall) addLine("B.ang:", ball, true);
      else addLine("B.ang:");
      addLine("B.wid:", ballWidth);
      addLine("B.siz:", ballArea);
      addLine("B.tim:", (millis() - seeBallTimer) / 1000);
      if (seeGoal) addLine("G.ang:", goal, true);
      else addLine("G.ang:");
      addLine("G.wid:", goalWidth);
      addLine("G.siz:", goalArea);
      addLine("G.tim", (millis() - seeGoalTimer) / 1000);
      addLine("==========");
      break;
    case 5:
      title = "Driving";
      addLine("Dir:", driveDirection, true);
      addLine("Rot:", driveRotation, true);
      addLine("Pwr:", drivePower, true);
      addLine(driveState);
      addLine("Ori:", pidSetpoint, true);
      addLine("Line:", onLine);
      addLine("Head:", isHeadstart);
      addLine("K.tim:", (millis() - lastKeeperToggle) / 1000);
      addLine("H.tim:", (millis() - headstartTimer) / 1000);
      addLine("F.tim:", (millis() - flatTimer) / 1000);
      addLine("R.tim:", p.lastRoleToggle() / 1000);
      addLine("==========");
      break;
    case 6:
      title = "Mate";
      addLine("Conn:", mate.timeout() / 1000);
      if (!mate.timeout()) {
        addLine("^" + String(mate.front()), String(mate.right()) + ">");
        addLine("<" + String(mate.left()), String(mate.back()) + "v");
        if (!mate.getMotEn()) addLine("Role:", "off");
        else if (mate.isKeeper()) addLine("Role:", "keeper");
        else if (mate.isRusher()) addLine("Role:", "rusher");
        if (mate.seeBall) {
          addLine("B.dif:", ball - mate.ball, true);
          addLine("B.ang:", mate.ball, true);
        } else {
          addLine("B.dif:");
          addLine("B.ang:");
        }
        addLine("B.wid:", mate.ballWidth);
      }
      addLine("==========");
      break;
  }
  if (batState == 3) {
    if (255 * (millis() % 2000 < 1000)) {
      line2 = "critVoltag";
    } else {
      line2 = "";
    }
  }
  if(subpageRange[page] != lineIndex) {
    subpageRange[page] = lineIndex;
    return false;
  } else {
    return true;
  }
}

void Display::addLine(String title, String value) {
  title += String("          ").substring(0, max(0, 10 - title.length() - value.length()));
  title = String(title + value).substring(0, 10);
  int line = lineIndex - subpage;
  shift(line, 0, subpageRange[page]);
  if (line == 0) {
    line0 = title;
  } else if (line == 1) {
    line1 = title;
  } else if (line == 2) {
    line2 = title;
  }
  lineIndex++;

}
void Display::addLine(String title, long value, bool showPlus) {
  if (showPlus) {
    addLine(title, intToStr(value));
  } else {
    addLine(title, String(value));
  }
}
void Display::addLine(String title, long value) {
  addLine(title, String(value));
}
void Display::addLine(String title) {
  addLine(title, "");
}
void Display::addLine() {
  addLine("", "");
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

