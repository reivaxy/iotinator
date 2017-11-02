
#pragma once
#include "XOLEDDisplay.h"

#define BLINKING true
#define NOT_BLINKING false
#define TRANSIENT true
#define NOT_TRANSIENT false

#define BLANK_ICON 65 // move this to xoled lib ?

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
  void refreshDateTime(char *dateTime);
};
