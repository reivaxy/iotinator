/**
 *  Class to persist the master module configuration data structure to EEPROM 
 *  Xavier Grosjean 2017
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
#include <Arduino.h>

#include "masterConfig.h"

MasterConfigClass::MasterConfigClass(unsigned int version, const char* name, void* dataPtr):EEPROMConfigClass(version, name, dataPtr, sizeof(masterConfigDataType)) {
  Serial.println("MasterConfigClass::MasterConfigClass");

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
  Serial.println("MasterConfigClass::initFromDefault");
  EEPROMConfigClass::initFromDefault(); // handles version and name.
  
  // Reset all registered phone numbers
  for(int i = 0; i < MAX_PHONE_NUMBERS; i++) {
    _phoneNumbers[i]->reset();
  }
}

/**
 * Return the typed data structure object
 *
 */
masterConfigDataType* MasterConfigClass::_getConfigPtr(void) {
  return (masterConfigDataType*)getData();
}

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
