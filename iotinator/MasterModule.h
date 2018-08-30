/**
 *  iotinator Master module 
 *  Xavier Grosjean 2018
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
#pragma once 
#include "MasterConfig.h"
#include <XIOTModule.h>

 
class MasterModule:public XIOTModule {
public:
  MasterModule(MasterConfigClass* config, int displayAddr, int displaySda, int displayScl);

protected:    
  
};