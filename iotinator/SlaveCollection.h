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

//#define DEBUG_SLAVE_COLLECTION // Uncomment this to enable debug messages over serial port

#ifdef DEBUG_SLAVE_COLLECTION
#define Debug(...) Serial.printf(__VA_ARGS__)
#else
#define Debug(...)
#endif

// Arbitrary "security" additional buffer size. 
#define LIST_BUFFER_SIZE 100

// must not use char* as key
typedef std::map <std::string, Slave*>  slaveMap;
typedef std::pair <std::string, Slave*>  slavePair;

class SlaveCollection {
public:
  SlaveCollection(XIOTModule* module);
  Slave* add(char* jsonStr);
  Slave* refresh(char* jsonStr);
  void remove(const char* mac);
  void ping();  // ping every slave
  void reset(); // reset every slave
  char *list();
  int getCount();
  void renameOne(Slave *slave);
  bool nameAlreadyExists(const char* name, const char* mac);

protected:
  slaveMap _slaves;
  XIOTModule* _module;
  int _listBufferSize;
  void _refreshListBufferSize();
  int _jsonAttributeSize(int moduleCount, const char *attrName, int valueSize);  
};