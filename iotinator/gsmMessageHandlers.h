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
  strlcpy(timeMsg, ++message, 100);
  timeMsg[8] = ' ';
  timeMsg[14] = 0;
  oledDisplay->refreshDateTime(timeMsg);  // Display time 
}
void clockLostHandler(char *message) {
  oledDisplay->blinkDateTime(true);  // Display time 
}

// message has a first header line, then text of message:
// "REC UNREAD","+33627333734","","19/09/24,15:29:14+08"
// Ventilo:0
void smsReceivedHandler(char *fullMessage) {
  char *copie = strdup(fullMessage);
  int size = MAX_MSG_LENGTH + 1;
  char *firstLine;
  char *message;
  char *number;
  char *date;
  
  char* eol = strstr(copie, "\r\n");
  if(eol == NULL) {
    Debug("Can't process this message, not on 2 lines.");
    return;
  }  
  *eol = 0;
  firstLine = copie;  
  Serial.println("got");
  Serial.println(firstLine);
  message = (char *)(eol + 2);  // skip \r\n , message points to the second line
  Serial.println(message);

  char* sep = "\",\"";
  number = strstr(firstLine, sep) + strlen(sep);
  char* end = strstr(number, sep);
  if(end == NULL) {
    Debug("Can't process this message, header format unknown.");
    return;
  } 
  *end = 0; 
  Serial.printf("Number: $%s$, %d\n", number, strlen(number));

  date = end + 2*strlen(sep); // empty field (would not be empty if number in sim card contact book)
  date[strlen(date) - 1] = 0; // remove last double quote
  Serial.printf("Date: $%s$, %d\n", date, strlen(date));

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
