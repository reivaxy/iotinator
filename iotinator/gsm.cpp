
#include <string>
#include <Arduino.h>
#include "gsm.h"

#define CHECK_NETWORK_PERIOD 30000 // in ms
#define RESET_GSM_AFTER 60000 * 2  // Reset GSM after x ms of disconnection (bad CREG response) 

#define SEND_INTERVAL_DELAY 150  // Delay after sending a command to SIM800 (solves lag when sending message after reading one)

#define RESET_LOW_DURATION 2000 // how long should SIM800 reset pin be kept low, in ms
#define RESET_HIGH_WAIT 2000    // delay after high before gsm init in ms

#define GSM_CMD_TIMEOUT 4000

// TODO handle a different fifo for gsm commands to store them while connection is not done ??
GsmClass::GsmClass(SoftwareSerial* serial, int resetGpio) {
  _serialSIM800 = serial;
  _resetGpio = resetGpio;
  if (!DISABLE_GSM) {
    _serialSIM800->begin(9600);
    pinMode(_resetGpio, OUTPUT);
  }   
};

void GsmClass::setPin(const char* pin) {
  strlcpy(_pinCode, pin, 5);
}

bool GsmClass::init() {  
  if (DISABLE_GSM) {
    Serial.println("GSM disabled.");
    return false;
  }
  Serial.println("GSM enabled.");
  return true;
}

void GsmClass::_resetIfNeeded() {
  if(!_needReset && !_resetting) return;
  if(_resetting) {
    if(_resetLowStart != 0 && XUtils::isElapsedDelay(millis(), &_resetLowStart, RESET_LOW_DURATION)) {
      Debug("Setting gsm reset pin: high\n");
      digitalWrite(_resetGpio, HIGH); 
      _resetLowStart = 0;
      _resetHighStart = millis();   
    }
    if(_resetHighStart != 0 && XUtils::isElapsedDelay(millis(), &_resetHighStart, RESET_HIGH_WAIT)) {
      _resetHighStart = 0;
      _resetting = false;    
      Debug("GSM hard reset done\n");
      initGsm();
    }
  }
  if(_needReset) {
    Debug("Setting gsm reset pin: low\n");
    _resetting = true;
    _needReset = false;
    digitalWrite(_resetGpio, LOW);
    _resetLowStart = millis();   
  }
}

void GsmClass::initGsm() {
  if (DISABLE_GSM) {
    return;
  }
  Serial.println("Initializing GSM");

  _lastConnectionOk = millis();
  _isInitialized = true;
  *_smsToProcess = 0;
  sendInitCmd("AT");  // This allows initializing uart parameters on sim board (rate...)
  sendInitCmd("AT");    // get rid of garbage characters (wait for response) 
  sendInitCmd("ATE0");  // no echo
  sendPin();
}

void GsmClass::forceReset() {
  _needReset = true;
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
  char* newCmd = strdup(cmd);
  Debug("Queueing init cmd %s\n", newCmd);
  _initCmds.push(newCmd);
}

/**
 * Push a new command in the command queue
 *
 */ 
void GsmClass::sendCmd(const char* cmd) {
  if (DISABLE_GSM || !_isInitialized) return;
  char* newCmd = strdup(cmd);
  Debug("Queueing cmd %s\n", newCmd);
  _cmds.push(newCmd);
}

