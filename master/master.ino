
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

// include "html.h"
#include "masterConfig.h"


// Global object to store config
masterConfigDataType masterConfigData;
MasterConfigClass *config;

ESP8266WebServer server(80);
MDNSResponder mdns;
byte clientConnected = 0;
boolean homeWifiConnected = false;

void setup(){

  Serial.begin(9600);
  delay(2000); // delay to connect serial monitor
  config = new MasterConfigClass((unsigned int)CONFIG_VERSION, (char*)CONFIG_NAME, (void*)&masterConfigData);
  server.on("/", [](){
    Serial.println("Rq on /");
    printInitPage();
  });

  Serial.println("Init wifi");
  WiFi.mode(WIFI_AP);
  WiFi.softAP("iotinator", "iotinator");

  server.begin();
  Serial.println(WiFi.softAPIP());
  config->init();

  Serial.print("Name ");
  Serial.println(config->getName());
  for(int i = 0; i < MAX_PHONE_NUMBERS; i++) {
    Serial.print("Numero ");
    Serial.print(i);
    Serial.print(" ");
    Serial.println(config->getRegisteredPhone(i)->getNumber());
  }
}

void loop() {
  server.handleClient();
}

void printInitPage() {
  server.send(200, "text/html", "<html><body>This is init page.</body></html>");
  server.on("/toto", [](){
    Serial.println("Rq on /toto");
    server.send(200, "text/html", "<html><body>Hi again.</body></html>");
  });
}
