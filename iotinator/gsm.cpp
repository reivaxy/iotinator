
#include <string>
#include <Arduino.h>
#include "gsm.h"
#define MAX_MSG_LENGTH 500

#define CHECK_NETWORK_PERIOD 15000 // 15 seconds
#define CHECK_TIME_PERIOD 15000    // 15 seconds
#define CHECK_SMS_PERIOD 15000     // 15 seconds
#define RESET_GSM_AFTER 60000 * 2  // Reset GSM after 2 minutes of disconnection (bad CREG response) 

#define TIME_OUT CHECK_NETWORK_PERIOD*3 // X seconds with no answer from SIM800. Needs to be more than check period!


// TODO handle a different fifo for gsm commands to store them while connection is not done ??
GsmClass::GsmClass(SoftwareSerial* serial, int resetGpio) {
  _serialSIM800 = serial;
  _resetGpio = resetGpio;
};

void GsmClass::setPin(const char* pin) {
  strlcpy(_pinCode, pin, 5);
}

bool GsmClass::init() {  
  if (DISABLE_GSM) {
    Serial.println("GSM disabled.");
    return false;
  }
  _serialSIM800->begin(9600);
  Serial.println("GSM enabled.");
  initGsm();
  return true;
}

void GsmClass::initGsm() {
  if (DISABLE_GSM) {
    return;
  }
  Serial.println("Initializing GSM");
  _lastConnectionOk = millis();
  pinMode(_resetGpio, OUTPUT);   
  digitalWrite(_resetGpio, LOW);
  delay(1000);
  digitalWrite(_resetGpio, HIGH);
  delay(5000);
  _isInitialized = true;
  sendInitCmd("AT");  // This allows initializing uart on sim board (rate...)
  sendInitCmd("");    // get rid of garbage characters (no command but wait for response) 
  //sendInitCmd("ATE0");
  sendPin();
}

void GsmClass::sendPin() {
  // Send PIN code
  char message[20];
  sprintf(message, "AT+CPIN=\"%s\"", _pinCode);
  sendInitCmd(message);
}

/**
 * Push a new command in the init command queue
 * 
 */ 
void GsmClass::sendInitCmd(const char* cmd) {
  if (DISABLE_GSM || !_isInitialized) return;
  char* newCmd = (char *)malloc(strlen(cmd) + 1);
  strcpy(newCmd, cmd);
  _initCmds.push(newCmd);
}

/**
 * Push a new command in the command queue
 *
 */ 
void GsmClass::sendCmd(const char* cmd) {
  if (DISABLE_GSM || !_isInitialized) return;
  char* newCmd = (char *)malloc(strlen(cmd) + 1);
  strcpy(newCmd, cmd);
  _cmds.push(newCmd);
}

void GsmClass::refresh() {
  if (DISABLE_GSM || !_isInitialized) return;
  // check gsm serial line for incoming stuff
  readGsm();
  unsigned now = millis();
  // If connection check delay is elapsted, check the connection state
  if(XUtils::isElapsedDelay(now, &_lastCheckConnection, CHECK_NETWORK_PERIOD)) {
    _checkConnection();
  }
  
  std::queue<char*> *queueToUse = &_cmds;
  long unsigned int *prevQSizeToUse = &_prevQSize; 
  if (_initCmds.size() > 0) {
    queueToUse = &_initCmds;
    prevQSizeToUse = &_prevInitQSize; 
  }
  // if not already waiting for a command result and command queue not empty, send command
  if(queueToUse->size() != *prevQSizeToUse) {
    *prevQSizeToUse = queueToUse->size();
    Serial.print("Queue size: ");
    Serial.println(*prevQSizeToUse);
  }
  if (!_waitingForCmdResult && !queueToUse->empty()) {
    char* cmd = queueToUse->front();
    Serial.print("GSM cmd: ");
    Serial.println(cmd);
    _serialSIM800->println(cmd);
    _waitingForCmdResult = true;
    queueToUse->pop();
    free(cmd);    
  }

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

void GsmClass::readGsm() {
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
    char resultId[20];
    char *ptr = NULL;
    
    ptr = strstr(message, ": ");
    if (ptr != NULL) {
      *ptr = 0;
      strcpy(resultId, message);  // resultId contains the response prefix (like +CMGR" for instance)
      Serial.println(resultId);
      ptr += 2;
      strcpy(resultValue, ptr);     
      Serial.println(resultValue);
      
      // If message is the result of CREG: connection status
      
      if (strncmp(resultId, "+CREG", 5) == 0) {
        if (strstr(resultValue, "0,5")) {
          gsmEvent = CONNECTION_ROAMING;
          _lastConnectionOk = millis();
        } else if (strstr(resultValue, "0,1")) {
          gsmEvent = CONNECTION;
          _lastConnectionOk = millis();
        } else {
          sendPin();
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
      // incoming SMS 
      if (strncmp(resultId, "+CMTI", 5) == 0) {
        gsmEvent = INCOMING_SMS;      
      }
      // response to read message command: need to read incoming message until empty line and then 'OK' alone on a line
      if (strncmp(resultId, "+CMGR", 5) == 0) {
        gsmEvent = READING_SMS;      
      }
    } else {
      if ((strncmp(message, "OK", 2) == 0) || (strncmp(message, ">", 1) == 0)) {
        _lastAnswer = millis();
        // Ready to send the next command in queue (if any)
        _waitingForCmdResult = false;      
      }
      // Message sent by SIM800 when ready for SMS (not a specific command response)
      if (strncmp(message, "SMS Ready", 9) == 0) {
        gsmEvent = READY_FOR_SMS;      
      }      
    }
  }
      
  if (_waitingForCmdResult && (millis() - _lastAnswer > CHECK_NETWORK_PERIOD)) {
    gsmEvent = TIMEOUT;
    _lastAnswer = millis(); // To not process at each gsmRefresh
     _waitingForCmdResult = false;      
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
  // If connection check delay is elapsted, check the connection state
  if(XUtils::isElapsedDelay(millis(), &_lastConnectionOk, RESET_GSM_AFTER)) {
    initGsm();
  }    
}
