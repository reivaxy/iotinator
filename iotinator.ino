/**
 *  iotinator master module 
 *  Xavier Grosjean 2017
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <stdio.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <XIOTDisplay.h>
#include <XIOTModule.h> 

#include "masterConfig.h"
#include "SlaveCollection.h"

#include "initPageHtml.h"
#include "appPageHtml.h"

#define TIME_STR_LENGTH 100

#define API_VERSION "1.0"    // modules can check API version to make sure they are compatible...

// Global object to store config
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

ESP8266WebServer* server;
bool homeWifiConnected = false;
bool ntpServerInitialized = false;
unsigned long elapsed200ms = 0;
unsigned long elapsed500ms = 0;
unsigned long elapsed2s = 0;
bool gsmEnabled = false;
MDNSResponder mdns;
time_t timeNow = 0; 
time_t timeLastTimeDisplay = 0;
time_t timeLastWifiDisplay = 0;
time_t timeLastPing = 0;
SlaveCollection *slaveCollection;
Slave* slaveToRename = NULL;

// Warning: XIOTModule class does not yet handle STA_AP module, but provides nice utilities
// that we want to reuse here :) 
// TODO: when XIOTModule class can handle AP_STA, use it here to get rid of
// TODO: a lot of common code.
XIOTModule* module; 

// Handlers will work as long as these variables exists. 
static WiFiEventHandler wifiSTAGotIpHandler, wifiSTADisconnectedHandler,
                        stationConnectedHandler, stationDisconnectedHandler ;
bool defaultAP = true;
bool displayAP = false;
String ipOnHomeSsid;

void setup() {
  WiFi.mode(WIFI_OFF);
  Serial.begin(9600);
  delay(100);
  config = new MasterConfigClass((unsigned int)CONFIG_VERSION, (char*)CONFIG_NAME);
  config->init();
  Serial.println(config->getName());

  // Initialise the OLED display
  oledDisplay = new DisplayClass(0x3C, D5, D6);
  initDisplay();
  
  // Beware the module instantiation initializes the server
  module = new XIOTModule(oledDisplay);
  addEndpoints();
  
  // Initialize the Slave Collection 
  slaveCollection = new SlaveCollection(module);
  
  // After a reset, open Default Access Point
  // If Access Point was customized, we'll switch to it after one minute
  // This is supposed to give slave modules time to initialize.
  // Before checking for Home Wifi configuration, module is Wifi Access Point only
  WiFi.mode(WIFI_AP);
  initSoftAP();
  
  // If Home wifi was configured previously, module should connect to Home Wifi.
  if(config->isHomeWifiConfigured()) {
    Serial.print(MSG_WIFI_CONNECTING_HOME);
    Serial.println(config->getHomeSsid());
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(config->getHomeSsid(), config->getHomePwd());
  }  
  
  stationConnectedHandler = WiFi.onSoftAPModeStationConnected(&onStationConnected);
  stationDisconnectedHandler  = WiFi.onSoftAPModeStationDisconnected(&onStationDisconnected);
  
  initGsmMessageHandlers();
  gsmEnabled = gsm.init();
  printNumbers();     
  
  wifiSTAGotIpHandler = WiFi.onStationModeGotIP(onSTAGotIP); 
  wifiSTADisconnectedHandler = WiFi.onStationModeDisconnected(onSTADisconnected); 
}

// Opens the Wifi network Access Point.
// For the first X seconds, it is the default one, then the custom one if configured.
void initSoftAP() {
  Serial.print(MSG_WIFI_OPENING_AP);
  Serial.println(config->getApSsid());
  WiFi.softAP(config->getApSsid(), config->getApPwd());
  Serial.println(WiFi.softAPIP());
  wifiDisplay();
}

void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt) {
  char message[50];
  sprintf(message, "Mac %02x:%02x:%02x:%02x:%02x:%02x\n", evt.mac[0], evt.mac[1], evt.mac[2], evt.mac[3], evt.mac[4], evt.mac[5]);
  oledDisplay->setLine(1, MSG_WIFI_STATION_CONNECTED, TRANSIENT, NOT_BLINKING);
  oledDisplay->setLine(2, message, TRANSIENT, NOT_BLINKING);  
}

void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt) {
  Serial.println(MSG_WIFI_STATION_DISCONNECTED);
  oledDisplay->setLine(1, MSG_WIFI_STATION_DISCONNECTED, TRANSIENT, NOT_BLINKING);
  // TODO: remove it from slave collection ? 
  // Disconnection needs a long time to be triggered (15mn ?)
  // May be periodic ping should be enough ?
}

// Called when STA is connected to home wifi and IP was obtained
void onSTAGotIP (WiFiEventStationModeGotIP ipInfo) {
  ipOnHomeSsid = ipInfo.ip.toString();
  Serial.printf("Got IP on %s: %s\n", config->getHomeSsid(), ipOnHomeSsid.c_str());
  homeWifiConnected = true;
  wifiDisplay();
  NTP.setInterval(7200);
  NTP.begin();
  NTP.setTimeZone(config->getGmtHourOffset(), config->getGmtMinOffset());
  NTP.onNTPSyncEvent([](NTPSyncEvent_t error) {
    if (error) {
      Serial.print("NTP Time Sync error: ");
      if (error == noResponse)
        Serial.println("NTP server not reachable");
      else if (error == invalidAddress)
        Serial.println("Invalid NTP server address");
      }
    else {
      Serial.print("Got NTP time: ");
      Serial.println(NTP.getTimeDateString(NTP.getLastNTPSync()));
      ntpServerInitialized = true;
      timeDisplay();
    }
  });
}

void onSTADisconnected(WiFiEventStationModeDisconnected event) {
  // Continuously get messages, so just output once.
  if(homeWifiConnected) {
    Serial.printf("Lost connection to %s, error: %d\n", event.ssid.c_str(), event.reason);
    homeWifiConnected = false;
    wifiDisplay();
    NTP.stop();
  }
}

void addEndpoints() {
  server = module->getServer();  
  server->on("/", [](){
    if (config->isAPInitialized()) {
      printAppPage();
    } else {
      printHomePage();
    }
  });

  server->on("/init", [](){
    printHomePage();
  });


  server->on("/api/list", [](){
    char *moduleListStr = slaveCollection->list();
    module->sendJson(moduleListStr, 200);
    free(moduleListStr); 

    uint32_t freeMem = system_get_free_heap_size();
    Serial.printf("Free heap mem: %d\n", freeMem);   
  });
  
  /**
   * This API returns the SSID and PWD of the customized Access Point: modules will use it to connect to iotinator
   * NB: only 4 clients  can connect (TODO: to check!) 
   * => may be slave modules could also create an access point and act as relay for
   * other modules.
   **/
  server->on("/api/config", [](){
//    Serial.println("Rq on /api/config");
    char configMsg[JSON_STRING_CONFIG_SIZE];
    StaticJsonBuffer<JSON_BUFFER_CONFIG_SIZE> jsonBuffer;    
    // Create the root object
    JsonObject& root = jsonBuffer.createObject();
    root[XIOTModuleJsonTag::version] = API_VERSION ;
    root[XIOTModuleJsonTag::APInitialized] = config->isAPInitialized();
    root[XIOTModuleJsonTag::APSsid] = config->getApSsid(true);
    root[XIOTModuleJsonTag::APPwd] = config->getApPwd(true);
    root[XIOTModuleJsonTag::timestamp] = now();
    root[XIOTModuleJsonTag::homeWifiConnected] = homeWifiConnected;
    root[XIOTModuleJsonTag::gsmEnabled] = gsmEnabled;
    root[XIOTModuleJsonTag::timeInitialized] = ntpServerInitialized;
    root.printTo(configMsg, JSON_STRING_CONFIG_SIZE);
    module->sendJson(configMsg, 200);
  });

  /**
   * This endpoints allows slave modules to register themselves to master
   */
  server->on("/api/register",  [](){
    char *jsonString;
    Serial.println("Registering module");
    // This will allocate jsonString
    XUtils::stringToCharP(server->arg("plain"), &jsonString);
    // slaveCollection->add method need to copy the data since jsonString will be freed. 
    Serial.println(jsonString); 
    Slave* slave = slaveCollection->add(jsonString);
    free(jsonString);
    if(slave == NULL) {
      module->sendJson("{}", 500);
      oledDisplay->setLine(1, "Registration failed", TRANSIENT, NOT_BLINKING);
    } else {
      module->sendJson("{}", 200);
      if(slave->getToRename()) {
        slaveToRename = slave;
      }
    }
    Serial.printf("New slave count: %d\n", slaveCollection->getCount());    
  });

  
  // TODO: remove this or make it better. Needed during dev
  // reset may be only possible by SMS from admin number ?
  server->on("/api/swarmReset", [](){
    Serial.println("Rq on /swarmReset");
    slaveCollection->reset();
    config->initFromDefault();
    config->saveToEeprom();
    module->sendJson("{}", 200);
    gsm.sendSMS(config->getAdminNumber(), "Reset done");  // 
    WiFi.mode(WIFI_AP);
    initSoftAP();  
  });
  
}  


