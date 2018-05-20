/**
 *  Class handling Slave module registered in iotinator master 
 *  Xavier Grosjean 2018
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */

#include "Slave.h" 

/*********************************************************************
 * Class to handle one slave module in master
 *
 *********************************************************************/
Slave::Slave(const char* name, const char* mac, XIOTModule* module) {
  XUtils::safeStringCopy(_name, name, NAME_MAX_LENGTH);
  XUtils::safeStringCopy(_mac, mac, NAME_MAX_LENGTH);
  _module = module;
}

Slave::~Slave() {
  if(_custom != NULL) free(_custom);
}

const char* Slave::getName() {
  Debug("Slave::getName\n");
  return _name;
}
void Slave::setName(const char* name) {
  XUtils::safeStringCopy(_name, name, NAME_MAX_LENGTH);
}

const char* Slave::getIP() {
  Debug("Slave::getIP\n");
  return _ip;
}

const char* Slave::getMAC() {
  Debug("Slave::getMAC\n");
  return _mac;
}

void Slave::setIP(const char* ip) {
  XUtils::safeStringCopy(_ip, ip, DOUBLE_IP_MAX_LENGTH);
}

void Slave::setUiClassName(const char* uiClassName) {
  XUtils::safeStringCopy(_uiClassName, uiClassName, UI_CLASS_NAME_MAX_LENGTH);
}
const char* Slave::getUiClassName() {
  return _uiClassName;
}

bool Slave::getPong() {
  return _pong;
}

void Slave::setToRename(bool flag) {
  _toRename = flag;
}

bool Slave::getToRename() {
  return _toRename;
}

bool Slave::getCanSleep() {
  return _canSleep;
}

void Slave::setCanSleep(bool canSleep) {
  _canSleep = canSleep;
}

void Slave::setCustom(const char *custom) {
  Debug("Slave::setCustom\n");
  if(_custom != NULL) {
    free(_custom); // This field is manually allocated, so it must be freed.
    _custom = NULL;
  }
  if(custom == NULL) {
    _custom = NULL;
    return;
  }
  int size = strlen(custom);
  if(size > MAX_CUSTOM_DATA_SIZE) {
    Serial.println(CUSTOM_DATA_TOO_BIG_VALUE);
    size = strlen(CUSTOM_DATA_TOO_BIG_VALUE) ;
    _custom = (char *)malloc(size + 1);   // Will need to be freed, when we unregister slave, or overwrite one...
    XUtils::safeStringCopy(_custom, CUSTOM_DATA_TOO_BIG_VALUE, size);
  } else {
    _custom = (char *)malloc(size + 1);   // Will need to be freed, when we unregister slave, or overwrite one...
    XUtils::safeStringCopy(_custom, custom, size);
  }
}

const char* Slave::getCustom() {
  Debug("Slave::getCustom\n");
  if(_custom != NULL) {
    if(strcmp(_custom, CUSTOM_DATA_TOO_BIG_VALUE) == 0) {
      Serial.println(CUSTOM_DATA_TOO_BIG_VALUE);
      _module->getDisplay()->setLine(1, "Custom Data too big", TRANSIENT, NOT_BLINKING);
      _module->getDisplay()->setLine(2, getName(), TRANSIENT, NOT_BLINKING);
    }
  }
  return _custom;
}

void Slave::renameTo(const char* newName) {
  Debug("Slave::renameTo %s\n", getIP());  // ip is easier for debug since displayed on slaves
  int httpCode;
  char renameMsg[101];
  StaticJsonBuffer<100> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root[XIOTModuleJsonTag::name] = newName ;
  root.printTo(renameMsg, 100);   
  _module->APIPost(getIP(), "/api/rename", renameMsg, &httpCode);
  if(httpCode != 200) {
    Serial.println("Renaming failed");
    _module->getDisplay()->setLine(1, "Renaming failed", TRANSIENT, NOT_BLINKING);
    setName(newName);
  } else {
    sprintf(renameMsg, "Renamed %s", newName);
    _module->getDisplay()->setLine(1, renameMsg, TRANSIENT, NOT_BLINKING);
    _toRename = false;
    setName(newName);
  }  
}

bool Slave::ping() {
  Debug("Slave::ping\n");
  int httpCode;
  _pong = false;
  _module->APIGet(getIP(), "/api/ping", &httpCode);  
  _pong = (httpCode == 200);
  return _pong;
}
