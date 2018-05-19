
#include <string>
#include <Arduino.h>
#include "gsm.h"
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

bool GsmClass::init() {
  if (DISABLE_GSM) {
    Serial.println("GSM disabled.");
    return false;
  }
  //sendCmd("ATE 0");      // No echo
  sendCmd("AT+CMGF=10");    // Set Text mode (before connection ? check if ok)
  sendCmd("AT+CLTS=1");    // Get local time stamp
  sendCmd("AT+COPS=0");    // Disconnect
  sendCmd("AT+COPS=2");    // Connect
  return true;
}

/**
 * Push a new command in the command queue
 *
 */ 
void GsmClass::sendCmd(const char* cmd) {
  if (DISABLE_GSM) return;
  char* newCmd = (char *)malloc(strlen(cmd) + 1);
  strcpy(newCmd, cmd);
  _cmds.push(newCmd);
}

void GsmClass::refresh() {
  if (DISABLE_GSM) return;
  unsigned now = millis();
  // If connection check delay is elapsted, check the connection state
  if(XUtils::isElapsedDelay(now, &_lastCheckConnection, CHECK_NETWORK_PERIOD)) {
    _checkConnection();
  }
  if(XUtils::isElapsedDelay(now, &_lastCheckTime, CHECK_TIME_PERIOD)) {
    _getTime();
  }
  
  // if not already waiting for a command result and command queue not empty, send command
  if (!_waitingForCmdResult && !_cmds.empty()) {
    char* cmd = _cmds.front();
    _serialSIM800->println(cmd);
    _waitingForCmdResult = true;
    _cmds.pop();
    free(cmd);    
  }
  // check gsm serial line for incoming stuff
  checkGsm();
}

void GsmClass::_getTime() {
  if (DISABLE_GSM) return;
  Serial.println("Get time from gsm");
  sendCmd("AT+CCLK?");
}

// TODO: this should be called and handled internally, periodically
void GsmClass::_checkConnection() {
  if (DISABLE_GSM) return;
  Serial.println("Check gsm network connection");
  sendCmd("AT+CREG?");
}
void GsmClass::setHandler(GsmEvents event, void (*handler)(char*)) {
  if (DISABLE_GSM) return;
  _handlers.insert(handlerPair((GsmEvents)event, (void (*)(char*))handler ));
}

void GsmClass::sendSMS(char* toNumber, const char* msg) {
  if (DISABLE_GSM) return;
  char message[MAX_MSG_LENGTH + 1];
  Serial.print("Sending SMS to ");
  Serial.println(toNumber);
  char sendToNum[50];
  sendCmd("AT+CSCS=\"GSM\"");
  sprintf(sendToNum, "AT+CMGS=\"%s\"", toNumber);
  sendCmd(sendToNum);
  strncpy(message, msg, MAX_MSG_LENGTH);
  strcat(message, "\x1A");
  sendCmd(message); 
}

void GsmClass::checkGsm() {
  if (DISABLE_GSM) return;
  int incomingChar, length;
  char message[MAX_MSG_LENGTH + 1];
  GsmEvents gsmEvent = NONE;
  *message = 0;
  char resultValue[MAX_MSG_LENGTH + 1];
  resultValue[0] = 0;
  
  while(_serialSIM800->available()){    
    incomingChar = _serialSIM800->read();
    if(incomingChar > 0) {
      // When 'cr' is detected, process received message
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
    Serial.print("$");
    Serial.print(message);
    Serial.println("$");
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
    } else {
      if ((strncmp(message, "OK", 2) == 0) || (strncmp(message, ">", 1) == 0)) {
        lastAnswer = millis();
        // Ready to send the next command in queue (if any)
        _waitingForCmdResult = false;      
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