void GsmClass::refresh() {
  if (DISABLE_GSM) return;  
  _resetIfNeeded();
  if(!_isInitialized) return; 
   
  unsigned now = millis();
  
  if(*_smsToProcess != 0) {
      // delete read messages 
      sendCmd("AT+CMGD=1,1");
      std::pair<handlerMap::iterator, handlerMap::iterator> range;
      range = _handlers.equal_range(SMS_READ); // get iterators on entries with key value gsmEvent
      for(handlerMap::iterator it = range.first; it != range.second; ++it) {
        it->second(_smsToProcess);
      }
      *_smsToProcess = 0;
      return; // just one at a time
  }
        
  // If connection check delay is elapsted, check the connection state
  if(_isConnected && XUtils::isElapsedDelay(now, &_lastCheckConnection, CHECK_NETWORK_PERIOD)) {
    checkConnection();
  }
  
  std::queue<char*> *queueToUse = &_cmds;
  if (_initCmds.size() > 0) {
    queueToUse = &_initCmds;
  }

  if (!_waitingForCmdResult && !queueToUse->empty() && XUtils::isElapsedDelay(now, &_lastWriteCommand, SEND_INTERVAL_DELAY)) {
    Debug("%s queue size: %d\n",  (queueToUse == &_cmds) ? "Standard": "Init", queueToUse->size());
    char* cmd = queueToUse->front();
    Debug("Writing cmd: %s\n", cmd);
    _serialSIM800->println(cmd);
    _waitingForCmdResult = true;
    _lastCmdSent = millis();
    queueToUse->pop();
    free(cmd);
    _lastWriteCommand = millis(); 
  } 

  // check gsm serial line for incoming stuff
  readGsm();
  
}

void GsmClass::checkConnection() {
  Debug("GsmClass::checkConnection\n");
  if (DISABLE_GSM) return;
  if(_initCmds.size() > 0) return;
  Debug("Check gsm network connection\n");
  sendCmd("AT+CREG?");
}
void GsmClass::setHandler(GsmEvents event, void (*handler)(char*)) {
  if (DISABLE_GSM) return;
  _handlers.insert(handlerPair((GsmEvents)event, (void (*)(char*))handler ));
}

void GsmClass::sendSMS(char* toNumber, const char* msg) {
  Debug("GsmClass::sendSMS\n");
  if (DISABLE_GSM) return;
  
  char message[MAX_MSG_LENGTH + 3];
  Serial.print("Sending SMS to ");
  Serial.println(toNumber);
  char sendToNum[50];
  sendCmd("AT+CMGF=1");
  sendCmd("AT+CSCS=\"GSM\"");
  sprintf(sendToNum, "AT+CMGS=\"%s\"", toNumber);
  sendCmd(sendToNum);
  strlcpy(message, msg, MAX_MSG_LENGTH + 3);
  strlcat(message, "\x1A", MAX_MSG_LENGTH + 3);
  sendCmd(message); 
}

