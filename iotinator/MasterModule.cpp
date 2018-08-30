/**
 *  iotinator Master module
 *  Xavier Grosjean 2018
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */

#include "MasterModule.h"

MasterModule::MasterModule(MasterConfigClass* config, int displayAddr, int displaySda, int displayScl, int intPin, int ctrlPinParam):XIOTModule(config, displayAddr, displaySda, displayScl) {
  _isMaster = true;
  _oledDisplay->setLineAlignment(2, TEXT_ALIGN_CENTER);
  setLevel(config->getDefaultLevel());
}

bool MasterModule::customBeforeOTA() {
  return true;
}

