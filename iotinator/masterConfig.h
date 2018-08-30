/**
 *  Definition of the config data structure for the master module and the class to persist it to EEPROM 
 *  Xavier Grosjean 2017
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
#pragma once
#include <Arduino.h>
#include "registeredPhoneNumber.h"
#include <XIOTConfig.h>
#include <XUtils.h>

#define CONFIG_VERSION 5
#define MODULE_NAME "iotinator"

#define HOSTNAME_MAX_LENGTH 50
#define API_KEY_MAX_LENGTH 40
#define DEFAULT_WEBSITE "http://www.iotinator.com/"
#define DEFAULT_NTP_SERVER "0.europe.pool.ntp.org"

#define DEFAULT_STAT_PERIOD 1800000 // Half an hour. Should 0 be default ?

#define DEFAULT_GMT_HOUR_OFFSSET 2
#define DEFAULT_GMT_MIN_OFFSSET 0

struct MasterConfigStruct:ModuleConfigStruct {
  // Array to store phone number information, permissions, ...
  phoneNumberDataType registeredNumbers[MAX_PHONE_NUMBERS];
  // Hostname for webapps, statistics, etc
  char webSite[HOSTNAME_MAX_LENGTH + 1];
  
  // API KEY from website registration, it's a uuid, 36 char
  char apiKey[API_KEY_MAX_LENGTH + 1];
    
  // ntp server
  char ntpHostName[HOSTNAME_MAX_LENGTH + 1];


  // Period at which statistics are sent to web server. 0 means no stat sent
  unsigned int statPeriod = DEFAULT_STAT_PERIOD;
  
  char homeSsid[SSID_MAX_LENGTH + 1];
  char homePwd[PWD_MAX_LENGTH + 1];
    
  char apSsid[SSID_MAX_LENGTH + 1];
  char apPwd[PWD_MAX_LENGTH + 1];
  int defaultAPExposition = DEFAULT_AP_EXPOSITION;
  
  int8_t gmtHourOffset = DEFAULT_GMT_HOUR_OFFSSET;
  int8_t gmtMinOffset = DEFAULT_GMT_MIN_OFFSSET;
  
};

 
class MasterConfigClass:public ModuleConfigClass {
public:
  MasterConfigClass(unsigned int version, const char* name);
  virtual void initFromDefault(void) override;
  
  RegisteredPhoneNumberClass* getRegisteredPhoneByNumber(const char* number); 
  RegisteredPhoneNumberClass* getRegisteredPhone(unsigned int offset); 
  void removePhone(const char*);
  void addPhone(const char*);  
  void setHomeSsid(const char* ssid);
  void setWebSite(const char* webSite);
  void setWebSite(String webSite);
  void setApiKey(const char* apiKey);
  void setApiKey(String apiKey);
  void setNtpServer(String ntpServer);
  void setNtpServer(const char *ntpServer);
  void setHomeSsid(String ssid);
  void setHomePwd(const char* pwd);
  void setHomePwd(String pwd);
  void setApSsid(const char* ssid);
  void setApSsid(String ssid);
  void setApPwd(const char* pwd);
  void setApPwd(String pwd);
  char* getHomeSsid(void);
  char* getWebSite(void);
  char* getApiKey(void);
  char* getNtpServer(void);
  char* getHomePwd(void);
  char* getApSsid(bool force=false);
  char* getApPwd(bool force=false);
  void setDefaultAPExposition(int delay);
  int getDefaultAPExposition(void);
  
  bool isHomeWifiConfigured(void);
  bool isAPInitialized(void);
  void setAdminNumber(char *number);
  void setAdminNumber(String numberString);
  char* getAdminNumber();
  void setGmtOffset(int8_t hour, int8_t min); 
  int8_t getGmtHourOffset(); 
  int8_t getGmtMinOffset(); 
  
protected:
  RegisteredPhoneNumberClass* _phoneNumbers[MAX_PHONE_NUMBERS];
  MasterConfigStruct* _getDataPtr(void);  
};
