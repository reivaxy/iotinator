
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
void DisplayClass::refreshDateTime(char* dateTime) {
  // If icon 0 is showing the clock, delete it.
  if (getIconChar(0) == 75) {
    setIcon(0, BLANK_ICON); // Hide clock icon
  }
  setLine(4, dateTime, false, false);  // Display time not blinking
}

void DisplayClass::blinkDateTime(bool blink) {
  blinkLine(4, true); // blink time
}
