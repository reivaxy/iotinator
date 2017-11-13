
#include <string>
#include <Arduino.h>
#include "gsm.h"
#include "utils.h"
#define MAX_MSG_LENGTH 500

#define CHECK_NETWORK_PERIOD 15000 // 15 seconds
#define CHECK_TIME_PERIOD 15000    // 15 seconds
#define CHECK_SMS_PERIOD 15000     // 15 seconds

#define TIME_OUT CHECK_NETWORK_PERIOD*3 // X seconds with no answer from SIM800. Needs to be more than check period!

unsigned long lastAnswer = 0;

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

void GsmClass::refresh() {
  unsigned now = millis();
  if(isElapsedDelay(now, &_lastCheckConnection, CHECK_NETWORK_PERIOD)) {
    _checkConnection();
  }
  if(isElapsedDelay(now, &_lastCheckTime, CHECK_TIME_PERIOD)) {
    _getTime();
  }
  checkGsm();
}

void GsmClass::_getTime() {
  Serial.println("Get time from gsm");
  _serialSIM800->println("AT+CCLK?");
}

// TODO: this should be called and handled internally, periodically
void GsmClass::_checkConnection() {
  Serial.println("Check gsm network connection");
  _serialSIM800->println("AT+CREG?");
}
void GsmClass::setHandler(GsmEvents event, void (*handler)(char*)) {
  _handlers.insert(handlerPair((GsmEvents)event, (void (*)(char*))handler ));
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
  GsmEvents gsmEvent = NONE;
  *message = 0;
  char resultValue[MAX_MSG_LENGTH + 1];
  resultValue[0] = 0;
  
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
    char *ptr = NULL;
    
    ptr = strstr(message, ": ");
    if (ptr != NULL) {
      *ptr = 0;
      strcpy(resultId, message);
      Serial.println(resultId);
      ptr += 2;
      strcpy(resultValue, ptr);     
      Serial.println(resultValue);
      
      // If message is the result of CREG: connection status
      if (strncmp(resultId, "+CREG", 5) == 0) {
        lastAnswer = millis();
        if (strstr(resultValue, "0,5")) {
          gsmEvent = CONNECTION_ROAMING;
        } else if (strstr(resultValue, "0,1")) {
          gsmEvent = CONNECTION;
        } else {
          gsmEvent = DISCONNECTION;
        }      
      }

      // If message is the result of CCLK: get time result
      if (strncmp(resultId, "+CCLK", 5) == 0) {
        // when datetime is not yet initialised it defaults to "04/01/01..." at least in my SIM module     
        if (resultValue[1] == '0') {    // 1 because double quote is 0
          gsmEvent = DATETIME_NOK;
        } else {
          gsmEvent = DATETIME_OK;
        }     
      }
    }
  }
      
  if (millis() - lastAnswer > CHECK_NETWORK_PERIOD) {
    gsmEvent = TIMEOUT;
    lastAnswer = millis(); // To not process at each gsmRefresh
  }
 
  if (gsmEvent != NONE) {      
    std::pair<handlerMap::iterator, handlerMap::iterator> range;
    range = _handlers.equal_range(gsmEvent); // get iterators on entries with key value gsmEvent
    bool found = false;  
    for(handlerMap::iterator it = range.first; it != range.second; ++it) {
      Serial.print("Found handler for ");
      Serial.println(gsmEvent);
      it->second(resultValue);
      found = true;
    }

    if (!found) {
      Serial.print("Unhandled event: ");
      Serial.println(gsmEvent);
    }
  }    
}