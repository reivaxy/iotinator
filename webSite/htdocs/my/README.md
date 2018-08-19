# my.iotinator.com


The purpose of this directory is to provide access to the webApp (or the future mobile app) of a master iotinator module without knowing its local IP on the home network.

For convenience, I've set up the iotinator.com webserver so that the 'my' subdomain is redirected to this www.iotinator.com/my directory.

For the webApp, just type http://my.iotinator.com in your browser and it there is one iotinator master module on the same network as yours, you'll be redirected to its webApp page.

The request to http://my.iotinator.com will check in a database if there is one iotinator master module registered for the current IP addess sent, which should be the IP of the home network gateway (dsl box, ...)

If one master module is found, the browser is redirected to the registered IP, which should show the master module's application page.

If none is found, an error message says so.

If several are found, which is unlikely, a list of modules with their names, ips, a date of registration is displayed for the user to pick one.

For (future) mobile application calls http://my.iotinator.com/jsonModules.php to recieve a json array with the registered iotinator master modules on the same network.

Remaining to code:

To achieve this feature, the iotinator master, once connected to the home wifi network, registers itself to the iotinator.com DB

It needs to provide an API key, which requires a user registration on the iotinator.com website


Issues to solve: removing a module from the DB... not really urgent. Who would want to turn one off anyway.