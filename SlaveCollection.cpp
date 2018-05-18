/**
 *  Class handling Slave module registered in iotinator master 
 *  Xavier Grosjean 2018
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */

#include "SlaveCollection.h" 


SlaveCollection::SlaveCollection(DisplayClass *oledDisplay, XIOTModule* module) {
  _oledDisplay = oledDisplay;
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
  // check if one already registered module already has this name but not on same IP.  
  for (slaveMap::iterator it=_slaves.begin(); it!=_slaves.end(); ++it) {
    if((strcmp(it->second->getName(), name) == 0) && (strcmp(it->second->getIP(), ip) != 0))  {
      Serial.printf("Found duplicate %s on ip %s\n", name, it->second->getIP());
      slave.setNeedRename(true);
    }
  }
  //  std::cout << it->first << " => " << it->second << '\n';      
  
  // Since it's a map (not a multimap) and ip is key, only one entry per ip.
  // Todo : check name uniqueness in the collection. Rename if needed
  _slaves.insert(slavePair(ip, slave));
  
   
  return slave;
}