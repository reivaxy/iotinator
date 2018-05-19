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
 */ 
Slave* SlaveCollection::add(const char* name, const char* ip) {
  Debug("SlaveCollection::add name '%s', ip '%s'\n", name, ip);
  Slave* slave = new Slave(name, ip, _module);
  
  // Insert it. If already inserted (same ip), get the one already inserted
  // If not already inserted, get the one we just inserted. So that name compare and flag setting work
  std::pair <slaveMap::iterator, bool> slaveIt = _slaves.insert(slavePair(ip, slave));
  // If not inserted, point to the already preset
  if(!slaveIt.second) {
    slave = slaveIt.first->second;
  }
  
  slave->setName(name); // in case it's a new name for an already registered module.
  // check if one OTHER (not same IP) already registered module already has this name
  if(alreadyExists(name, ip)) {
    slave->setToRename(true);
  }  

  return slave;
}

void SlaveCollection::list() {
  for (slaveMap::iterator it=_slaves.begin(); it!=_slaves.end(); ++it) {
    Serial.printf("Name '%s' on ip '%s'\n", it->second->getName(), it->second->getIP());
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
    sprintf(newName, "%s_%d", alpha, digit +1);
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
      Serial.printf("Found duplicate %s on ip %s\n", name, it->second->getIP());
      return true;
    }
  }
  return false;
}