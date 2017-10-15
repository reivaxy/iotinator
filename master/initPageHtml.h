/**
 *  iotinator master module init page 
 *  Xavier Grosjean 2017
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
 #include "messages.h"

char initPage[] = "\
<html title='iotinator'>\
<body>\
<h1>" MSG_INIT_WELCOME "</h1>\
<form action='/init' method='post'>\
  <input name='admin' type='text' placeholder='" MSG_INIT_ADMIN_PHONE_NUMBER "'/><br/>\
  <input name='apSsid' type='text' placeholder='" MSG_INIT_AP_SSID "'/><br/>\
  <input name='apPwd' type='text' placeholder='" MSG_INIT_AP_PWD "'/><br/>\
  <input type='submit'/>\
</form>\
</body>\
</html>\
";