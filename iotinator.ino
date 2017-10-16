/**
 *  iotinator master module 
 *  Xavier Grosjean 2017
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
#include <EEPROM.h>
//#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
//#include <ESP8266HTTPClient.h>
//#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#include "initPageHtml.h"
#include "masterConfig.h"
#include "XOLEDDisplay.h"

// Global object to store config
masterConfigDataType masterConfigData;
MasterConfigClass *config;
XOLEDDisplayClass *oledDisplay;

// I couldn't find a way to instanciate this in the XOLEDDisplay lib
// and keep it working further than in the constructor...
SSD1306 display(0x3C, D5, D6);

//MDNSResponder mdns;
ESP8266WebServer server(80);
byte clientConnected = 0;
boolean homeWifiConnected = false;

void setup(){
  Serial.begin(9600);
  delay(100);
  config = new MasterConfigClass((unsigned int)CONFIG_VERSION, (char*)CONFIG_NAME, (void*)&masterConfigData);
  config->init();
  Serial.println(config->getName());
  
  server.on("/", [](){
    Serial.println("Rq on /");
    printHomePage();
  });

  // TODO: remove this !!! Needed during dev
  server.on("/reset", [](){
    Serial.println("Rq on /reset");
    config->initFromDefault();
    config->saveToEeprom();
    sendPage("Reset Done", 200);
  });
  
  Serial.print(MSG_OPENING_AP);
  Serial.println(config->getApSsid());
  WiFi.mode(WIFI_AP);
  WiFi.softAP(config->getApSsid(), config->getApPwd());
  Serial.println(WiFi.softAPIP());
  server.begin();
  printNumbers();
   
  // Initialise the OLED display
  oledDisplay = new XOLEDDisplayClass(&display);
  displayMessages();
}

void loop() {
  server.handleClient();
  oledDisplay->refresh();
  delay(100);
}

void printNumbers() {
  for(int i = 0; i < MAX_PHONE_NUMBERS; i++) {
    Serial.print("Numero ");
    Serial.print(i);
    Serial.print(" ");
    Serial.print(config->getRegisteredPhone(i)->getNumber());
    Serial.println(config->getRegisteredPhone(i)->isAdmin()?" true":" false");
  }
}

void printHomePage() {
  // If admin phone number is not defined, print page to define it
  if (config->getRegisteredPhone(0)->isAdmin()) {
    Serial.println("Admin set Page");
    sendPage(MSG_INIT_ADMIN_ALREADY_SET, 200);
  } else {
    server.send(200, "text/html", initPage);
    server.on("/init", [](){
      Serial.println("Rq on /init");
      if(config->getRegisteredPhone(0)->isAdmin()) {
        sendPage(MSG_ERR_ALREADY_INITIALIZED, 403);
        return;
      }
      if (!server.hasArg("admin")) {
        sendPage(MSG_ERR_BAD_REQUEST, 403);
        return;
      }
      String adminNumber = server.arg("admin");
      if (adminNumber.length() > 0) {
        if (adminNumber.length() < 10) {
          sendPage(MSG_ERR_ADMIN_LENGTH, 403);
          return;
        }
        config->setAdminNumber(adminNumber);
      }
      String apSsid = server.arg("apSsid");
      if (apSsid.length() > 0) {
        config->setApSsid(apSsid);
      }
      String apPwd = server.arg("apPwd");
      if( apPwd.length() > 0) {
        // Password need to be at least 8 characters
        if(apPwd.length() < 8) {
          sendPage(MSG_ERR_PASSWORD_LENGTH, 403);
          return;
        }
        config->setApPwd(apPwd);
      }
      // TODO: when GSM connected, send code, display confirmation page, 
      // and save once code confirmed
      // in the meantime, just save
      config->saveToEeprom();
      printNumbers();
      sendPage("Admin set", 200);
    });     
  }
}

void sendPage(const char* msg, int code) {
  char format[] = "<html><body>%s</body></html>";
  char* html = (char*)malloc(strlen(msg) + strlen(format) + 1);
  sprintf(html, format, msg);
  server.send(code, "text/html", html);
  free(html); 
}

void displayMessages( void )
{
  char message[100];
  oledDisplay->setTitle(config->getName());
  sprintf(message, MSG_FORMAT_SSID, config->getApSsid());
  oledDisplay->setLine(0, message);  
  IPAddress ipAddress = WiFi.softAPIP();
  sprintf(message, MSG_FORMAT_IP, ipAddress[0], ipAddress[1], ipAddress[2], ipAddress[3]);
  oledDisplay->setLine(1, message, false, true);
  oledDisplay->setLine(2, message);
  oledDisplay->setLine(3, message);
}
