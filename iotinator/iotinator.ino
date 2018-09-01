/**
 *  iotinator master module 
 *  Xavier Grosjean 2017
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <stdio.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <XIOTDisplay.h>
#include <XIOTModule.h> 

#include "masterConfig.h"
#include "AgentCollection.h"

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
time_t timeLastPing = 0;
AgentCollection *agentCollection;
Agent* agentToRename = NULL;


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

  //#define ESP01
  #ifdef ESP01
  Serial.begin(115200,SERIAL_8N1,SERIAL_TX_ONLY); 
  scl = 2;
  sda = 0;
  #endif

  WiFi.mode(WIFI_OFF);
  Serial.begin(9600);
  delay(100);
  config = new MasterConfigClass((unsigned int)CONFIG_VERSION, (char*)MODULE_NAME);
  config->init();
  Serial.println(config->getName());

  // Initialise the OLED display
  oledDisplay = new DisplayClass(0x3C, sda, scl);
  initDisplay();
  
  // TODO: this implemenation is crap. It uses some of the module features, but not others...
  module = new XIOTModule(oledDisplay);
  // Master endpoints need to be set first (when same endpoints: only first one set is called)
  addEndpoints();
  module->addModuleEndpoints();
  
  // Initialize the Agent Collection
  agentCollection = new AgentCollection(module);
  
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
      printAppPage();
    } else {
      printHomePage();
    }
  });

  server->on("/init", HTTP_GET, [](){
    printHomePage();
  });

  server->on("/api/list", HTTP_GET, [](){
    int size = agentCollection->getCount();
    int customStrSize = 0;
    
    // Size estimation: https://arduinojson.org/assistant/
    // TODO: update this when necessary : 10 fields per agent (for now it's actually 8)
    const size_t bufferSize = size*JSON_OBJECT_SIZE(10)
                              + JSON_OBJECT_SIZE(size)
                              +  + JSON_OBJECT_SIZE(1) ;
    
    DynamicJsonBuffer jsonBuffer(bufferSize);
    JsonObject& root = jsonBuffer.createObject();    
    JsonObject& agentList = root.createNestedObject("agentList");
    
    agentCollection->list(agentList, &customStrSize);
 
    char* strBuffer = (char *)malloc(customStrSize); 
    root.printTo(strBuffer, customStrSize-1);
    Serial.printf("Reserved size: %d, actual size: %d\n", customStrSize, strlen(strBuffer));
    module->sendJson(strBuffer, 200);
    free(strBuffer); 

    uint32_t freeMem = system_get_free_heap_size();
    Serial.printf("%s After /api/list Free heap mem: %d\n", NTP.getTimeDateString().c_str(), freeMem);   
  });
  
  // TODO: remove duplicated code with XIOTModule !!
  server->on("/api/rename", HTTP_POST, [&]() {
    char *forwardTo;
    XUtils::stringToCharP(server->header("Xiot-forward-to"), &forwardTo);
    String jsonBody = server->arg("plain");
    char message[100];
    
    // I've seen a few unexplained parsing error so I have set a bigger buffer size...
    const int bufferSize = 2* JSON_OBJECT_SIZE(2);
    StaticJsonBuffer<bufferSize> jsonBuffer; 
    JsonObject& root = jsonBuffer.parseObject(jsonBody); 
    if (!root.success()) {
      module->sendJson("{}", 500);
      if(strlen(forwardTo) != 0) { 
        oledDisplay->setLine(1, "Renaming agent failed", TRANSIENT, NOT_BLINKING);
      } else {
        oledDisplay->setLine(1, "Renaming master failed", TRANSIENT, NOT_BLINKING);
      }
      free(forwardTo);
      return;
    }
    // Forward the rename to an agent
    if(strlen(forwardTo) != 0) {     
      agentCollection->renameAgent(forwardTo, (const char*)root["name"]);
      free(forwardTo);
    } else {
      free(forwardTo);    
      if(config == NULL) {
        module->sendJson("{\"error\": \"No config to update.\"}", 404);
        return;
      }
      sprintf(message, "Renaming master to %s\n", (const char*)root["name"] ); 
      oledDisplay->setLine(1, message, TRANSIENT, NOT_BLINKING);
      config->setName((const char*)root["name"]);
      config->saveToEeprom(); // TODO: partial save !!   
      oledDisplay->setTitle(config->getName());
    }    
    module->sendJson("{}", 200);   // HTTP code 200 is enough
  });  
  /**
   * This API returns the SSID and PWD of the customized Access Point: modules will use it to connect to iotinator
   * NB: only 4 clients  can connect (TODO: to check!) 
   * => may be agent modules could also create an access point and act as relay for
   * other modules.
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

  /**
   * This endpoint allows agent modules to register themselves to master when they initialize
   */
  server->on("/api/register", HTTP_POST, [](){
    char *jsonString;
    Serial.println("Registering module");
    // This will allocate jsonString
    XUtils::stringToCharP(server->arg("plain"), &jsonString);
    // agentCollection->add method need to copy the data since jsonString will be freed.
    Serial.println(jsonString); 
    Agent* agent = agentCollection->add(jsonString);
    free(jsonString);
    if(agent == NULL) {
      module->sendJson("{}", 500);
      oledDisplay->setLine(1, "Registration failed", TRANSIENT, NOT_BLINKING);
    } else {
      module->sendJson("{}", 200);
      if(agent->getToRename()) {
        agentToRename = agent;
      }
    }
    char message[100];
    sprintf(message, "Registered modules: %d", agentCollection->getCount());
    oledDisplay->setLine(2, message, NOT_TRANSIENT, NOT_BLINKING);    
  });

  /**
   * This endpoint allows removing a module
   */
  server->on("/api/register", HTTP_DELETE, [](){
    char *jsonString;
    Serial.println("Unregistering module");

  });

  // This endpoint is used by modules when they want to update data in the agent collection
  // (which is the data that the UI is polling)
  server->on("/api/refresh", HTTP_POST, [](){
    char *jsonString;
    Serial.println("Refreshing module");
    // This will allocate jsonString
    XUtils::stringToCharP(server->arg("plain"), &jsonString);
    // agentCollection->add method need to copy the data since jsonString will be freed.
    Serial.println(jsonString); 
    Agent* agent = agentCollection->refresh(jsonString);
    free(jsonString);
    if(agent == NULL) {
      module->sendJson("{}", 500);
      oledDisplay->setLine(1, "Refreshing failed", TRANSIENT, NOT_BLINKING);
    } else {
      module->sendJson("{}", 200);
    }          
  });
  
  // TODO: remove this or make it better. Needed during dev
  // reset may be only possible by SMS from admin number ?
  server->on("/api/swarmReset",  HTTP_GET, [](){
    Serial.println("Rq on /swarmReset");
    agentCollection->reset();
    config->initFromDefault();
    config->saveToEeprom();
    module->sendJson("{}", 200);
    gsm.sendSMS(config->getAdminNumber(), "Reset done");  // 
    WiFi.mode(WIFI_AP);
    initSoftAP();  
  });

  // OTA: update 
  server->on("/api/ota", HTTP_POST, [&]() {
    String forwardTo = server->header("Xiot-forward-to");
    String jsonBody = server->arg("plain");
    int httpCode = 200;
    if(forwardTo.length() != 0) {    
      Serial.print("Forwarding ota to ");
      Serial.println(forwardTo);
      char message[SSID_MAX_LENGTH + PWD_MAX_LENGTH + 40];
      sprintf(message, "{\"%s\":\"%s\",\"%s\":\"%s\"}", XIOTModuleJsonTag::ssid, config->getHomeSsid(), XIOTModuleJsonTag::pwd, config->getHomePwd());
      module->APIPost(forwardTo, "/api/ota", message, &httpCode, NULL, 0);
    } else {
      WiFi.mode(WIFI_OFF);
      delay(400);
      WiFi.mode(WIFI_STA);   
      httpCode = module->startOTA(config->getHomeSsid(), config->getHomePwd());
//      httpCode = module->startOTA("", "");
    }
    module->sendJson("{}", httpCode);      
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
  char *page = (char *)malloc(strlen(appPage) + strlen(config->getWebSite()) + 1);
  sprintf(page, appPage, config->getWebSite());
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

void registerToWebsite() {
  if(!checkApiKey()) return; 

  const size_t bufferSize = JSON_OBJECT_SIZE(6); 
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
  } else {
    Serial.println("Registered");
  }
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
  
  // check if any new added agent needs to be renamed
  if(agentToRename != NULL) {
    agentCollection->autoRename(agentToRename);
    agentToRename = NULL;
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
      
  if(timeNow - timeLastPing >= MIN_PING_PERIOD*1000) {
    timeLastPing = timeNow; 
    agentCollection->ping();
    uint32_t freeMem = system_get_free_heap_size();
    Serial.printf("%s After ping Free heap mem: %d\n", NTP.getTimeDateString().c_str(), freeMem);  
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