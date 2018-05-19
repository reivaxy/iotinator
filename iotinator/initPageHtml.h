/**
 *  iotinator master module init page 
 *  Xavier Grosjean 2017
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
 #include "messages.h"

char initPage[] = "\
<html title='iotinator'>\
<head>\
<style>\
body.noGsm .ifGsm {\
  display:none;\
}\
</style>\
</head>\
<body class='%s'>\
<h1>" MSG_INIT_WELCOME "</h1>\
<form action='/init' method='post'>\
  <input class='ifGsm' name='admin' type='text' placeholder='" MSG_INIT_ADMIN_PHONE_NUMBER "'/><br/>\
  <input name='apSsid' type='text' placeholder='" MSG_INIT_AP_SSID "'/><br/>\
  <input name='apPwd' type='text' placeholder='" MSG_INIT_AP_PWD "'/><br/>\
  <input name='homeSsid' type='text' placeholder='" MSG_INIT_HOME_SSID "'/><br/>\
  <input name='homePwd' type='text' placeholder='" MSG_INIT_HOME_PWD "'/><br/>\
  <input type='submit'/>\
</form>\
</body>\
</html>\
";