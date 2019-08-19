

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

// message is like  "SM",3
void smsReceivedHandler(char *message) {
  char msgId[10];
  char readCmd[20];
  Serial.println("Received SMS");
  Serial.println(message);
  char *ptr = strstr(message, ",");
  if (ptr != NULL) {
    strcpy(msgId, ptr + 1);     
    Serial.println(msgId);
    sprintf(readCmd, "AT+CMGR=%s", msgId);
    gsm.sendCmd(readCmd);
  }   
}

void smsReadyHandler(char *message) {
  gsm.sendCmd("AT+CMGF=1");
  gsm.sendCmd("AT+CSCS=\"GSM\"");
}

void initGsmMessageHandlers() {
  gsm.setHandler(CONNECTION, connectionHandler);
  gsm.setHandler(CONNECTION_ROAMING, connectionRoamingHandler);
  gsm.setHandler(DISCONNECTION, disconnectionHandler);
  gsm.setHandler(DATETIME_OK, clockHandler);
  gsm.setHandler(DATETIME_NOK, clockLostHandler);
  gsm.setHandler(NEW_SMS, smsReceivedHandler);
  gsm.setHandler(SMS_READY, smsReadyHandler);

}
