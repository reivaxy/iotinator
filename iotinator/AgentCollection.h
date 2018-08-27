/**
 *  Class handling Agent module registered in iotinator master
 *  Xavier Grosjean 2018
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
#pragma once
 
#include <XIOTDisplay.h>
#include <XIOTModule.h>
#include <XUtils.h>
#include "Agent.h"
#include <map>

//#define DEBUG_AGENT_COLLECTION // Uncomment this to enable debug messages over serial port

#ifdef DEBUG_AGENT_COLLECTION
#define Debug(...) Serial.printf(__VA_ARGS__)
#else
#define Debug(...)
#endif

// Arbitrary "security" additional buffer size. 
#define LIST_BUFFER_SIZE 100

// must not use char* as key
typedef std::map <std::string, Agent*>  agentMap;
typedef std::pair <std::string, Agent*>  agentPair;

class AgentCollection {
public:
  AgentCollection(XIOTModule* module);
  Agent* add(char* jsonStr);
  Agent* refresh(char* jsonStr);
  void remove(const char* mac);
  void ping();  // ping every agent
  void reset(); // reset every agent
  void list(JsonObject& root, int* customSize);
  int getCount();
  void autoRename(Agent *agent);
  bool nameAlreadyExists(const char* name, const char* mac);
  void renameAgent(const char* agentIp, const char* newName);
  
protected:
  agentMap _agents;
  XIOTModule* _module;
  int _listBufferSize = LIST_BUFFER_SIZE;
  void _refreshListBufferSize();
  int _jsonAttributeSize(int moduleCount, const char *attrName, int valueSize);  
};