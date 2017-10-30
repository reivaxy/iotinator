

#include "gsm.h"
#define MAX_MSG_LENGTH 300

// TODO fifo for gsm commands ??

GsmClass::GsmClass(SoftwareSerial* serial) {
  _serialSIM800 = serial;
  _serialSIM800->println("ATE 0");
  _serialSIM800->println("AT+CLTS=1");
  _serialSIM800->println("AT+COPS=0");
  _serialSIM800->println("AT+COPS=2");
};

void GsmClass::getTime() {
  Serial.println("Get time from gsm");
  _serialSIM800->println("AT+CCLK?");
}

void GsmClass::checkGsm() {
  Serial.print("check gsm ");
  int incomingChar, length;
  char message[MAX_MSG_LENGTH];
  *message = 0;
  while(_serialSIM800->available()){    
    incomingChar = _serialSIM800->read();
    if(incomingChar > 0) {
      if(incomingChar == 10) {
        break;
      } else {
        length = strlen(message);
        if(length < MAX_MSG_LENGTH - 2) {
          message[length] = incomingChar;
          message[length + 1] = 0;
        } else {
          // Ignore  message
          message[0] = 0;
          Serial.println("Serial message too big");
        }        
      }
    }
  }
  Serial.println(message);
}