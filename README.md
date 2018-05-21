# iotinator
The global multipurpose home iot solution.

This is the master module of the iotinator framework.

Its goal is to control many different devices by SMS or through a webApp (or mobile app), it's the common entry (and exit) point of communication.

Example of such devices are: light or heater switch, weather station, water detector, smoke detector, movement detector, aquarium monitoring (much like https://github.com/reivaxy/aquaMonitor).

It runs on esp8266 hardware (will investigate esp32 hardware too), connected to a SIM900 chip and an Oled screen, and offers an interface to slave modules in order to monitor them through SMS and webApps.

It defines APIs to communicate with the modules, and dispatches and forwards messages received by SMS to them, and sends their responses back.

It also provides an API so that modules can record logs, stats, etc on a website, and expose a web UI for module configuration and status diplay.

It connects to a domestic Wifi network and exposes a "private" Wifi network to communicate with slave modules.

Configuration is persisted in EEPROM, it uses network autodiscovery, (re)connection management is automatic.


Waiting to solve stability issues with cheap GSM boards, gsm handling has been disabled. Current time is fetched from NTP servers once Home Wifi is configured and connected.

GSM will be an option, the framework being able to work just using internet.

But GSM is nice in case of power outage (modules can have batteries, but my box doesn't), and also to handle security
(sending codes to validate authentication for instance)

You'll need to clone these repositories into your arduino "libraries" directory to be able to compile:

- https://github.com/reivaxy/XOLEDDisplay.git 
- https://github.com/reivaxy/XIOTDisplay.git
- https://github.com/reivaxy/XEEPROMConfig.git
- https://github.com/reivaxy/XUtils.git
- https://github.com/reivaxy/XIOTModule.git

And some libraries that can be found through the Arduino IDE

TODO: list the libraries :)

Picture of the early prototype:

<img src="resources/prototype.jpg" width="400px"/>

Picture of the swarm: one master, two slaves:

<img src="resources/swarm.jpg" width="400px"/>

Screen shot of the first draft of the webApp, showing 2 connected modules, and the status of the Switch module (no UI yet for Xenia module)

The data is refreshed every 10 seconds and reflects the status of the modules. It's only display for now, actions will come soon:

<img src="resources/webApp.png" width="400px"/>