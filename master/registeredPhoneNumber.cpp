/**
 *  Class to persist the master module configuration data structure to EEPROM 
 *  Xavier Grosjean 2017
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
#include <Arduino.h>
#include "utils.h"

#include "registeredPhoneNumber.h"
#define DEFAULT_MIN_ALERT_INTERVAL 1800000 // Half an hour

RegisteredPhoneNumberClass::RegisteredPhoneNumberClass(phoneNumberDataType* phoneNumberPtr) {
  _phoneNumberPtr = phoneNumberPtr;
}

void RegisteredPhoneNumberClass::setNumber(char *number) {
  // Copy number making sure there is no overflow
  safeStringCopy(_phoneNumberPtr->number, number, PHONE_NUMBER_LENGTH);
}

char* RegisteredPhoneNumberClass::getNumber(void) {
  return _phoneNumberPtr->number;
}

void RegisteredPhoneNumberClass::reset(void) {
  _phoneNumberPtr->number[0] = 0; // faster than calling setNumber with ""
  _phoneNumberPtr->permissionFlags = 0;
  _phoneNumberPtr->lastAlertSmsTime = 0;
  _phoneNumberPtr->lastNotifSmsTime = 0;
  _phoneNumberPtr->minAlertInterval = DEFAULT_MIN_ALERT_INTERVAL;
}

bool RegisteredPhoneNumberClass::isUnset(void) {
  return (_phoneNumberPtr->number[0] == 0);
}

void RegisteredPhoneNumberClass::setAdmin(bool flag) {
  _setPermissionBit(ADMIN, flag);
}

bool RegisteredPhoneNumberClass::isAdmin(void) {
  return _getPermissionBit(ADMIN);
}

void RegisteredPhoneNumberClass::setAlertee(bool flag) {
  _setPermissionBit(ALERT, flag);
}

bool RegisteredPhoneNumberClass::isAlertee(void) {
  return _getPermissionBit(ALERT);
}

void RegisteredPhoneNumberClass::setNotifee(bool flag) {
  _setPermissionBit(NOTIF, flag);
}

bool RegisteredPhoneNumberClass::isNotifee(void) {
  return _getPermissionBit(NOTIF);
}

void RegisteredPhoneNumberClass::_setPermissionBit(unsigned int mask, bool flag) {
  if (flag) {
    _phoneNumberPtr->permissionFlags |= mask;
  } else {
    _phoneNumberPtr->permissionFlags &= !mask;
  }
}

bool RegisteredPhoneNumberClass::_getPermissionBit(unsigned int mask) {
  return ((_phoneNumberPtr->permissionFlags &= mask) != 0);
}