/**
 *  Definition of the config data structure for the master module and the class to persist it to EEPROM 
 *  Xavier Grosjean 2017
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
#pragma once

#include "registeredPhoneNumber.h"
#include "EEPROMConfig.h"

#define CONFIG_VERSION 2
#define CONFIG_NAME "Iotinator"

#define HOSTNAME_MAX_LENGTH 50
#define DEFAULT_WEBAPP_HOST "http://www.iotinator.com/"
#define DEFAULT_STAT_PERIOD 1800000 // Half an hour. Should 0 be default ?

#define SSID_MAX_LENGTH 20
#define PWD_MAX_LENGTH 50

// The default Access Point SSID and Password are known and used by slave modules
// to connect the first time.
// Master module needs to open this AP when connecting a new slave, before it returns to
// the customized SSID and PWD (after communicating them to newly connected slave)
// Should be in some common header file...
#define DEFAULT_APSSID "iotinator" 
#define DEFAULT_APPWD "iotinator"
 
typedef struct {
  // First two members version and name are mandatory
  unsigned int version;
  char name[NAME_MAX_LENGTH + 1];
  
  // Array to store phone number information, permissions, ...
  phoneNumberDataType registeredNumbers[MAX_PHONE_NUMBERS];
  // Hostname for webapps, statistics, etc
  char webAppHost[HOSTNAME_MAX_LENGTH + 1];
  // Period at which statistics are sent to web server. 0 means no stat sent
  unsigned int statPeriod = DEFAULT_STAT_PERIOD;
  
  char homeSsid[SSID_MAX_LENGTH + 1];
  char homePwd[PWD_MAX_LENGTH + 1];
    
  char apSsid[SSID_MAX_LENGTH + 1];
  char apPwd[PWD_MAX_LENGTH + 1];
  
} masterConfigDataType;

 
class MasterConfigClass:public EEPROMConfigClass {
public:
  MasterConfigClass(unsigned int version, const char* name, void* dataPtr);
  virtual void initFromDefault(void) override;
  
  RegisteredPhoneNumberClass* getRegisteredPhoneByNumber(const char* number); 
  RegisteredPhoneNumberClass* getRegisteredPhone(unsigned int offset); 
  void removePhone(const char*);
  void addPhone(const char*);
  
  void setHomeSsid(const char* ssid);
  void setHomePwd(const char* pwd);
  void setApSsid(const char* ssid);
  void setApPwd(const char* pwd);
  void setApSsid(String ssid);
  void setApPwd(String pwd);
  char* getHomeSsid(void);
  char* getHomePwd(void);
  char* getApSsid(void);
  char* getApPwd(void);
  void setAdminNumber(char *number);
  void setAdminNumber(String numberString);
  
  
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

