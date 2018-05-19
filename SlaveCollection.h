/**
 *  Class handling Slave module registered in iotinator master 
 *  Xavier Grosjean 2018
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
#pragma once
 
#include <XIOTDisplay.h>
#include <XIOTModule.h>
#include <XUtils.h>
#include "Slave.h"
#include <map>

#define DEBUG_SLAVE_COLLECTION // Uncomment this to enable debug messages over serial port

#ifdef DEBUG_SLAVE_COLLECTION
#define Debug(...) Serial.printf(__VA_ARGS__)
#else
#define Debug(...)
#endif

// must not use char* as key
typedef std::map <std::string, Slave*>  slaveMap;
typedef std::pair <std::string, Slave*>  slavePair;

class SlaveCollection {
public:
  SlaveCollection(XIOTModule* module);
  Slave* add(String jsonStr);
  void remove(const char* ip);
  void ping();  // ping every slave
  void reset(); // reset every slave
  void list(char* buffer, int size);
  int getCount();
  void renameOne(Slave *slave);
  bool alreadyExists(const char* name, const char* ip);

protected:
  slaveMap _slaves;
  XIOTModule* _module;  
};