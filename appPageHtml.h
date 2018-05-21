/**
 *  iotinator master module web app main page 
 *  Xavier Grosjean 2018
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
char appPage[] = "\
<html title='Iotinator'>\
<head>\
<meta charset='utf-8'>\
<base href='" DEFAULT_WEBAPP_HOST "'>\
<link href='app/css/app.css' rel='stylesheet'>\
<link href='app/css/locale.css' rel='stylesheet'>\
<meta name='viewport' content='width=device-width, initial-scale=1'>\
</head>\
<body>\
<div id='app'>\
{{ message }}\
</div>\
<script src='https://cdn.jsdelivr.net/npm/vue'></script>\
<script src='app/js/app.js'></script>\
</body>\
</html>\
";