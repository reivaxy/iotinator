/**
 *  iotinator master module 
 *  Xavier Grosjean 2017
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */

// This is used to conditionally compile master related stuff in XIOTModule
// Should ease the enhancement that will make any agent a secondary master
// to work around the limit of how many agents can connect to the master wifi ssid
#define XIOT_MASTER 1
 
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <stdio.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <XIOTDisplay.h>

#include "MasterConfig.h"
#include "MasterModule.h" 

#include "initPageHtml.h"
#include "appLoader.h"
#include "appGLALoader.h"

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
bool homeWifiFirstConnected = false;
NTPSyncEvent_t ntpEvent;
bool ntpEventToProcess = false;
bool ntpServerInitialized = false;
bool ntpTimeInitialized = false;
unsigned long elapsed200ms = 0;
unsigned long elapsed500ms = 0;
unsigned long elapsed2s = 0;
bool gsmEnabled = false;
MDNSResponder mdns;
time_t timeNow = 0; 
time_t timeLastTimeDisplay = 0;
time_t timeLastWifiDisplay = 0;

// Temporary: handling of new UI
// TODO ?: have webside provide a manifest with available UIs
// TODO ?: have master download and serve the loader page of the chosen UI
char glaCss1[50];
char glaCss2[50];
char glaJs1[50];
char glaJs2[50];

int scl = 12;
int sda = 14;


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
bool ntpListenerInitialized = false;
String ipOnHomeSsid;

void setup() {

  //#define ESP01  // Uncomment this to use an ESP01 as master.
  #ifdef ESP01
  Serial.begin(115200,SERIAL_8N1,SERIAL_TX_ONLY);  // Serial used only to send messages, not read => reuse output
  scl = 2;
  sda = 0;
  #else
  Serial.begin(115200);
  #endif

  WiFi.mode(WIFI_OFF);

  config = new MasterConfigClass((unsigned int)CONFIG_VERSION, (char*)MODULE_NAME);
  config->init();

  initDisplay();
 
  module = new MasterModule(config, 0x3C, sda, scl);

  // Master endpoints need to be set first (when same endpoints: only first one set is called)
  // TODO XIOTMODULE Check this 
  addEndpoints();
  module->addModuleEndpoints();
  
  // After a reset, open Default Access Point
  // If Access Point was customized, we'll switch to it after one minute
  // This is supposed to give agent modules time to initialize.
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
  NTP.onNTPSyncEvent([](NTPSyncEvent_t event) {
    Serial.printf("NTP event: %d\n", event);
    ntpEventToProcess = true;
    ntpEvent = event;
  });
     
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
  oledDisplay->setLine(1, MSG_WIFI_STATION_DISCONNECTED, TRANSIENT, NOT_BLINKING);
  // TODO: remove it from agent collection ?
  // Disconnection needs a long time to be triggered (15mn ?)
  // May be periodic ping should be enough ?
}

// Called when STA is connected to home wifi and IP was obtained
void onSTAGotIP (WiFiEventStationModeGotIP ipInfo) {
  ipOnHomeSsid = ipInfo.ip.toString();
  Serial.printf("Got IP on %s: %s\n", config->getHomeSsid(), ipOnHomeSsid.c_str());
  homeWifiConnected = true;
  if(module->isWaitingOTA()) {
    char message[40];
    sprintf(message, "Ota master ready: %s", ipOnHomeSsid.c_str());
    oledDisplay->setLine(0, message, NOT_TRANSIENT, NOT_BLINKING); 
    ArduinoOTA.begin();    
    return;
  }
  homeWifiFirstConnected = true;
  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
  wifiDisplay();
}

void initNtp() {
  if(ntpServerInitialized) return;
  ntpServerInitialized = true;
  Serial.printf("Fetching time from %s\n", config->getNtpServer());
  NTP.begin(config->getNtpServer());
  NTP.setInterval(63, 7200);  // 63s retry, 2h refresh
  NTP.setTimeZone(config->getGmtHourOffset(), config->getGmtMinOffset());
}

