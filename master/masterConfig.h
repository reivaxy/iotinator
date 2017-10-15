/**
 *  Definition of the config data structure for the master module and the class to persist it to EEPROM 
 *  Xavier Grosjean 2017
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
#pragma once

#include "registeredPhoneNumber.h"
#include "EEPROMConfig.h"

#define CONFIG_VERSION 1
#define CONFIG_NAME "Iotinator"

typedef struct {
  // First two members version and name are mandatory
  unsigned int version;
  char name[NAME_MAX_LENGTH];
  phoneNumberDataType registeredNumbers[MAX_PHONE_NUMBERS];
} masterConfigDataType;

 
class MasterConfigClass:public EEPROMConfigClass {
public:
  MasterConfigClass(unsigned int version, const char* name, void* dataPtr);
  virtual void initFromDefault(void) override;
  
  RegisteredPhoneNumberClass* getRegisteredPhoneByNumber(const char* number); 
  RegisteredPhoneNumberClass* getRegisteredPhone(unsigned int offset); 
  void removePhone(const char*);
  void addPhone(const char*);

protected:
  masterConfigDataType* _getConfigPtr(void);
  RegisteredPhoneNumberClass* _phoneNumbers[MAX_PHONE_NUMBERS];  
};

/*

// Init config structure from EEPROM
// If not in EEPROM or version has changed from what is stored in EEPROM,
// reset config to default values, and save it to EEPROM for next time
void readConfig() {
  unsigned char i;

  unsigned int configSize = sizeof(config);
  unsigned int cptr = 0;
  byte *configPtr = (byte *)&config;
  for(cptr = 0; cptr < configSize; cptr++) {
    *(configPtr ++) = EEPROM.read(cptr);
  }

  if(config.version != CONFIG_VERSION) {
    Serial.println(WIFI_CONFIG_INIT);
    config.version = CONFIG_VERSION;
    strcpy(config.webAppHost, DEFAULT_WEBAPP_HOST);
    strcpy(config.statisticsHost, DEFAULT_STAT_HOST);
    strcpy(config.statisticsPath, DEFAULT_STAT_PATH);
    strcpy(config.logPath, DEFAULT_LOG_PATH);
    strcpy(config.homeSsid, DEFAULT_WIFI_SSID);
    strcpy(config.homePwd, DEFAULT_WIFI_PWD);
    strcpy(config.APSsid, DEFAULT_AP_SSID);
    strcpy(config.APPwd, DEFAULT_AP_PWD);
    strcpy(config.deviceName, "Aquarium");
    config.statisticsInterval = DEFAULT_STAT_INTERVAL; // number of milliseconds between two statistic sending
    config.getMeasuresInterval = DEFAULT_GET_MEASURES_INTERVAL; // number of milliseconds between two arduino msg checks

    // Save the configuration to EEPROM
    Serial.println(WIFI_CONFIG_SAVING);
    configPtr = (byte *)&config;
    for(cptr = 0; cptr < configSize; cptr++) {
      EEPROM.write(cptr, *(configPtr++));
    }
    EEPROM.commit();

    Serial.println(WIFI_CONFIG_SAVED);
  }
}*/

