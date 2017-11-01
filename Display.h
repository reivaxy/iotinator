
#pragma once
#include "XOLEDDisplay.h"

class DisplayClass: public XOLEDDisplayClass {
public:
  DisplayClass(SSD1306* display);
  void batteryIcon(bool blink);
  void clockIcon(bool blink);
  void emptyBatteryIcon(bool blink);
  void alertIcon(bool blink);
  void wifiIcon(bool blink);
  void gsmIcon(bool blink);
  void roamingIcon(bool blink);
};