void processNtpEvent() {
  if (ntpEvent) {
    Serial.print("NTP Time Sync error: ");
    if (ntpEvent == noResponse)
      Serial.println("NTP server not reachable");
    else if (ntpEvent == invalidAddress)
      Serial.println("Invalid NTP server address");
  } else {
    Serial.print("Got NTP time: ");
    Serial.println(NTP.getTimeDateString(NTP.getLastNTPSync()));
    ntpTimeInitialized = true;
    timeDisplay();
    NTP.setInterval(7200, 7200);  // 5h retry, 2h refresh. once we have time, refresh failure is not critical
  }
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
  server->on("/", HTTP_GET, [](){
    if (config->isAPInitialized()) {
      if(server->arg("app") == "gla") {
        printAppGLAPage();
      } else {
        printAppPage();
      }
    } else {
      printHomePage();
    }
  });

  server->on("/init", HTTP_GET, []() {
    printHomePage();
  });

  /**
   * This API returns the SSID and PWD of the customized Access Point: modules will use it to connect to iotinator
   * NB: only 4 clients  can connect
   * => may be agent modules could also create an access point and act as relay for
   * other modules.
   * TODO: this will need to be moved to XIOModule to allow agent to be secondary masters
   **/
  server->on("/api/config", HTTP_GET, [](){
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
    root[XIOTModuleJsonTag::timeInitialized] = ntpTimeInitialized;
    root.printTo(configMsg, JSON_STRING_CONFIG_SIZE);
    module->sendJson(configMsg, 200);
  });

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

// For now, use specific methods to load each app.
// Later: website could provide a manifest exposing all available apps, and their dependencies,
// and make this generic.
void printAppPage() {
  char *page = (char *)malloc(strlen(appLoader) + strlen(config->getWebSite()) + 1);
  sprintf(page, appLoader, config->getWebSite());
  module->sendHtml(page, 200);
  free(page);
}

void printAppGLAPage() {
  char *page = (char *)malloc(strlen(appGLALoader) 
  + strlen(config->getWebSite()) 
  + strlen(glaCss1) 
  + strlen(glaCss2) 
  + strlen(glaJs1) 
  + strlen(glaJs2) 
  + 1);
  sprintf(page, appGLALoader, config->getWebSite(), glaCss1, glaCss2, glaJs1, glaJs2);
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
    server->on("/initSave",  HTTP_POST, [](){
      Serial.println("Rq on /initSave");
      
      // TODO: /initSave might need to be disabled once done ?
      if(false && config->isAPInitialized()) {
        module->sendHtml(MSG_ERR_ALREADY_INITIALIZED, 403);
        return;
      }
      
      // TODO add some controls
      if (false && !server->hasArg("apSsid")) {
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
      String webSite = server->arg("webSite");
      if (webSite.length() > 0) {
        // TODO: add checks
        config->setWebSite(webSite);
      }
      // Read and save the api key      
      String apiKey = server->arg("apiKey");
      if (apiKey.length() > 0) {
        // TODO: add checks
        config->setApiKey(apiKey);
      }
            
      // Read and save the ntp host      
      String ntpHost = server->arg("ntpHost");
      if (ntpHost.length() > 0) {
        // TODO: add checks
        config->setNtpServer(ntpHost);
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
  oledDisplay->clockIcon(!ntpTimeInitialized);
  
  // TODO: if no Home wifi, no NTP, => test if  GSM enabled and use its time
  
  time_t millisec = millis();
  if(ntpTimeInitialized && millisec > config->getDefaultAPExposition()) {
    oledDisplay->refreshDateTime(NTP.getTimeDateString().c_str());
  } else {
    char message[10];
    sprintf(message, "%d", millisec/1000);
    oledDisplay->refreshDateTime(message);
    
  }
}

// Registration will allow to autodiscover the master module by opening the page http://my.iotinator.com
// The server will return the manifest for the new web app (app1) so that the master module can 
// generate the bootstrap page to load the app.
void registerToWebsite() {
  if(!checkApiKey()) return; 

  const size_t bufferSize = JSON_OBJECT_SIZE(6) + 100; 
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& root = jsonBuffer.createObject();  
  root["name"] = config->getName();
  root["apikey"] = config->getApiKey(); 
  root["ip"] =  ipOnHomeSsid;
  
  char macAddrStr[100];
  uint8_t macAddr[6];
  WiFi.macAddress(macAddr);
  sprintf(macAddrStr, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]);
  
  root["mac"] = macAddrStr;
  oledDisplay->setLine(1, "Registering to website", TRANSIENT, NOT_BLINKING);
  char* response = (char *)malloc(201);
  *response = 0;
  char body[1001];
  int httpCode;
   
  root.printTo(body, 1000);
// TODO: investigate why this call does not work:    
//  module->APIPost(config->getWebSite(), "/my/register.php", body, &httpCode, response, 200);

  HTTPClient http;
  char host[200];
  sprintf(host, "%s/my/register.php", config->getWebSite());
  http.begin(host);
  httpCode = http.POST(body);
  if(httpCode <= 0) {
    Serial.printf("HTTP POST failed, error: %s\n", http.errorToString(httpCode).c_str());
    return;
  }

  String jsonResultStr = http.getString();
  http.end();
  if(httpCode != 200) {
    Serial.println("Registration failed");
    Serial.println(jsonResultStr); // it's a json string, actually
    return;
  }
  Serial.println("Registered");
  // Got the manifest. For now, just one App, hardcoded. Later, could be all available apps... etc.
  // Buffer size: https://arduinojson.org/assistant
  const int bufferSize1 = 3*JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(5) + 400;
  StaticJsonBuffer<bufferSize1> jsonBuffer1; 
  JsonObject& root1 = jsonBuffer1.parseObject(jsonResultStr);
  
  strlcpy(glaCss1, root1["vendor"]["css"], sizeof(glaCss1));   
  Serial.println(glaCss1);
  strlcpy(glaCss2, root1["main"]["css"], sizeof(glaCss2));   
  Serial.println(glaCss2);
  strlcpy(glaJs1, root1["vendor"]["js"], sizeof(glaJs1));   
  Serial.println(glaJs1);
  strlcpy(glaJs2, root1["main"]["js"], sizeof(glaJs2));   
  Serial.println(glaJs2);
  
}

unsigned char checkApiKey() {
  if(strlen(config->getApiKey()) == 0) {
    Serial.println("No Api Key provided.");
    return 0;
  } else {
    Serial.println("Api Key is defined.");
    return 1;
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

  if(module->isWaitingOTA()) {
    oledDisplay->refresh();
    ArduinoOTA.handle();
    return;
  }
    
  if(ntpEventToProcess) {
    ntpEventToProcess = false;
    processNtpEvent();
  }

  now();  // Needed to refresh the Time lib, so that NTP server is called
  // X seconds after reset, switch to custom AP if set
  if(defaultAP && (millis() > config->getDefaultAPExposition()) && config->isAPInitialized()) {
    defaultAP = false;
    initSoftAP();
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
 
  
  // Things to do only once after connection to internet.
  if(homeWifiFirstConnected) {
  // Init ntp   
    initNtp();
    registerToWebsite();
    homeWifiFirstConnected = false;
  }
  delay(20);
  
}