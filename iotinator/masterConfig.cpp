/**
 *  Class to persist the master module configuration data structure to EEPROM 
 *  Xavier Grosjean 2017
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
#include "masterConfig.h"

// TODO: use XIOTConfig as super class. 
MasterConfigClass::MasterConfigClass(unsigned int version, const char* name):XEEPROMConfigClass(version, "iotinator", sizeof(MasterConfigStruct)) {
  setName(name);
  // Initialize the array of RegisteredPhoneNumberClass objects from the data structure
  for(int i = 0; i < MAX_PHONE_NUMBERS; i++) {
    _phoneNumbers[i] = new RegisteredPhoneNumberClass(&(_getDataPtr()->registeredNumbers[i]));
  }  
}

/**
 * Reset the config data structure to the default values.
 * This is done each time the data structure version is different from the one saved in EEPROM
 * NB: version and name are handled by base class 
 */
void MasterConfigClass::initFromDefault() {
  XEEPROMConfigClass::initFromDefault(); // handles version init
  setName(MODULE_NAME);
  // Reset all registered phone numbers
  for(int i = 0; i < MAX_PHONE_NUMBERS; i++) {
    _phoneNumbers[i]->reset();
  }
  MasterConfigStruct* configPtr = _getDataPtr();
  setWebSite(DEFAULT_WEBSITE);
  setApiKey("");
  setNtpServer(DEFAULT_NTP_SERVER);
  configPtr->statPeriod = DEFAULT_STAT_PERIOD;
  configPtr->homeSsid[0] = 0;
  configPtr->homePwd[0] = 0;
  setApSsid(DEFAULT_APSSID);
  setApPwd(DEFAULT_APPWD);
  setHomeSsid("");
  setHomePwd("");
  setGmtOffset(DEFAULT_GMT_HOUR_OFFSSET, DEFAULT_GMT_MIN_OFFSSET);
  setDefaultAPExposition(DEFAULT_AP_EXPOSITION);
}

 /**
  * Set the name in the data structure
  *
  */
 void MasterConfigClass::setName(const char* name) {
   XUtils::safeStringCopy(_getDataPtr()->name, name, NAME_MAX_LENGTH);
 }
 
 /**
  * Get the name from the data structure
  *
  */
 char* MasterConfigClass::getName(void) {
   return _getDataPtr()->name;
 }
 
void MasterConfigClass::setHomeSsid(const char* ssid) {
  XUtils::safeStringCopy(_getDataPtr()->homeSsid, ssid, SSID_MAX_LENGTH);
}
void MasterConfigClass::setHomeSsid(String ssidString) {
  char ssid[SSID_MAX_LENGTH + 1];
  ssidString.toCharArray(ssid, (unsigned int)SSID_MAX_LENGTH);
  setHomeSsid(ssid);
}
void MasterConfigClass::setHomePwd(const char* pwd) {
  XUtils::safeStringCopy(_getDataPtr()->homePwd, pwd, PWD_MAX_LENGTH);
}
void MasterConfigClass::setHomePwd(String pwdString) {
  char pwd[PWD_MAX_LENGTH + 1];
  pwdString.toCharArray(pwd, (unsigned int)PWD_MAX_LENGTH);
  setHomePwd(pwd);
}

void MasterConfigClass::setApSsid(const char* ssid) {
  XUtils::safeStringCopy(_getDataPtr()->apSsid, ssid, SSID_MAX_LENGTH);
}
void MasterConfigClass::setApiKey(const char* apiKey) {
  XUtils::safeStringCopy(_getDataPtr()->apiKey, apiKey, API_KEY_MAX_LENGTH);
}
void MasterConfigClass::setApiKey(String apiKeyString) {
  char apiKey[HOSTNAME_MAX_LENGTH + 1];
  apiKeyString.toCharArray(apiKey, (unsigned int)API_KEY_MAX_LENGTH);
  setApiKey(apiKey);
}
void MasterConfigClass::setWebSite(const char* webSite) {
  XUtils::safeStringCopy(_getDataPtr()->webSite, webSite, HOSTNAME_MAX_LENGTH);
}
void MasterConfigClass::setWebSite(String webSiteString) {
  char webSite[HOSTNAME_MAX_LENGTH + 1];
  webSiteString.toCharArray(webSite, (unsigned int)HOSTNAME_MAX_LENGTH);
  setWebSite(webSite);
}
void MasterConfigClass::setNtpServer(const char* ntpServer) {
  XUtils::safeStringCopy(_getDataPtr()->ntpHostName, ntpServer, HOSTNAME_MAX_LENGTH);
}
void MasterConfigClass::setNtpServer(String ntpServerString) {
  char ntpServer[HOSTNAME_MAX_LENGTH + 1];
  ntpServerString.toCharArray(ntpServer, (unsigned int)HOSTNAME_MAX_LENGTH);
  setNtpServer(ntpServer);
}

