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

/**
 * Register a new slave
 * BEWARE: we are in a server GET processing callback: do not attempt any outgoing request.
 * (no ping, no slave renaming here)
 * data from jsonStr needs to be copied, since it will be freed
 */ 
Slave* SlaveCollection::add(char* jsonStr) {
  StaticJsonBuffer<JSON_BUFFER_REGISTER_SIZE> jsonBuffer; 
  JsonObject& root = jsonBuffer.parseObject(jsonStr); 
  if (!root.success()) {
    Serial.println("Registration: parse failure for:");
    Serial.println(jsonStr);
    _module->sendJson("{}", 500);
    return NULL;
  }
  const char *name = (const char*)root[XIOTModuleJsonTag::name]; 
  const char *ip = (const char*)root[XIOTModuleJsonTag::ip];
  Debug("SlaveCollection::add name '%s', ip '%s'\n", name, ip);
  char message[100];
  sprintf(message, "Registering %s", name);
  _module->getDisplay()->setLine(1, message, TRANSIENT, NOT_BLINKING);
  Slave* slave = new Slave(name, ip, _module);
  // Insert it. If already inserted (same ip), get the one already inserted
  // If not already inserted, get the one we just inserted. So that name compare and flag setting work
  std::pair <slaveMap::iterator, bool> slaveIt = _slaves.insert(slavePair(ip, slave));
  // If not inserted, point to the already preset
  if(!slaveIt.second) {
    slave = slaveIt.first->second;
  }
  // We need to update some fields...  
  slave->setCanSleep((bool)root[XIOTModuleJsonTag::canSleep]);
  slave->setCustom((const char*)root[XIOTModuleJsonTag::custom]);
  
  slave->setName(name); // in case it's a new name for an already registered module.
  // check if one OTHER (not same IP) already registered module already has this name
  if(alreadyExists(name, ip)) {
    slave->setToRename(true);
  }  

  return slave;
}

void SlaveCollection::list(char *strBuffer, int strBufferSize) {
  int size = getCount();
  Debug("SlaveCollection::list %d slaves\n", size);
  
  // Size estimation: https://arduinojson.org/assistant/
  // plan for 10 fields per slave
  const size_t bufferSize = size*JSON_OBJECT_SIZE(10) + JSON_OBJECT_SIZE(size);
  
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& root = jsonBuffer.createObject();
  
  for (slaveMap::iterator it=_slaves.begin(); it!=_slaves.end(); ++it) {
    JsonObject& slave = root.createNestedObject(it->second->getIP());
    slave[XIOTModuleJsonTag::name] = it->second->getName();
    slave[XIOTModuleJsonTag::canSleep] = (bool)it->second->getCanSleep();
    slave[XIOTModuleJsonTag::pong] = (bool)it->second->getPong();
    char *custom = (char *)it->second->getCustom();
    if(custom != NULL) {
      slave[XIOTModuleJsonTag::custom] = custom;    
    }
    Debug("Name '%s' on ip '%s'\n", it->second->getName(), it->second->getIP());
  }
  root.printTo(strBuffer, strBufferSize-1);
}

void SlaveCollection::ping() {
  int size = getCount();
  Debug("SlaveCollection::ping %d slaves -----------------\n", size);
  bool canSleep;  // If true, must not be pinged
  const char *ip, *name;
  
  
  for (slaveMap::iterator it=_slaves.begin(); it!=_slaves.end(); ++it) {
    ip = it->second->getIP();
    name = it->second->getName();
    canSleep = (bool)it->second->getCanSleep();
    if(!canSleep) {
      Serial.printf("Ping module '%s' on ip '%s'\n", name, ip);
      bool result = it->second->ping();
      Serial.printf("Result: %s\n", result?"true":"false");
    } else {
      Serial.printf("Not ping module '%s' on ip '%s' (canSleep)\n", name, ip);
    }
  }
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
    if(!alreadyExists(newName, slave->getIP())) {
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
 * check if a name exists in the collection on a different ip
 */
bool SlaveCollection::alreadyExists(const char* name, const char* ip) {
  for (slaveMap::iterator it=_slaves.begin(); it!=_slaves.end(); ++it) {
    if((strcmp(it->second->getName(), name) == 0) && (strcmp(it->second->getIP(), ip) != 0))  {
      Debug("Found duplicate %s on ip %s\n", name, it->second->getIP());
      return true;
    }
  }
  return false;
}