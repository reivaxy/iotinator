
#include <string>
#include <Arduino.h>
#include "gsm.h"
#define MAX_MSG_LENGTH 500

// TODO fifo for gsm commands ??

GsmClass::GsmClass(SoftwareSerial* serial) {
  _serialSIM800 = serial;
};

void GsmClass::init() {
  //_serialSIM800->println("ATE 0");      // No echo
  _serialSIM800->println("AT+CMGF=1");    // Set Text mode (before connection ? check if ok)
  delay(300);
  _serialSIM800->println("AT+CLTS=1");    // Get local time stamp
  delay(500);
  _serialSIM800->println("AT+COPS=0");    // Disconnect
  delay(500);
  _serialSIM800->println("AT+COPS=2");    // Connect
  delay(500);
}

void GsmClass::getTime() {
  Serial.println("Get time from gsm");
  _serialSIM800->println("AT+CCLK?");
}
void GsmClass::checkNetwork() {
  Serial.println("Check gsm network connection");
  _serialSIM800->println("AT+CREG?");
}
void GsmClass::setConnectionHandler(void (*handler)(char *)) {
  setHandler("+CREG", handler);
}
void GsmClass::setClockHandler(void (*handler)(char *)) {
  setHandler("+CCLK", handler);
}
void GsmClass::setSmsReceivedHandler(void (*handler)(char *)) {
  setHandler("+CMTI", handler);
}

void GsmClass::setHandler(const char* key, void (*handler)(char*)) {
  _handlers[key] = handler;
}

void GsmClass::sendSMS(char* toNumber, char* message) {
  Serial.print("Sending SMS to ");
  Serial.println(toNumber);
  _serialSIM800->println("AT+CSCS=\"GSM\"");
  _serialSIM800->print("AT+CMGS=\"");
  _serialSIM800->print(toNumber);
  _serialSIM800->println("\"");
  delay(2000);  // Need to wait for the prompt. Handle this better ?
  _serialSIM800->print(message);
  _serialSIM800->print("\x1A");
  
}

void GsmClass::checkGsm() {
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
  if(strlen(message) > 0) {
    Serial.println(message);
    char resultId[10];
    char resultValue[MAX_MSG_LENGTH + 1];
    char *ptr = NULL;
    
    ptr = strstr(message, ": ");
    if (ptr != NULL) {
      *ptr = 0;
      strcpy(resultId, message);
      Serial.println(resultId);
      ptr += 2;
      strcpy(resultValue, ptr);     
      Serial.println(resultValue);

      if (_handlers.find(resultId) != _handlers.end()) {
        _handlers[resultId](resultValue);
      } else {
        Serial.print("Unhandled response: ");
        Serial.println(resultId);
      }
    }
    
    // If message is the result of CREG
//    if (strncmp(message, "+CREG:", 6) == 0) {
//      _connectionHandler(message);
//    }
  }
}