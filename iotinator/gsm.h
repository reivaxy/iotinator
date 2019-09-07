#pragma once

#define DISABLE_GSM false

#include <SoftwareSerial.h>
#include <Arduino.h>
#include <XUtils.h>
#include <map>
#include <string>
#undef max  // Because Arduino.h and queue are not compatible otherwise
#undef min  // Because Arduino.h and queue are not compatible otherwise
#include <queue>

enum GsmEvents {NONE, CONNECTION, CONNECTION_ROAMING, DISCONNECTION, DATETIME_OK, DATETIME_NOK, INCOMING_SMS, TIMEOUT, READY_FOR_SMS, READING_SMS};

typedef std::multimap <GsmEvents, void (*)(char*)>  handlerMap;
typedef std::pair <GsmEvents, void (*)(char*)>  handlerPair;


class GsmClass {
public:
  GsmClass(SoftwareSerial* serial, int resetGpio);
  
  void initTimeFromNetwork();
  void readGsm();
  
  bool init();
  void refresh();
  void sendSMS(char* toNumber, const char* message);
  void setHandler(GsmEvents event, void (*)(char*));
  void sendCmd(const char* cmd);
  void sendInitCmd(const char* cmd);
  void setPin(const char* pin);
  void initGsm();
  void sendPin();
protected:
  void _connectionTimeOutHandler(char *message);
  void _checkConnection();
  
  SoftwareSerial* _serialSIM800;
  handlerMap _handlers;  

  int _resetGpio = 0;
  char _pinCode[5] ;
  unsigned long _lastCheckConnection = 0;
  unsigned long _lastCheckSms = 0;
  bool _isConnected = false;
  bool _timeisValid = false;
  bool _isInitialized = false;
  unsigned long _lastConnectionOk = 0;
  
  std::queue<char*> _cmds;
  unsigned long _prevQSize = 0;
  unsigned long _prevInitQSize = 0;

  std::queue<char*> _initCmds;
  bool _waitingForCmdResult = false; 
  unsigned long _lastAnswer = 0;
  
  bool needReset = true;  // when a hard reset is needed
  bool resetting = false; // while reset is being done 
  unsigned long resetLowStart = 0;         // date when reset pin was set to 0
  unsigned long resetLowDuration = 1000;   // how long shoud reset be kept = 0: one second
  unsigned long resetHighStart = 0;        // date when reset was set high
  unsigned long resetHighWait = 4000;      // delay after high before gsm init: 4 seconds
  
};