void MasterConfigClass::setApSsid(String ssidString) {
  char ssid[SSID_MAX_LENGTH + 1];
  ssidString.toCharArray(ssid, (unsigned int)SSID_MAX_LENGTH);
  setApSsid(ssid);
}
void MasterConfigClass::setApPwd(const char* pwd) {
  XUtils::safeStringCopy(_getDataPtr()->apPwd, pwd, PWD_MAX_LENGTH);
}
void MasterConfigClass::setApPwd(String pwdString) {
  char pwd[PWD_MAX_LENGTH + 1];
  pwdString.toCharArray(pwd, (unsigned int)PWD_MAX_LENGTH);
  setApPwd(pwd);
}
char* MasterConfigClass::getHomeSsid(void) {
   return _getDataPtr()->homeSsid;
}
char* MasterConfigClass::getHomePwd(void) {
   return _getDataPtr()->homePwd;
}

char* MasterConfigClass::getApiKey(void) {
   return _getDataPtr()->apiKey;
}

char* MasterConfigClass::getWebSite(void) {
   return _getDataPtr()->webSite;
}

char* MasterConfigClass::getNtpServer(void) {
   return _getDataPtr()->ntpHostName;
}

// For the first 60 seconds the default AP is opened
char* MasterConfigClass::getApSsid(bool force) {
  if(force || millis() > getDefaultAPExposition())
    return _getDataPtr()->apSsid;
  else 
    return (char *)DEFAULT_APSSID;
}
// For the first 60 seconds the default AP is opened
char* MasterConfigClass::getApPwd(bool force) {
  if(force || millis() > getDefaultAPExposition())
    return _getDataPtr()->apPwd;
  else
    return (char *)DEFAULT_APPWD; 
}

void MasterConfigClass::setDefaultAPExposition(int msDelay) {
  _getDataPtr()->defaultAPExposition = msDelay;
}
int MasterConfigClass::getDefaultAPExposition(void) {
  return _getDataPtr()->defaultAPExposition;
}
  
void MasterConfigClass::setAdminNumber(char *number) {
  _phoneNumbers[0]->setNumber(number);
  _phoneNumbers[0]->setAdmin(true);
}
char* MasterConfigClass::getAdminNumber() {
  return _phoneNumbers[0]->getNumber();
}

void MasterConfigClass::setAdminNumber(String numberString) {
  char number[PHONE_NUMBER_LENGTH + 1];
  numberString.toCharArray(number, (unsigned int)PHONE_NUMBER_LENGTH);
  setAdminNumber(number);
}

void MasterConfigClass::setGmtOffset(int8_t hour, int8_t min) {
  _getDataPtr()->gmtHourOffset = hour;
  _getDataPtr()->gmtMinOffset = min;
} 
int8_t MasterConfigClass::getGmtHourOffset() {
  return _getDataPtr()->gmtHourOffset;
} 
int8_t MasterConfigClass::getGmtMinOffset() {
  return _getDataPtr()->gmtMinOffset;
}

// The home Wifi is configured if its ssid is not an empty string...  
bool MasterConfigClass::isHomeWifiConfigured() {
  if(*getHomeSsid() != 0) {
    return true;
  }
  return false;
}

// The AP is configured if its password is not the default password.
bool MasterConfigClass::isAPInitialized() {
  if(strcmp(_getDataPtr()->apPwd, DEFAULT_APPWD) != 0) {
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
MasterConfigStruct* MasterConfigClass::_getDataPtr(void) {
  return (MasterConfigStruct*)XEEPROMConfigClass::_getDataPtr();
}