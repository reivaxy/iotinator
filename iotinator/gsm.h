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

enum GsmEvents {NONE, CONNECTION, CONNECTION_ROAMING, DISCONNECTION, DATETIME_OK, DATETIME_NOK, INCOMING_SMS, TIMEOUT, READY_FOR_SMS, SMS_READ};

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
  void sendCmdNoNl(const char* cmd);
  void sendInitCmd(const char* cmd);
  void setPin(const char* pin);
  void initGsm();
  void sendPin();
  void forceReset();
  void checkConnection();
protected:
  void _connectionTimeOutHandler(char *message);
  void _resetIfNeeded();
  void _readUntil2CharMsg(char *twoCharMsg);
  
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
  unsigned long _lastCmdSent = 0;
  
  bool _needReset = true;  // when a hard reset is needed
  bool _resetting = false; // while reset is being done 
  unsigned long _resetLowStart = 0;         // date when reset pin was set to 0
  unsigned long _resetHighStart = 0;        // date when reset was set high
  
};
