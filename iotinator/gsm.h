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
  void checkGsm();
  
  bool init();
  void refresh();
  void sendSMS(char* toNumber, const char* message);
  void setHandler(GsmEvents event, void (*)(char*));
  void sendCmd(const char* cmd);
  void setPin(const char* pin);
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
  
  std::queue<char*> _cmds;
  bool _waitingForCmdResult = false;   
};
