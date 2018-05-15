/**
 *  Class to persist the master module configuration data structure to EEPROM 
 *  Xavier Grosjean 2017
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
#include <Arduino.h>

#include "masterConfig.h"
#include "utils.h"

MasterConfigClass::MasterConfigClass(unsigned int version, const char* name, void* dataPtr):XEEPROMConfigClass(version, name, dataPtr, sizeof(masterConfigDataType)) {
  // Initialize the array of RegisteredPhoneNumberClass objects from the data structure
  for(int i = 0; i < MAX_PHONE_NUMBERS; i++) {
    _phoneNumbers[i] = new RegisteredPhoneNumberClass(&(_getConfigPtr()->registeredNumbers[i]));
  }  
}

/**
 * Reset the config data structure to the default values.
 * This is done each time the data structure version is different from the one saved in EEPROM
 * NB: version and name are handled by base class 
 */
void MasterConfigClass::initFromDefault() {
  XEEPROMConfigClass::initFromDefault(); // handles version and name init
  
  // Reset all registered phone numbers
  for(int i = 0; i < MAX_PHONE_NUMBERS; i++) {
    _phoneNumbers[i]->reset();
  }
  masterConfigDataType* configPtr = _getConfigPtr();
  safeStringCopy(configPtr->webAppHost, DEFAULT_WEBAPP_HOST, HOSTNAME_MAX_LENGTH);
  configPtr->statPeriod = DEFAULT_STAT_PERIOD;
  configPtr->homeSsid[0] = 0;
  configPtr->homePwd[0] = 0;
  setApSsid(DEFAULT_APSSID);
  setApPwd(DEFAULT_APPWD);
  // TODO : reset Home Wifi
  setHomeSsid("");
  setHomePwd("");
  setGmtOffset(DEFAULT_GMT_HOUR_OFFSSET, DEFAULT_GMT_MIN_OFFSSET);

}

void MasterConfigClass::setHomeSsid(const char* ssid) {
  safeStringCopy(_getConfigPtr()->homeSsid, ssid, SSID_MAX_LENGTH);
}
void MasterConfigClass::setHomeSsid(String ssidString) {
  char ssid[SSID_MAX_LENGTH];
  ssidString.toCharArray(ssid, (unsigned int)SSID_MAX_LENGTH);
  setHomeSsid(ssid);
}
void MasterConfigClass::setHomePwd(const char* pwd) {
  safeStringCopy(_getConfigPtr()->homePwd, pwd, PWD_MAX_LENGTH);
}
void MasterConfigClass::setHomePwd(String pwdString) {
  char pwd[PWD_MAX_LENGTH];
  pwdString.toCharArray(pwd, (unsigned int)PWD_MAX_LENGTH);
  setHomePwd(pwd);
}

void MasterConfigClass::setApSsid(const char* ssid) {
  safeStringCopy(_getConfigPtr()->apSsid, ssid, SSID_MAX_LENGTH);
}
void MasterConfigClass::setApSsid(String ssidString) {
  char ssid[SSID_MAX_LENGTH];
  ssidString.toCharArray(ssid, (unsigned int)SSID_MAX_LENGTH);
  setApSsid(ssid);
}
void MasterConfigClass::setApPwd(const char* pwd) {
  safeStringCopy(_getConfigPtr()->apPwd, pwd, PWD_MAX_LENGTH);
}
void MasterConfigClass::setApPwd(String pwdString) {
  char pwd[PWD_MAX_LENGTH];
  pwdString.toCharArray(pwd, (unsigned int)PWD_MAX_LENGTH);
  setApPwd(pwd);
}
char* MasterConfigClass::getHomeSsid(void) {
   return _getConfigPtr()->homeSsid;
}
char* MasterConfigClass::getHomePwd(void) {
   return _getConfigPtr()->homePwd;
}
char* MasterConfigClass::getApSsid(void) {
  return _getConfigPtr()->apSsid;
}
char* MasterConfigClass::getApPwd(void) {
  return _getConfigPtr()->apPwd;
}

void MasterConfigClass::setAdminNumber(char *number) {
  _phoneNumbers[0]->setNumber(number);
  _phoneNumbers[0]->setAdmin(true);
}
char* MasterConfigClass::getAdminNumber() {
  return _phoneNumbers[0]->getNumber();
}

void MasterConfigClass::setAdminNumber(String numberString) {
  char number[PHONE_NUMBER_LENGTH];
  numberString.toCharArray(number, (unsigned int)PHONE_NUMBER_LENGTH);
  setAdminNumber(number);
}

void MasterConfigClass::setGmtOffset(int8_t hour, int8_t min) {
  _getConfigPtr()->gmtHourOffset = hour;
  _getConfigPtr()->gmtMinOffset = min;
} 
int8_t MasterConfigClass::getGmtHourOffset() {
  return _getConfigPtr()->gmtHourOffset;
} 
int8_t MasterConfigClass::getGmtMinOffset() {
  return _getConfigPtr()->gmtMinOffset;
}

  
bool MasterConfigClass::homeWifiConfigured() {
  if(*getHomeSsid() != 0) {
    return true;
  }
  return false;
}

bool MasterConfigClass::isInitialized() {
  if(strcmp(getApPwd(), DEFAULT_APPWD) != 0) {
    return true;
  }
  return false;
}
/**
 * Return the phoneNumber object stored at a given offset
 *
 */
RegisteredPhoneNumberClass* MasterConfigClass::getRegisteredPhone(unsigned int offset) {
  return _phoneNumbers[offset];
} 

/**
 * Return the phoneNumber object stored in the config data structure that matches a given number
 * or null if not found
 *
 */
RegisteredPhoneNumberClass* MasterConfigClass::getRegisteredPhoneByNumber(const char* number) {
  for(unsigned int i = 0; i < MAX_PHONE_NUMBERS; i++) {
    if(strcmp(number, _phoneNumbers[i]->getNumber()) == 0) {
      return _phoneNumbers[i];
    }
  }
  return NULL;
}

/**
 * Return the typed data structure object
 *
 */
masterConfigDataType* MasterConfigClass::_getConfigPtr(void) {
  return (masterConfigDataType*)getData();
}
