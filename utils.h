/**
 *  Useful stuff 
 *  Xavier Grosjean 2017
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
#pragma once
 
bool isElapsedDelay(unsigned long now, unsigned long* lastTime, unsigned long delay);
void safeStringCopy(char* to, const char* from, unsigned int length);