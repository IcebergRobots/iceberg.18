#include "Display.h"

// Implementierung: OBJEKTE
extern Pilot m;
extern Mate mate;
extern Led led;

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
  if (_level == 0) {
    drawLine(map(_page, 0, PAGE_RANGE, 3, 123), 11, map(_page, -1, PAGE_RANGE - 1, 3, 123), 11, WHITE);
  } else if (_level == 1) {
    drawLine(3, 11, map(_subpage, 0, SUBPAGE_RANGE[_page], 3, 123), 11, WHITE);
    drawLine(map(_subpage, -1, SUBPAGE_RANGE[_page] - 1, 3, 123), 11, 123, 11, WHITE);
  }

  setTextSize(1);
  setCursor(3, 3);
  print(_title.substring(0, 14) + String("               ").substring(0, max(1, 15 - _title.length())) + _time);

  setTextSize(2);
  setCursor(3, 14);
  print(_line0.substring(0, 10));

  setCursor(3, 30);
  print(_line1.substring(0, 10));

  setCursor(3, 46);
  print(_line2.substring(0, 10));

  invertDisplay(m.getMotEn());
  display();      // aktualisiere Display

  lastDisplay = millis(); // merke Zeitpunkt
}

void Display::select() {
  if (_level < 2) {
    _level++;
    update();
  }
}

void Display::back() {
  if (_level == 0) {
    asm ("jmp 0");   // starte den Arduino neu
  } else {
    _level--;
    update();
  }
}

void Display::change(int change) {
  if (_level == 0) {
    _page += change;
    shift(_page, 0, PAGE_RANGE);
    _subpage = 0;
  } else if (_level == 1) {
    _subpage += change;
    shift(_subpage, 0, SUBPAGE_RANGE[_page]);
  }
  update();
}

void Display::set() {
  _time = "";
  int min = numberOfMinutes(millis());
  if (min < 10) {
    _time += "0";
  }
  _time += String(min) + ":";
  int sec = numberOfSeconds(millis());
  if (sec < 10) {
    _time += "0";
  }
  _time += String(sec);

  _title = "";
  _line0 = "";
  _line1 = "";
  _line2 = "";

  switch (_page) {
    case 0:
      if (isTypeA) {
        _title = "IcebergRobotsA";
      } else {
        _title = "IcebergRobotsB";
      }

      if (seeBall) {
        setLine(0, "Ball:", ball, true);
        //setCursor(50, 20);
        //setTextSize(1);
        //println(ball);
        //drawLine(91, 27, constrain(map(ball, -160, 160, 60, 123), 60, 123), 14, WHITE);
      } else {
        setLine(0, "Ball:blind");
      }

      setLine(1, "Dir:", driveDir);
      if (batState == 2) {
        setLine(2, "lowVoltage");
      } else {
        setLine(2, displayDebug);
      }
      break;
    case 1:
      _title = "Sensor";
      setLine(0, "Ball:", ball, true);
      setLine(1, "^" + String(us[1]), String(us[0]) + ">");
      setLine(2, "<" + String(us[2]), String(us[3]) + "v");
      setLine(3, "Barr:", analogRead(LIGHT_BARRIER));
      setLine(4, "Volt:", String(batVol / 10) + "." + String(batVol % 10)); // battery voltage
      if (onLine) {
        setLine(5, "Line:", lineDir, true);
      } else {
        setLine(5, "Line:");
      }
      setLine(6, "Head:", heading, true);
      setLine(7, "T:", millis());
      break;
    case 2:
      _title = "Config";
      break;
    case 3:
      _title = "Pixy";
      break;
    case 4:
      _title = "Driving";
      break;
    case 5:
      _title = "Mate";
      break;
  }
  if (batState == 3) {
    if (255 * (millis() % 250 < 170)) {
      _line2 = "critVoltag";
    } else {
      _line2 = "";
    }
  }
}

void Display::setLine(int line, String title, String value) {
  title += String("          ").substring(0, max(0, 10 - title.length() - value.length()));
  title = String(title + value).substring(0, 10);
  line -= _subpage;
  shift(line, 0, SUBPAGE_RANGE[_page]);
  if (line == 0) {
    _line0 = title;
  } else if (line == 1) {
    _line1 = title;
  } else if (line == 2) {
    _line2 = title;
  }
}
void Display::setLine(int line, String title, int value, bool showPlus) {
  if (showPlus) {
    setLine(line, title, intToStr(value));
  } else {
    setLine(line, title, String(value));
  }
}
void Display::setLine(int line, String title, int value) {
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

