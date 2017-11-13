#pragma once

#include <SoftwareSerial.h>
#include <Arduino.h>
#include <map>
#include <string>
#undef max  // Because Arduino.h and queue are not compatible otherwise
#undef min  // Because Arduino.h and queue are not compatible otherwise
#include <queue>

enum GsmEvents {NONE, CONNECTION, CONNECTION_ROAMING, DISCONNECTION, DATETIME_OK, DATETIME_NOK, NEW_SMS, TIMEOUT};

typedef std::multimap <GsmEvents, void (*)(char*)>  handlerMap;
typedef std::pair <GsmEvents, void (*)(char*)>  handlerPair;


class GsmClass {
public:
  GsmClass(SoftwareSerial* serial);
  
  void initTimeFromNetwork();
  void checkGsm();
  
  void init();
  void refresh();
  void sendSMS(char* toNumber, char* message);
  void setHandler(GsmEvents event, void (*)(char*));
  void sendCmd(char* cmd);
protected:
  void _connectionTimeOutHandler(char *message);
  void _checkConnection();
  void _getTime();
  
  SoftwareSerial* _serialSIM800;
  handlerMap _handlers;  

  unsigned long _lastCheckConnection = 0;
  unsigned long _lastCheckTime = 0;
  unsigned long _lastCheckSms = 0;
  bool _isConnected = false;
  bool _timeisValid = false;
  
  std::queue<char*> _cmds;
  bool _waitingForCmdResult = false;   
};
