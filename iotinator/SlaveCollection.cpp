/**
 *  Class handling Slave module registered in iotinator master 
 *  Xavier Grosjean 2018
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */

#include "SlaveCollection.h" 

SlaveCollection::SlaveCollection(XIOTModule* module) {
  _module = module;
  Debug("Slave count: %d\n", getCount());
}

int SlaveCollection::getCount() {
  return _slaves.size();
}

Slave* SlaveCollection::refresh(char* jsonStr) {
  Debug("SlaveCollection::refresh\n");
  StaticJsonBuffer<JSON_BUFFER_REGISTER_SIZE> jsonBuffer; // registration is bigger than needed
  JsonObject& root = jsonBuffer.parseObject(jsonStr); 
  if (!root.success()) {
    Serial.println("Refreshing parse failure for:");
    Serial.println(jsonStr);
    return NULL;
  }
  const char *mac = (const char*)root[XIOTModuleJsonTag::MAC];
  if(!mac) {
    Serial.println("Refreshing: missing MAC addr");
    return NULL;
  }
  _module->getDisplay()->setLine(1, "Refreshing", TRANSIENT, NOT_BLINKING);
  _module->getDisplay()->setLine(2, mac, TRANSIENT, NOT_BLINKING);
  
  slaveMap::iterator it;
  it = _slaves.find(mac);
  if(it == _slaves.end()) {
    Serial.println("Refreshing: could not find module.");
    return NULL;
  }
  Slave *slave = it->second;
  _module->getDisplay()->setLine(2, slave->getName(), TRANSIENT, NOT_BLINKING);
  slave->setCustom((const char*)root[XIOTModuleJsonTag::custom]);
  return slave; // ptr to slave in collection, safe to return.
}

/**
 * Register a new slave
 * data from jsonStr needs to be copied, since it will be freed
 */ 
Slave* SlaveCollection::add(char* jsonStr) {
  StaticJsonBuffer<JSON_BUFFER_REGISTER_SIZE> jsonBuffer; 
  JsonObject& root = jsonBuffer.parseObject(jsonStr); 
  if (!root.success()) {
    Serial.println("Registration parse failure for:");
    Serial.println(jsonStr);
    _module->sendJson("{}", 500);
    return NULL;
  }
  const char *name = (const char*)root[XIOTModuleJsonTag::name]; 
  const char *mac = (const char*)root[XIOTModuleJsonTag::MAC];
  const char *ip = (const char*)root[XIOTModuleJsonTag::ip];
  if(!name || !mac || !ip) {
    return NULL;
  }
  Debug("SlaveCollection::add name '%s', mac '%s', ip '%s'\n", name, mac, ip);
  _module->getDisplay()->setLine(1, "Registering", TRANSIENT, NOT_BLINKING);
  _module->getDisplay()->setLine(2, name, TRANSIENT, NOT_BLINKING);
  Slave* slave = new Slave(name, mac, _module);
  // Insert it.
  std::pair <slaveMap::iterator, bool> slaveIt = _slaves.insert(slavePair(mac, slave));
  // If not inserted because exists, point to the one already registered so that we can update it
  if(!slaveIt.second) {
    delete slave;
    slave = slaveIt.first->second;
  }
  // We need to update some fields...  
  slave->setCanSleep((bool)root[XIOTModuleJsonTag::canSleep]);
  slave->setCustom((const char*)root[XIOTModuleJsonTag::custom]);
  slave->setUiClassName((const char*)root[XIOTModuleJsonTag::uiClassName]);
  slave->setHeap((int32_t)root[XIOTModuleJsonTag::heap]);
  slave->setPingPeriod((int)root[XIOTModuleJsonTag::pingPeriod]);  // Will set it to 0 if absent

  slave->setIP(ip);
  
  slave->setName(name); // in case it's a new name for an already registered module.
  // check if one OTHER (not same mac) already registered module already has this name
  if(nameAlreadyExists(name, mac)) {
    // Renaming will occur later, not within this request processing
    slave->setToRename(true);
  }  
  _refreshListBufferSize();
  return slave;
}

/**
 * Compute the buffer size to hold one attribute, its value and json syntax elements, for all registered modules
 **/
int SlaveCollection::_jsonAttributeSize(int moduleCount, const char *attrName, int valueSize) { 
  // size of the value of the attribute + size of its name + 2 double quotes + semi colon + coma 
  return moduleCount * (valueSize + strlen(attrName) + 2 + 1 + 1);
}

/**
 * Compute the buffer size to hold the json string listing all registered slave modules
 **/
void SlaveCollection::_refreshListBufferSize() {
  int moduleCount = getCount();
  _listBufferSize = LIST_BUFFER_SIZE;
  _listBufferSize += _jsonAttributeSize(moduleCount, XIOTModuleJsonTag::MAC, MAC_ADDR_MAX_LENGTH);
  _listBufferSize += _jsonAttributeSize(moduleCount, XIOTModuleJsonTag::name, NAME_MAX_LENGTH);
  _listBufferSize += _jsonAttributeSize(moduleCount, XIOTModuleJsonTag::ip, DOUBLE_IP_MAX_LENGTH);
  _listBufferSize += _jsonAttributeSize(moduleCount, XIOTModuleJsonTag::canSleep, 5);  // true or false
  _listBufferSize += _jsonAttributeSize(moduleCount, XIOTModuleJsonTag::pong, 5); // true or false
  _listBufferSize += _jsonAttributeSize(moduleCount, XIOTModuleJsonTag::uiClassName, UI_CLASS_NAME_MAX_LENGTH);
  _listBufferSize += _jsonAttributeSize(moduleCount, XIOTModuleJsonTag::heap, sizeof(uint32_t));
}

