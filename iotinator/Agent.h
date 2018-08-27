/**
 *  Class handling Agent module registered in iotinator master
 *  Xavier Grosjean 2018
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
#pragma once

#include <XIOTDisplay.h>
#include <XIOTModule.h>
#include <XUtils.h>

//#define DEBUG_AGENT // Uncomment this to enable debug messages over serial port

#define MIN_PING_PERIOD 30

#ifdef DEBUG_AGENT
#define Debug(...) Serial.printf(__VA_ARGS__)
#else
#define Debug(...)
#endif

class Agent {
public:
  Agent(const char *name, const char* mac, XIOTModule* module);
  ~Agent();
  int8_t ping(); // ping this agent
  bool reset(); // reset this agent
  void setName(const char*);
  const char* getName();
  void setIP(const char*);
  const char* getIP();
  void setUiClassName(const char*);
  const char* getUiClassName();
  const char* getMAC();
  int8_t getConnected();
  void setToRename(bool flag);
  bool getToRename();
  void setHeap(uint32_t heap);
  uint32_t getHeap();
  /**
   * Modules powered by batteries should take advantage of the sleep feature.
   * They should not be 'pinged' by master
   */
  bool getCanSleep();
  void setCanSleep(bool);
  int getPingPeriod();
  void setPingPeriod(int);
  time_t getLastPing();
  void setLastPing(time_t);
  void setCustom(const char*);
  const char* getCustom();
  void renameTo(const char* newName);
  
protected:   

  XIOTModule* _module;
  char _mac[MAC_ADDR_MAX_LENGTH + 1]; // for modules connected to a agent's AP, store 2 ips
  char _ip[DOUBLE_IP_MAX_LENGTH + 1]; // for modules connected to a agent's AP, store 2 ips and separator
  char _name[NAME_MAX_LENGTH + 1];
  char _uiClassName[UI_CLASS_NAME_MAX_LENGTH + 1];
  int8_t _connected = 1;  // New agent is created upon registration, so ping is true of course
  bool _toRename = false; // if true, module must be renamed 
  bool _canSleep = false; // if true, module must not be pinged 
  int _pingPeriod = 0; // default ping period is "do not ping"
  time_t _lastPing = 0;
  uint32_t _heap = 0;
  char * _custom = NULL; // custom data sent by module at registration, dynamicall allocated

}; 