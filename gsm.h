#pragma once

#include <SoftwareSerial.h>
#include <Arduino.h>
#include <map>
#include <string>

typedef std::multimap <std::string, void (*)(char*)>  handlerMap;
typedef std::pair <std::string, void (*)(char*)>  handlerPair;


class GsmClass {
public:
  GsmClass(SoftwareSerial* serial);
  
  void initTimeFromNetwork();
  void checkGsm();
 
  void setConnectionHandler(void (*handler)(char *));
  void setClockHandler(void (*handler)(char *));
  void setSmsReceivedHandler(void (*handler)(char *));
  
  void init();
  void refresh();
  void sendSMS(char* toNumber, char* message);
  void setHandler(const char* key, void (*)(char*));
  
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
     
};