char* SlaveCollection::list() {
  int size = getCount();
  Debug("SlaveCollection::list %d slaves\n", size);
  if(size == 0) {
    return strcpy((char*)malloc(3), "{}");  
  }
  
  // Size estimation: https://arduinojson.org/assistant/
  // TODO: update this when necessary : max 10 fields per slave
  const size_t bufferSize = size*JSON_OBJECT_SIZE(10) + JSON_OBJECT_SIZE(size);
  
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& root = jsonBuffer.createObject();
  int customSize = 0;
  
  for (slaveMap::iterator it=_slaves.begin(); it!=_slaves.end(); ++it) {
    JsonObject& slave = root.createNestedObject(it->second->getMAC());
    slave[XIOTModuleJsonTag::name] = it->second->getName();
    slave[XIOTModuleJsonTag::ip] = it->second->getIP();
    slave[XIOTModuleJsonTag::canSleep] = (bool)it->second->getCanSleep();
    slave[XIOTModuleJsonTag::pong] = (bool)it->second->getPong();
    slave[XIOTModuleJsonTag::uiClassName] = it->second->getUiClassName();
    slave[XIOTModuleJsonTag::heap] = it->second->getHeap();
    char *custom = (char *)it->second->getCustom();
    if(custom != NULL) {
      slave[XIOTModuleJsonTag::custom] = custom;
      customSize = strlen(custom);    
    }
    Debug("Name '%s' on mac '%s'\n", it->second->getName(), it->second->getMAC());
  }
  
  // listBufferSize is updated when a slave registers
  int strBufferSize = _listBufferSize + customSize;
  char* strBuffer = (char *)malloc(strBufferSize); 
  root.printTo(strBuffer, strBufferSize-1);
  Debug("Reserved size: %d, actual size: %d\n", strBufferSize, strlen(strBuffer));
  return strBuffer;
}

void SlaveCollection::reset() {
  int size = getCount();
  const char *ip, *name; 
  Serial.printf("SlaveCollection::reset %d slaves\n", size);
  for (slaveMap::iterator it=_slaves.begin(); it!=_slaves.end(); ++it) {
    ip = it->second->getIP();
    name = it->second->getName();
    Serial.printf("Reset module '%s' on ip '%s'\n", name, ip);
    bool result = it->second->reset();
    if(result) {
      
    }
    
    Serial.printf("Result: %s\n", result?"ok":"nok");
  }
}

void SlaveCollection::ping() {
  int size = getCount();
  Debug("SlaveCollection::ping %d slaves\n", size);
  bool canSleep;  // If true, must not be pinged
  const char *ip, *name;
  int pingPeriod;
  
  for (slaveMap::iterator it=_slaves.begin(); it!=_slaves.end(); ++it) {
    ip = it->second->getIP();
    name = it->second->getName();
    canSleep = (bool)it->second->getCanSleep();
    pingPeriod = (bool)it->second->getPingPeriod();
    if(!canSleep && pingPeriod > 0) {
      Serial.printf("Ping module '%s' on ip '%s'\n", name, ip);
      bool result = it->second->ping();
      Serial.printf("Connected: %s\n", result?"true":"false");
      if(!result) {
        char message[100];
        sprintf(message, "Ping failed: %s", name);
        _module->getDisplay()->setLine(1, message, TRANSIENT, NOT_BLINKING);      
      }
    } else {
      Serial.printf("Not pinging module '%s' on ip '%s': canSleep: %d, pingPeriod: %d\n", name, ip, canSleep, pingPeriod);
    }
  }
  uint32_t freeMem = system_get_free_heap_size();
  Serial.printf("Free heap mem: %d\n", freeMem);    
}


void SlaveCollection::renameOne(Slave *slave) {
  Debug("SlaveCollection::renameOne");
  int digit = 0;
  char alpha[NAME_MAX_LENGTH + 1];
  char newName[NAME_MAX_LENGTH + 1];

  bool ok = false;
  int i;
  strcpy(alpha, slave->getName());
  char *withUnderscore = strtok(alpha, "_");
  if(withUnderscore != NULL) {
    char *digitPtr = strtok(NULL, "_");
    if(digitPtr != NULL) {
       digit = atoi(digitPtr);
    }
  }
  
  while (!ok && strlen(newName) < NAME_MAX_LENGTH) {
    sprintf(newName, "%s_%d", alpha, ++digit);
    Debug("Testing name %s\n", newName);   
    if(!nameAlreadyExists(newName, slave->getMAC())) {
      ok = true;
    }   
  }
  if(!ok) {
    Serial.println("Can't find a non duplicated name");
  } else {
    slave->renameTo(newName);
  }
  
}

/**
 * check if a name exists in the collection on a different mac
 */
bool SlaveCollection::nameAlreadyExists(const char* name, const char* mac) {
  for (slaveMap::iterator it=_slaves.begin(); it!=_slaves.end(); ++it) {
    if((strcmp(it->second->getName(), name) == 0) && (strcmp(it->second->getMAC(), mac) != 0))  {
      Debug("Found duplicate %s on ip %s\n", name, it->second->getIP()); // ip is easier for debugging since it's displayed on modules
      return true;
    }
  }
  return false;
}