void GsmClass::readGsm() {
  //Debug("GsmClass::readGsm\n");
  if (DISABLE_GSM) return;
  
  int incomingChar, length;
  char message[MAX_MSG_LENGTH + 1];
  GsmEvents gsmEvent = NONE;
  *message = 0;
  char resultValue[MAX_MSG_LENGTH + 1];
  *resultValue = 0;
  
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
          Serial.println("Serial message too big, truncating");
          break;
        }        
      }
    }
  }
  if(strlen(message) > 0) {
    Debug("$%s$\n", message);
    char resultId[20];
    char *ptr = NULL;
    
    ptr = strstr(message, ": ");
    if (ptr != NULL) {
      strlcpy(resultId, message, ptr - message + 1);  // resultId contains the response prefix (like +CMGR" for instance)
      Debug("%s\n", resultId);
      ptr += 2;
      strlcpy(resultValue, ptr, MAX_MSG_LENGTH + 1);     
      Debug("%s\n", resultValue);
      
      // If message is the result of CREG: connection status
      
      if (strncmp(resultId, "+CREG", 5) == 0) {
        if (strstr(resultValue, "0,5")) {
          gsmEvent = CONNECTION_ROAMING;
          _lastConnectionOk = millis();
          _isConnected = true;
        } else if (strstr(resultValue, "0,1")) {
          gsmEvent = CONNECTION;
          _lastConnectionOk = millis();
          _isConnected = true;
        } else {
          gsmEvent = DISCONNECTION;
          _isConnected = false;
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
      // incoming SMS is available to read
      if (strncmp(resultId, "+CMTI", 5) == 0) {
        char *ptr = strstr(resultValue, ",");
        if (ptr != NULL) {
          char msgId[10];
          char readCmd[50];        
          char delCmd[50];        
          strlcpy(msgId, ptr + 1, 10);     
          Serial.print("Reading incoming SMS ");
          Serial.println(msgId);
          sprintf(readCmd, "AT+CMGR=%s", msgId);
          sendCmd("AT+CMGF=1");
          sendCmd("AT+CSCS=\"GSM\"");
          sendCmd(readCmd);
        }
        gsmEvent = INCOMING_SMS;       
      }
      // response to read message command: need to read incoming message until empty line and then 'OK' alone on a line
      if (strncmp(resultId, "+CMGR", 5) == 0) {
        Serial.println("Received SMS");
        // In test mode (CMGF=1) messages are followed by a CR LF only line, and then an "OK" line.
        // Empty lines sent within the message are just LF
        // Read the message: read characters until first CR LF only line.
        int previousChar = 0;
        strcat(resultValue, "\n");
        unsigned long timeOut = millis();
        
        while(true && !XUtils::isElapsedDelay(millis(), &timeOut, 2000)) {    
          incomingChar = _serialSIM800->read();
          if(incomingChar > 0) {
            // did we get an empty line ? current char is CR and previous was LF ?            
            if(incomingChar == 13) {
              if(previousChar == 10) {
                resultValue[strlen(resultValue) - 1] = 0;  // remove LF
                break;
              }
            }
            length = strlen(resultValue);
            if(length < MAX_MSG_LENGTH - 2) {
              resultValue[length] = incomingChar;
              resultValue[length + 1] = 0;
              previousChar = incomingChar;
            } else {
              // Ignore rest of message
              Serial.println("Serial message too big, truncating");
              break;
            }        

          }
        }             
        _readUntil2CharMsg("OK");
        _readUntil2CharMsg("\r\n");
        _waitingForCmdResult = false;
        gsmEvent = SMS_READ;
        Serial.println(resultValue);    
        Serial.println("Not waiting");    
    
      }
    } else {
      if ((strncmp(message, "OK", 2) == 0) || (strncmp(message, ">", 1) == 0)) {
        // Ready to send the next command in queue (if any)
        _waitingForCmdResult = false;
        Serial.println("Not waiting");    
      }
      // Message sent by SIM800 when ready for SMS (not a specific command response)
      if (strncmp(message, "SMS Ready", 9) == 0) {
        gsmEvent = READY_FOR_SMS;      
      }      
    }
  }
      
  if (_waitingForCmdResult && (millis() - _lastCmdSent > GSM_CMD_TIMEOUT)) {
    gsmEvent = TIMEOUT;
    Serial.println("GSM cmd response Timeout");
    _waitingForCmdResult = false;      
  }
 
  if (gsmEvent != NONE) {
    // processing incoming SMS (sending another one) here sometime crashes... Couldn't explain it for now, so 
    // this is a work around: process it at next refresh call.
    if(SMS_READ == gsmEvent) {
      strlcpy(_smsToProcess, resultValue, MAX_MSG_LENGTH + 1);
    } else {
      std::pair<handlerMap::iterator, handlerMap::iterator> range;
      range = _handlers.equal_range(gsmEvent); // get iterators on entries with key value gsmEvent
      bool found = false;  
      for(handlerMap::iterator it = range.first; it != range.second; ++it) {
        Serial.print("Found gsm handler for ");
        Serial.println(gsmEvent);
        it->second(resultValue);
        found = true;
      }
  
      if (!found) {
        Serial.print("Unhandled gsm event: ");
        Serial.println(gsmEvent);
      }
    }      
  }
  // If connection check delay is elapsted, check the connection state
  if(XUtils::isElapsedDelay(millis(), &_lastConnectionOk, RESET_GSM_AFTER)) {
    _needReset = true;
  }    
}

void GsmClass::_readUntil2CharMsg(char *twoCharMsg) {
  int incomingChar = 0;
  int previousChar = 0;
  while(_serialSIM800->available()) {    
    incomingChar = _serialSIM800->read();
//    Serial.println(incomingChar);
    if(incomingChar == twoCharMsg[1] && previousChar == twoCharMsg[0]) {
      break;
    }
    previousChar = incomingChar;
  }       
}