// Temp, for tests
//void ping() {
//  Serial.println("Ping");
//  Serial.println(config->getHomeSsid());
//  Serial.println(WiFi.localIP());
//  HTTPClient http;
//  http.begin("http://c-est-simple.com/cgi-bin/webdistrib.cgi?toto=1");
//  http.addHeader("Content-Type", "multipart/form-data");
//   doc on payload format: https://docs.internetofthings.ibmcloud.com/messaging/payload.html
//  int httpCode = http.GET();   // Log stuff to Serial ?
//  Serial.print("HTTP code: ");
//  Serial.println(httpCode);
//  http.end();
//}

void printNumbers() {
  if(!gsmEnabled) return;
  for(int i = 0; i < MAX_PHONE_NUMBERS; i++) {
    Serial.print("Numero ");
    Serial.print(i);
    Serial.print(" ");
    Serial.print(config->getRegisteredPhone(i)->getNumber());
    Serial.println(config->getRegisteredPhone(i)->isAdmin()?" true":" false");
  }
}

void printAppPage() {
  char *page = (char *)malloc(strlen(appPage) + strlen(DEFAULT_WEBAPP_HOST) + 10);
  sprintf(page, appPage, DEFAULT_WEBAPP_HOST);
  module->sendHtml(page, 200);
  free(page);
}

