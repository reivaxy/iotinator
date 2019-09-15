// This is clumsy.
// When iotinator master is refactored into a subclass of XIOTModule, this will be
// refactored too
#define MAX_MSG_LENGTH 500

void processSMS(char* message, char* phoneNumber, char* date);


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

// message has a first header line, then text of message:
// message text is here (possibly multi line)
void smsReceivedHandler(char *fullMessage) {

  return;
  char firstLine[MAX_MSG_LENGTH + 1];
  char message[MAX_MSG_LENGTH + 1];
  char phoneNumber[MAX_MSG_LENGTH + 1];
  char date[MAX_MSG_LENGTH + 1];
  char* eol = strstr(fullMessage, "\r\n");
  strlcpy(firstLine, fullMessage, eol - fullMessage + 1);  
  Serial.println("got");
  Serial.println(firstLine);
  strlcpy(message, (const char *)(eol + 2), MAX_MSG_LENGTH + 1);
  Serial.println(message);
 
  char* sep = "\",\"";
  char* start = strstr(firstLine, sep) + strlen(sep);
  char* end = strstr(start, sep);
  *end = 0;  
  strcpy(phoneNumber, start);
  start = end + 2*strlen(sep); // empty field
  strcpy(date, start);
  date[strlen(date) - 1] = 0; // remove last double quote
  Serial.println(phoneNumber);
  Serial.println(date);

//  processSMS(message, phoneNumber, date);

}

void smsReadyHandler(char *message) {
  gsm.sendCmd("AT+CREG?");
}

void initGsmMessageHandlers() {
  gsm.setHandler(CONNECTION, connectionHandler);
  gsm.setHandler(CONNECTION_ROAMING, connectionRoamingHandler);
  gsm.setHandler(DISCONNECTION, disconnectionHandler);
  gsm.setHandler(DATETIME_OK, clockHandler);
  gsm.setHandler(DATETIME_NOK, clockLostHandler);
  gsm.setHandler(SMS_READ, smsReceivedHandler);
  gsm.setHandler(READY_FOR_SMS, smsReadyHandler);

}
