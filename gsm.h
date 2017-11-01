#pragma once

#include <SoftwareSerial.h>
#include <Arduino.h>
#include <map>
#include <string>


class GsmClass {
public:
  GsmClass(SoftwareSerial* serial);
  
  void initTimeFromNetwork();
  void getTime();
  void checkGsm();
  void checkNetwork();
 
  void setConnectionHandler(void (*handler)(char *));
  void setClockHandler(void (*handler)(char *));
  void init();
  void sendSMS(char* toNumber, char* message);
  
protected:
  SoftwareSerial* _serialSIM800;
  void (*_connectionHandler)(char *);
  unsigned int _disconnectionTime = 0;
  void _setHandler(const char* key, void (*)(char*));
  std::map <std::string, void (*)(char*)> _handlers;   
};
