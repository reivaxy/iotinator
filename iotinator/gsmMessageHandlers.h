

void connectionHandler(char *message) {
  oledDisplay->gsmIcon(false);
}
void connectionRoamingHandler(char *message) {
  oledDisplay->roamingIcon(false);
}
void disconnectionHandler(char *message) {
  oledDisplay->gsmIcon(true); // blinking icon : not connected
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
void clockLostHandler(char *message) {
  oledDisplay->blinkDateTime(true);  // Display time 
}

void smsReceivedHandler(char *message) {

}

void initGsmMessageHandlers() {
  gsm.setHandler(CONNECTION, connectionHandler);
  gsm.setHandler(CONNECTION_ROAMING, connectionRoamingHandler);
  gsm.setHandler(DISCONNECTION, disconnectionHandler);
  gsm.setHandler(DATETIME_OK, clockHandler);
  gsm.setHandler(DATETIME_NOK, clockLostHandler);
  /*
  gsm.setSmsReceivedHandler(smsReceivedHandler);
  */
}
