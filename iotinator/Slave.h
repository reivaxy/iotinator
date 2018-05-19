/**
 *  Class handling Slave module registered in iotinator master 
 *  Xavier Grosjean 2018
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
#pragma once

#include <XIOTDisplay.h>
#include <XIOTModule.h>
#include <XUtils.h>

#define DEBUG_SLAVE // Uncomment this to enable debug messages over serial port

#ifdef DEBUG_SLAVE
#define Debug(...) Serial.printf(__VA_ARGS__)
#else
#define Debug(...)
#endif

class Slave {
public:
  Slave(const char *name, const char* ip, XIOTModule* module);
  bool ping(); // ping this slave
  void reset(); // reset this slave
  void setName(const char*);
  const char* getName();
  const char* getIP();
  bool getPong();
  void setToRename(bool flag);
  bool getToRename();
  void renameTo(const char* newName);
protected:   

  char _ip[DOUBLE_IP_MAX_LENGTH]; // for modules connected to a slave's AP, store 2 ips
  char _name[NAME_MAX_LENGTH];
  bool _pong = false;  // Is true if last ping was successful
  XIOTModule* _module;
  bool _toRename = false; // if true, module must be renamed 

}; 