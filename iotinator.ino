/**
 *  iotinator master module 
 *  Xavier Grosjean 2017
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <stdio.h>

#include "initPageHtml.h"
#include "masterConfig.h"
#include "Display.h"

#define TIME_STR_LENGTH 100

// Global object to store config
masterConfigDataType masterConfigData;
MasterConfigClass *config;
DisplayClass *oledDisplay;

#include "gsm.h"
//SIM800 TX is connected to RX MCU 13 (D7)
#define SIM800_TX_PIN 13
//SIM800 RX is connected to TX MCU 15 (D8)
#define SIM800_RX_PIN 15
SoftwareSerial serialSIM800(SIM800_TX_PIN, SIM800_RX_PIN, false, 1000);
GsmClass gsm(&serialSIM800);
#include "gsmMessageHandlers.h"

// I couldn't find a way to instanciate this in the XOLEDDisplay lib
// and keep it working further than in the constructor...
SSD1306 display(0x3C, D5, D6);

//MDNSResponder mdns;
ESP8266WebServer server(80);
byte clientConnected = 0;
boolean homeWifiConnected = false;
unsigned long elapsed200ms = 0;
unsigned long elapsed500ms = 0;
unsigned long elapsed2s = 0;
unsigned long elapsed10s = 0;
bool gsmEnabled = false;
MDNSResponder mdns;

void setup(){
  char timeStr[TIME_STR_LENGTH+1];
  int result;
  Serial.begin(9600);
  delay(100);
  config = new MasterConfigClass((unsigned int)CONFIG_VERSION, (char*)CONFIG_NAME, (void*)&masterConfigData);
  config->init();
  Serial.println(config->getName());
  
  server.on("/", [](){
    Serial.println("Rq on /");
    printHomePage();
    ping();
  });

  // TODO: remove this !!! Needed during dev
  server.on("/reset", [](){
    Serial.println("Rq on /reset");
    config->initFromDefault();
    config->saveToEeprom();
    sendPage("Reset Done", 200);
    gsm.sendSMS(config->getAdminNumber(), "Reset done");  //   
  });
  
  Serial.print(MSG_WIFI_OPENING_AP);
  Serial.println(config->getApSsid());
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(config->getApSsid(), config->getApPwd());
  Serial.println(WiFi.softAPIP());
  
  if(config->homeWifiConfigured()) {
    Serial.print(MSG_WIFI_CONNECTING_HOME);
    Serial.println(config->getHomeSsid());
    WiFi.begin(config->getHomeSsid(), config->getHomePwd());
  }  
  server.begin();
  printNumbers();
   
  // Initialise the OLED display
  oledDisplay = new DisplayClass(&display);
  initMessages();
  initGsmMessageHandlers();
  
  unsigned long now = millis();
  elapsed10s = now;
  gsmEnabled = gsm.init();
}

void loop() {
  unsigned long now = millis();
  // Check if any request to serve
  server.handleClient();
  int wifiStatus = WiFi.status();
  if (wifiStatus == WL_CONNECTED) {
    if(!homeWifiConnected) {
      Serial.println("Home wifi connected");
      ping();
    }
    homeWifiConnected = true;
  } else {
    if(homeWifiConnected) {
      Serial.println("Home wifi disconnected");
    }
    homeWifiConnected = false;
  }
  // Display needs to be refreshed periodically to handle blinking
  oledDisplay->refresh();
  
  // Let gsm do its tasks: checking connection, incomming messages, 
  // handler notifications...
  gsm.refresh();   
 
  delay(20);
  
}


// Temp, for tests
void ping() {
  Serial.println("Ping");
  Serial.println(config->getHomeSsid());
  Serial.println(WiFi.localIP());
  WiFiClient client;

  HTTPClient http;
  http.begin("http://c-est-simple.com/cgi-bin/webdistrib.cgi?toto=1");
  http.addHeader("Content-Type", "multipart/form-data");
  // doc on payload format: https://docs.internetofthings.ibmcloud.com/messaging/payload.html
  int httpCode = http.GET();   // Log stuff to Serial ?
  Serial.print("HTTP code: ");
  Serial.println(httpCode);
  http.end();
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
  // If init already done, display page saying so, otherwise display init page
  // TODO Disabled for now
  if (false && config->isInitialized()) {
    Serial.println("Init done Page");
    sendPage(MSG_INIT_ALREADY_DONE, 200);
  } else {
  
    char *page = (char *)malloc(strlen(initPage) + 10);
    sprintf(page, initPage, gsmEnabled ? "": "noGsm");
    server.send(200, "text/html", page);
    free(page);
    server.on("/init", [](){
      Serial.println("Rq on /init");
//      if(config->isInitialized()) {
//        sendPage(MSG_ERR_ALREADY_INITIALIZED, 403);
//        return;
//      }
      
      if (!server.hasArg("apSsid")) {
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
        gsm.sendSMS(config->getAdminNumber(), "You are admin");  //
        oledDisplay->setLine(2, ""); 
        oledDisplay->setLine(2, MSG_INIT_DONE, true, false); 
      }
      
      // TODO: add checks in the config methods
      // Read and save new AP SSID 
      String apSsid = server.arg("apSsid");
      if (apSsid.length() > 0) {
        // TODO: add checks
        config->setApSsid(apSsid);
        initSsidMsg();
      }
      // Read and save new AP PWD 
      String apPwd = server.arg("apPwd");
      if( apPwd.length() > 0) {
        // Password need to be at least 8 characters
        if(apPwd.length() < 8) {
          sendPage(MSG_ERR_PASSWORD_LENGTH, 403);
          return;
        }
        config->setApPwd(apPwd);
      }
            
      // Read and save home SSID 
      String homeSsid = server.arg("homeSsid");
      if (homeSsid.length() > 0) {
        config->setHomeSsid(homeSsid);
      }
      // Read and save home PWD 
      String homePwd = server.arg("homePwd");
      if( homePwd.length() > 0) {
        // Password need to be at least 8 characters
        if(homePwd.length() < 8) {
          sendPage(MSG_ERR_PASSWORD_LENGTH, 403);
          return;
        }
        config->setHomePwd(homePwd);
      }
      
      
      // TODO: when GSM connected, send code, display confirmation page, 
      // and save once code confirmed
      // in the meantime, just save
      config->saveToEeprom();
      
      // New Access Point
      WiFi.mode(WIFI_AP_STA);
      WiFi.softAP(config->getApSsid(), config->getApPwd());
      if(config->homeWifiConfigured()) {
        WiFi.begin(config->getHomeSsid(), config->getHomePwd());
      }
      
      printNumbers();
      sendPage(MSG_INIT_DONE, 200);
      
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

void initMessages( void )
{
  char message[100];
  oledDisplay->setTitle(config->getName());
  initSsidMsg();
  IPAddress ipAddress = WiFi.softAPIP();
  sprintf(message, MSG_FORMAT_IP, ipAddress[0], ipAddress[1], ipAddress[2], ipAddress[3]);
  oledDisplay->setLine(1, message);
  if(!config->isInitialized()) {
    oledDisplay->setLine(2, MSG_INIT_REQUEST, NOT_TRANSIENT, BLINKING);
  } 
  if(gsmEnabled) oledDisplay->gsmIcon(BLINKING);  
  oledDisplay->clockIcon(BLINKING);
  
}

void initSsidMsg() {
  char message[100];
  sprintf(message, MSG_FORMAT_SSID, config->getApSsid());
  bool blinkWifi = false;
  if (strcmp(config->getApSsid(), DEFAULT_APSSID) == 0) {
    blinkWifi = true;
  }
  oledDisplay->setLine(0, message); 
  oledDisplay->wifiIcon(blinkWifi);
}