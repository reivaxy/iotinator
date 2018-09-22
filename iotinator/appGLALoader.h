/**
 *  iotinator master module GLA web app loader page 
 *  Guillaume Labat 2018
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */
 
char appGLALoader[] = "\
<!DOCTYPE html>\
<html lang='en'>\
<head>\
  <meta charset='utf-8'>\
  <base href='%s/appGLA/'>\
  <title>iotinator UI experiments</title>\
  <link rel='stylesheet' href='https://use.fontawesome.com/releases/v5.3.1/css/all.css' integrity='sha384-mzrmE5qonljUremFsqc01SB46JvROS7bZs3IO2EmfFsd15uHvIt+Y8vEf7N7fWAU'\
    crossorigin='anonymous'>\
  <link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/bulma/0.7.1/css/bulma.min.css'>\
  <link href='%s' rel='stylesheet'>\
  <link href='%s' rel='stylesheet'>\
</head>\
<body>\
  <h1 class='title'>\
    iotinator\
  </h1>\
  <div id='app'></div>\
<script type='text/javascript' src='https://unpkg.com/react@16.5.2/umd/react.production.min.js'></script>\
<script type='text/javascript' src='https://unpkg.com/mobx@4.4.0/lib/mobx.umd.min.js'></script>\
<script type='text/javascript' src='https://unpkg.com/react-dom@16.5.2/umd/react-dom.production.min.js'></script>\
<script type='text/javascript' src='%s'></script>\
<script type='text/javascript' src='%s'></script>\
</body>\
</html>\
";