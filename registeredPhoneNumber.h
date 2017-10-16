/**
 *  Definition of the phone number class for the master module 
 *  The fields holding the data are grouped in a struc so that they can be easily and 
 *  safely saved/retrieved to/from EEPROM since they are part of a EEPROMConfig data structure.
 *  Xavier Grosjean 2017
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
#pragma once

#define PHONE_NUMBER_LENGTH 15
#define MAX_PHONE_NUMBERS 4
#define DEFAULT_MIN_ALERT_INTERVAL 1800000 // Half an hour

// These fields need to be grouped in a struct to be safely peristed in EEPROM
typedef struct {
  char number[PHONE_NUMBER_LENGTH+1];
  unsigned int permissionFlags; // bit field, 0x1 : admin, 0x2: alert, 0x4: notifs  
  unsigned long lastAlertSmsTime = 0UL;  // When was last time (millis()) an alert SMS sent to that number
  unsigned long lastNotifSmsTime = 0UL;  // When was last time (millis()) a notification SMS sent to that number
  unsigned long minAlertInterval = DEFAULT_MIN_ALERT_INTERVAL;  // minimum interval in milliseconds between 2 alert SMS to avoid flooding  
} phoneNumberDataType;

class RegisteredPhoneNumberClass {
public:
  RegisteredPhoneNumberClass(phoneNumberDataType* phoneNumber);
  
  void setNumber(char* number);
  char* getNumber(void);
  void setAdmin(bool flag);
  bool isAdmin(void);
  void setAlertee(bool flag);
  bool isAlertee(void);
  void setNotifee(bool flag);
  bool isNotifee(void);
  void reset(void);
  bool isUnset(void);
  
private:
  // bit masks for permission handling
  static const unsigned int ADMIN = 0x1;
  // 0x2, 0x4, 0x8 are reserved for future use  
  static const unsigned int ALERT = 0x10;  
  static const unsigned int NOTIF = 0x20;
  phoneNumberDataType *_phoneNumberPtr;
  
  void _setPermissionBit(unsigned int mask, bool flag);
  bool _getPermissionBit(unsigned int mask);  
};

