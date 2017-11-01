
void connectionHandler(char *message) {
  if (strstr(message, "0,5")) {
    oledDisplay->roamingIcon(false);
  } else if (strstr(message, "0,1")) {
    oledDisplay->gsmIcon(false);
  } else {
    oledDisplay->gsmIcon(true); // blinking icon : not connected
  }
}

void clockHandler(char *message) {
  // when datetime is not yet initialised it defaults to 04/01/01 at least in my SIM module
  if (message[1] == '0') return;
  char timeMsg[100];
  int hour, minute;
  char date[20];
  char dummy[20];
  if (oledDisplay->getIconChar(0) == 75) {
    oledDisplay->setIcon(0, 65); // Hide clock icon
  }
  strcpy(timeMsg, ++message);
  timeMsg[8] = ' ';
  timeMsg[14] = 0;
  oledDisplay->setLine(4, timeMsg);  // Display time
  
}

void initGsmMessageHandlers() {
  gsm.setConnectionHandler(connectionHandler);
  gsm.setClockHandler(clockHandler);
}
