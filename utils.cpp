/**
 *  Useful stuff 
 *  Xavier Grosjean 2017
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
#include <Arduino.h>
 
// return true if current time is after given time + delay
bool isElapsedDelay(unsigned long now, unsigned long* lastTime, unsigned long delay) {
  unsigned long elapsed = now - *lastTime;
  bool result = false;
  // millis() overflows unsigned long after about 50 days => 0  but since unsigned,
  // no problem !
  if(elapsed >= delay){
    result = true;
  }
  if(lastTime == 0) {
    result = true;
  }
  if(result) {
    *lastTime = now;
  }
  return result;
}

// Copy one string into another making sure there is no length overflow
void safeStringCopy(char* to, const char* from, unsigned int length) {
  strncpy(to, from, length);
  to[length] = 0;
}

// convert a String into a char array that will need to be freed 
void stringToCharP(String in, char** out) {
  unsigned int size = in.length() + 1;
  *out = (char *)malloc(size);
  in.toCharArray(*out, size);
}