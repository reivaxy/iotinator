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
Slave::Slave(const char* name, const char* ip, XIOTModule* module) {
  XUtils::safeStringCopy(_name, name, NAME_MAX_LENGTH);
  XUtils::safeStringCopy(_ip, ip, NAME_MAX_LENGTH);
  _module = module;
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

void Slave::renameTo(const char* newName) {
  Debug("Slave::renameTo %s\n", getIP());
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
  return (httpCode == 200);
}
