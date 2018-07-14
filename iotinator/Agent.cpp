/**
 *  Class handling Agent module registered in iotinator master
 *  Xavier Grosjean 2018
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */

#include "Agent.h"

/*********************************************************************
 * Class to handle one agent module in master
 *
 *********************************************************************/
Agent::Agent(const char* name, const char* mac, XIOTModule* module) {
  XUtils::safeStringCopy(_name, name, NAME_MAX_LENGTH);
  XUtils::safeStringCopy(_mac, mac, NAME_MAX_LENGTH);
  _module = module;
}

Agent::~Agent() {
  free(_custom);
}

const char* Agent::getName() {
  Debug("Agent::getName\n");
  return _name;
}
void Agent::setName(const char* name) {
  XUtils::safeStringCopy(_name, name, NAME_MAX_LENGTH);
}

const char* Agent::getIP() {
  Debug("Agent::getIP\n");
  return _ip;
}

const char* Agent::getMAC() {
  Debug("Agent::getMAC\n");
  return _mac;
}

void Agent::setIP(const char* ip) {
  XUtils::safeStringCopy(_ip, ip, DOUBLE_IP_MAX_LENGTH);
}

void Agent::setUiClassName(const char* uiClassName) {
  XUtils::safeStringCopy(_uiClassName, uiClassName, UI_CLASS_NAME_MAX_LENGTH);
}
const char* Agent::getUiClassName() {
  return _uiClassName;
}

bool Agent::getPong() {
  return _pong;
}

void Agent::setToRename(bool flag) {
  _toRename = flag;
}

bool Agent::getToRename() {
  return _toRename;
}

void Agent::setHeap(uint32_t heap) {
  _heap = heap;
}

uint32_t Agent::getHeap() {
  return _heap;
}

bool Agent::getCanSleep() {
  return _canSleep;
}

void Agent::setCanSleep(bool canSleep) {
  _canSleep = canSleep;
}

int Agent::getPingPeriod() {
  return _pingPeriod;
}

void Agent::setPingPeriod(int pingPeriod) {
  // If value too small, keep default (legacy: when absent, value is 0)
  if(pingPeriod >= MIN_PING_PERIOD ) {
    _pingPeriod = pingPeriod;
  }
}
int Agent::getLastPing() {
  return _lastPing;
}

void Agent::setLastPing(int timestamp) {
  _lastPing = timestamp;
}

void Agent::setCustom(const char *custom) {
  Debug("Agent::setCustom\n");
  free(_custom); // This field is manually allocated, so it must be freed.
  _custom = NULL;

  if(custom == NULL) {
    _custom = NULL;
    return;
  }
  int size = strlen(custom);
  if(size > MAX_CUSTOM_DATA_SIZE) {
    Serial.println(CUSTOM_DATA_TOO_BIG_VALUE);
    size = strlen(CUSTOM_DATA_TOO_BIG_VALUE) ;
    _custom = (char *)malloc(size + 1);   // Will need to be freed, when we unregister agent, or overwrite one...
    XUtils::safeStringCopy(_custom, CUSTOM_DATA_TOO_BIG_VALUE, size);
  } else {
    _custom = (char *)malloc(size + 1);   // Will need to be freed, when we unregister agent, or overwrite one...
    XUtils::safeStringCopy(_custom, custom, size);
  }
}

const char* Agent::getCustom() {
  Debug("Agent::getCustom\n");
  if(_custom != NULL) {
    if(strcmp(_custom, CUSTOM_DATA_TOO_BIG_VALUE) == 0) {
      Serial.println(CUSTOM_DATA_TOO_BIG_VALUE);
      _module->getDisplay()->setLine(1, "Custom Data too big", TRANSIENT, NOT_BLINKING);
      _module->getDisplay()->setLine(2, getName(), TRANSIENT, NOT_BLINKING);
    }
  }
  return _custom;
}

void Agent::renameTo(const char* newName) {
  Debug("Agent::renameTo %s\n", getIP());  // ip is easier for debug since displayed on agents
  int httpCode;
  char renameMsg[101];
  const int bufferSize = JSON_OBJECT_SIZE(1);
  StaticJsonBuffer<bufferSize> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root[XIOTModuleJsonTag::name] = newName ;
  root.printTo(renameMsg, 100);
  Serial.printf("Renaming payload: %s\n", renameMsg);   
  _module->APIPost(getIP(), "/api/rename", renameMsg, &httpCode);
  if(httpCode != 200) {
    _module->getDisplay()->setLine(1, "Renaming failed", TRANSIENT, NOT_BLINKING);
    setName(newName);
  } else {
    sprintf(renameMsg, "Renamed %s", newName);
    _module->getDisplay()->setLine(1, renameMsg, TRANSIENT, NOT_BLINKING);
    _toRename = false;
    setName(newName);
  }  
}

bool Agent::ping() {
  Debug("Agent::ping\n");
  int httpCode;
  _pong = false;
  int resultSize = 100 + MAX_CUSTOM_DATA_SIZE; 
  char resultPayload[resultSize];
  _module->APIGet(getIP(), "/api/ping", &httpCode, resultPayload, resultSize);  
  _pong = (httpCode == 200);
  if(_pong) {
    const int bufferSize = JSON_OBJECT_SIZE(2);  // At most 2 fields in one object
    StaticJsonBuffer<bufferSize> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(resultPayload);
    int heap = root[XIOTModuleJsonTag::heap];
    Serial.printf("Heap module %s: %d\n", getName(), heap);
    setHeap(heap);
    Debug("Custom: %s\n", (const char *)root[XIOTModuleJsonTag::custom]);
    setCustom(root[XIOTModuleJsonTag::custom]);  
  }
  return _pong;
}

bool Agent::reset() {
  Debug("Agent::reset\n");
  int httpCode;
  _module->APIGet(getIP(), "/api/moduleReset", &httpCode);  
  return (httpCode == 200);
}
