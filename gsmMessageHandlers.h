
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
  strcpy(timeMsg, ++message);
  timeMsg[8] = ' ';
  timeMsg[14] = 0;
  oledDisplay->refreshDateTime(timeMsg);  // Display time
  
}

void smsReceivedHandler(char *message) {

}

void initGsmMessageHandlers() {
  gsm.setConnectionHandler(connectionHandler);
  gsm.setClockHandler(clockHandler);
  gsm.setSmsReceivedHandler(smsReceivedHandler);
}
