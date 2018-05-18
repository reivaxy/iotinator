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
  return _name;
}

const char* Slave::getIP() {
  return _ip;
}

bool Slave::getPong() {
  return _pong;
}

void Slave::setNeedRename(bool flag) {
  _needRename = flag;
}

bool Slave::ping() {
  Debug("Slave::ping\n");
  int httpCode;
  _pong = false;
  _module->APIGet(_ip, "/api/ping", &httpCode);
  return (httpCode == 200);
}
