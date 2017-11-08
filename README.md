# iotinator
The global multipurpose home iot solution.

This is the master module of the iotinator framework.


It runs on esp8266 hardware (Investigating esp32 hardware too), connected to a SIM800 chip and an Oled screen, and offers an interface to slave modules in order to monitor them through SMS and webApps.

It connects to a domestic Wifi network and exposes a "private" Wifi network to communicate with slave modules.

It defines APIs to communicate with module, and serves as forwards SMS to them, and sends their responses back.

Configuration is persisted in EEPROM.


Picture of the early prototype:

<img src="resources/prototype.jpg" width="400px"/>