void printHomePage() {
  // If init already done, display page saying so, otherwise display init page
  
  // TODO Disabled for now, need to be enabled !!
  if (false && config->isAPInitialized()) {
    Serial.println("Init done Page");
    module->sendText(MSG_INIT_ALREADY_DONE, 200);
  } else {
  
    char *page = (char *)malloc(strlen(initPage) + 10);
    sprintf(page, initPage, gsmEnabled ? "": "noGsm");
    module->sendHtml(page, 200);
    free(page);
    server->on("/initSave", [](){
      Serial.println("Rq on /initSave");
      
      // TODO: /initSave might need to be disabled once done ?
      if(false && config->isAPInitialized()) {
        module->sendHtml(MSG_ERR_ALREADY_INITIALIZED, 403);
        return;
      }
      
      if (!server->hasArg("apSsid")) {
        module->sendText(MSG_ERR_BAD_REQUEST, 403);
        return;
      }
      String adminNumber = server->arg("admin");
      if (adminNumber.length() > 0) {
        if (adminNumber.length() < 10) {
          module->sendText(MSG_ERR_ADMIN_LENGTH, 403);
          return;
        }
        config->setAdminNumber(adminNumber);
        gsm.sendSMS(config->getAdminNumber(), "You are admin");  //
        oledDisplay->setLine(2, ""); 
        oledDisplay->setLine(2, MSG_INIT_DONE, true, false); 
      }
      
      // TODO: add checks in the config methods
      // Read and save new AP SSID 
      String apSsid = server->arg("apSsid");
      if (apSsid.length() > 0) {
        // TODO: add checks
        config->setApSsid(apSsid);
        wifiDisplay();
      }
      // Read and save the web app server      
      String appHost = server->arg("appHost");
      if (apSsid.length() > 0) {
        // TODO: add checks
        config->setAppHost(appHost);
      }
      // Read and save new AP PWD 
      String apPwd = server->arg("apPwd");
      if( apPwd.length() > 0) {
        // Password need to be at least 8 characters
        if(apPwd.length() < 8) {
          module->sendText(MSG_ERR_PASSWORD_LENGTH, 403);
          return;
        }
        config->setApPwd(apPwd);
      }
            
      // Read and save home SSID 
      String homeSsid = server->arg("homeSsid");
      if (homeSsid.length() > 0) {
        config->setHomeSsid(homeSsid);
      }
      // Read and save home PWD 
      String homePwd = server->arg("homePwd");
      if( homePwd.length() > 0) {
        // Password need to be at least 8 characters
        if(homePwd.length() < 8) {
          module->sendText(MSG_ERR_PASSWORD_LENGTH, 403);
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
      if(config->isHomeWifiConfigured()) {
        WiFi.begin(config->getHomeSsid(), config->getHomePwd());
      }
      
      printNumbers();
      module->sendText(MSG_INIT_DONE, 200);
      
    });     
  }
}

void initDisplay( void ) {
  char message[100];
  oledDisplay->setTitle(config->getName());
  if(gsmEnabled) oledDisplay->gsmIcon(BLINKING);  
  wifiDisplay();
  timeDisplay();
}

void timeDisplay() {
  oledDisplay->clockIcon(!ntpServerInitialized);
  
  // TODO: if no Home wifi, no NTP, => test if  GSM enabled and use its time
  
  int millisec = millis();
  if(ntpServerInitialized && millisec > config->getDefaultAPExposition()) {
    oledDisplay->refreshDateTime(NTP.getTimeDateString().c_str());
  } else {
    char message[10];
    sprintf(message, "%d", millisec/1000);
    oledDisplay->refreshDateTime(message);
    
  }
}

void wifiDisplay() {
  char message[100];
  WifiType wifiType = AP;
  
  if(config->isAPInitialized()) {
    oledDisplay->setLine(1, "", NOT_TRANSIENT, NOT_BLINKING);
  } else {
    oledDisplay->setLine(1, MSG_INIT_REQUEST, NOT_TRANSIENT, BLINKING);
  }
    
  if(displayAP && homeWifiConnected) {
    strcpy(message, config->getHomeSsid());     
    strcat(message, " ");
    ipOnHomeSsid.getBytes((byte *)message + strlen(message), 50);
    oledDisplay->setLine(0, message);
  } else {
    strcpy(message, config->getApSsid());
    strcat(message, " ");
    IPAddress ipAddress = WiFi.softAPIP();
    ipAddress.toString().getBytes((byte *)message + strlen(message), 50);
    oledDisplay->setLine(0, message);
  }
  displayAP = !displayAP;
  
  bool blinkWifi = false;
  if (!homeWifiConnected && config->isHomeWifiConfigured()) {
    blinkWifi = true;
  }
  
  if(config->isHomeWifiConfigured()) {
    wifiType = AP_STA;
  } 
  oledDisplay->wifiIcon(blinkWifi, wifiType);
}



/*********************************
 * Main Loop
 *********************************/
void loop() {
  now();  // Needed to refresh the Time lib, so that NTP server is called
  // X seconds after reset, switch to custom AP if set
  if(defaultAP && (millis() > config->getDefaultAPExposition()) && config->isAPInitialized()) {
    defaultAP = false;
    initSoftAP();
  }
  
  // check if any new added slave needs to be renamed
  if(slaveToRename != NULL) {
    slaveCollection->renameOne(slaveToRename);
    slaveToRename = NULL;
  }
  
  // Check if any request to serve
  server->handleClient();
 
  // Let gsm do its tasks: checking connection, incomming messages, 
  // handler notifications...
  gsm.refresh();   
  
  // Display needs to be refreshed periodically to handle blinking
  oledDisplay->refresh();

  // Time on display should be refreshed every second
  // Intentionnally not using the value returned by now(), since it changes
  // when time is set.  
  timeNow = millis();
  
  if(timeNow - timeLastTimeDisplay >= 1000) {
    timeLastTimeDisplay = timeNow;
    timeDisplay();
  }
  if(timeNow - timeLastWifiDisplay >= 3500) {
    timeLastWifiDisplay = timeNow;
    // refresh wifi display every Xs to display both ssid/ips alternatively
    wifiDisplay();    
  }
  
  // TODO: ping every ? minute only      
  if(timeNow - timeLastPing >= 60000) {
    timeLastPing = timeNow; 
    slaveCollection->ping();
  } 
  
  delay(20);
  
}