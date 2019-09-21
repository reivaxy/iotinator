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
  char * copie = strdup(fullMessage);
  int size = MAX_MSG_LENGTH + 1;
  char firstLine[size];
  char *message;
  char number[30];   // could actually just be a pointer in the input string
  char date[40];     // could actually just be a pointer in the input string
  char* eol = strstr(copie, "\r\n");
  // TODO check eol
  
  *eol = 0;
  strlcpy(firstLine, copie, size);  
  Serial.println("got");
  Serial.println(firstLine);
  message = (char *)(eol + 2);
  Serial.println(message);

  char* sep = "\",\"";
  char* start = strstr(firstLine, sep) + strlen(sep);
  char* end = strstr(start, sep);
  // TODO: check end
  
  *end = 0; 
  Serial.printf("Start: $%s$, %d\n", start, strlen(start));
  Serial.printf("End: $%s$\n", end);
 
  strlcpy(number, start, size);
  Serial.printf("Number: $%s$\n", number);
  start = end + 2*strlen(sep); // empty field (would not be empty if number in sim card contact book)
  strlcpy(date, start, size);
  date[strlen(date) - 1] = 0; // remove last double quote
  Serial.println(number);
  Serial.println(date);
  //gsm.sendSMS(number, message);
  processSMS(message, number, date);
  free(copie);
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
