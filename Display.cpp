
#include "Display.h"

DisplayClass::DisplayClass(SSD1306* display): XOLEDDisplayClass(display) {
}

void DisplayClass::batteryIcon(bool blink) {
  setIcon(0, 66, blink);
}
void DisplayClass::clockIcon(bool blink) {
  setIcon(0, 75, blink);
}
void DisplayClass::emptyBatteryIcon(bool blink) {
  setIcon(0, 70, blink);
}
void DisplayClass::alertIcon(bool blink) {
  setIcon(1, 71, blink);
}
void DisplayClass::wifiIcon(bool blink) {
  setIcon(2, 72, blink);
}
void DisplayClass::gsmIcon(bool blink) {
  setIcon(3, 74, blink);
}
void DisplayClass::roamingIcon(bool blink) {
  setIcon(3, 73, blink);
}
