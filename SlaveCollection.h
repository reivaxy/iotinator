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
  SlaveCollection(DisplayClass *oledDisplay, XIOTModule* module);
  Slave* add(const char* name, const char* ip);
  void remove(const char* ip);
  void ping();  // ping every slave
  void reset(); // reset every slave
  int getCount();

protected:
  slaveMap _slaves;
  DisplayClass* _oledDisplay;
  XIOTModule* _module;  
};