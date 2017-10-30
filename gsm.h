#pragma once

#include <SoftwareSerial.h>
#include <Arduino.h>


 

class GsmClass {
public:
  GsmClass(SoftwareSerial* serial);
  
  void initTimeFromNetwork();
  void getTime();
  void checkGsm();
  
protected:
  SoftwareSerial* _serialSIM800;
  
};
