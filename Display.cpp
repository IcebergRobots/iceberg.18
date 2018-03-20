#include "Display.h"

void Display::init() {
  begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialisiere das Displays
  clearDisplay(); // leere den Bildschirm
  drawBitmap(0, 0, logo, 114, 64, WHITE); // zeige das Logo
  display();  //wendet Aenderungen an
}
