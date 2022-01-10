#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
using WebServerClass = WebServer;
fs::SPIFFSFS& FlashFS = SPIFFS;
#define FORMAT_ON_FAIL  true

#include <AutoConnect.h>

#define PARAM_FILE "/elements.json"
#define PIN_FILE "/pin.txt"

String thePin;
static const char PAGE_ELEMENTS[] PROGMEM = R"(
{
  "uri": "/posconfig",
  "title": "PoS Options",
  "menu": true,
  "element": [

    {
      "name": "text",
      "type": "ACText",
      "value": "bitcoinPoS options",
      "style": "font-family:Arial;font-size:16px;font-weight:400;color:#191970;margin-botom:15px;"
    },
    {
      "name": "pin",
      "type": "ACInput",
      "label": "PoS Admin Pin",
      "value": "1989",
      "apply": "number",
      "pattern": "\\d*"
    },

    {
      "name": "offline",
      "type": "ACText",
      "value": "Onchain *optional",
      "style": "font-family:Arial;font-size:16px;font-weight:400;color:#191970;margin-botom:15px;"
    },
    {
      "name": "masterkey",
      "type": "ACInput",
      "label": "Master Public Key"
    },

    {
      "name": "lightning1",
      "type": "ACText",
      "value": "Lightning *optional",
      "style": "font-family:Arial;font-size:16px;font-weight:400;color:#191970;margin-botom:15px;"
    },
    {
      "name": "server",
      "type": "ACInput",
      "label": "LNbits Server"
    },
    {
      "name": "invoice",
      "type": "ACInput",
      "label": "Wallet Invoice Key"
    },
    {
      "name": "lightning2",
      "type": "ACText",
      "value": "Offline Lightning *optional",
      "style": "font-family:Arial;font-size:16px;font-weight:400;color:#191970;margin-botom:15px;"
    },
    {
      "name": "baseurl",
      "type": "ACInput",
      "label": "LNURLPoS BaseURL"
    },
    {
      "name": "secret",
      "type": "ACInput",
      "label": "LNURLPoS Secret"
    },
    {
      "name": "currency",
      "type": "ACInput",
      "label": "LNURLPoS Currency"
    },
    {
      "name": "load",
      "type": "ACSubmit",
      "value": "Load",
      "uri": "/posconfig"
    },
    {
      "name": "save",
      "type": "ACSubmit",
      "value": "Save",
      "uri": "/save"
    },
    {
      "name": "adjust_width",
      "type": "ACElement",
      "value": "<script type='text/javascript'>window.onload=function(){var t=document.querySelectorAll('input[]');for(i=0;i<t.length;i++){var e=t[i].getAttribute('placeholder');e&&t[i].setAttribute('size',e.length*.8)}};</script>"
    }
  ]
 }
)";

static const char PAGE_SAVE[] PROGMEM = R"(
{
  "uri": "/save",
  "title": "Elements",
  "menu": false,
  "element": [
    {
      "name": "caption",
      "type": "ACText",
      "format": "Elements have been saved to %s",
      "style": "font-family:Arial;font-size:18px;font-weight:400;color:#191970"
    },
    {
      "name": "validated",
      "type": "ACText",
      "style": "color:red"
    },
    {
      "name": "echo",
      "type": "ACText",
      "style": "font-family:monospace;font-size:small;white-space:pre;"
    },
    {
      "name": "ok",
      "type": "ACSubmit",
      "value": "OK",
      "uri": "/posconfig"
    }
  ]
}
)";

WebServerClass  server;
AutoConnect portal(server);
AutoConnectConfig config;
AutoConnectAux  elementsAux;
AutoConnectAux  saveAux;


void setup() {
  delay(1000);
  Serial.begin(115200);

  FlashFS.begin(FORMAT_ON_FAIL);
  
  File paramFile = FlashFS.open(PARAM_FILE, "r");
  Serial.println(paramFile.readString());
  paramFile.close();
  
  File pinFile = FlashFS.open(PIN_FILE, "r");
  Serial.println(pinFile.readString());
  pinFile.close();
  
  File pinfile = FlashFS.open(PIN_FILE, "w");
    if (pinfile) {
      thePin = pinfile.readString();
      Serial.println(thePin);
    }
    else{
      pinfile.print("1989");
    }
  
  pinfile.close();
  server.on("/", []() {
    String content = "<h1>bitcoinPoS</br>Free open-source bitcoin PoS</h1>";
    content += AUTOCONNECT_LINK(COG_24);
    server.send(200, "text/html", content);
  });
  
  elementsAux.on([] (AutoConnectAux& aux, PageArgument& arg) {
    if (portal.where() == "/posconfig") {
      Serial.println("cunt");
      File param = FlashFS.open(PARAM_FILE, "r");
      if (param) {
        aux.loadElement(param, { "text", "pin", "masterkey", "server", "invoice", "baseurl", "secret", "currency"} );
        param.close();
      }
    }
    return String();
  });

  saveAux.load(FPSTR(PAGE_SAVE));
  saveAux.on([] (AutoConnectAux& aux, PageArgument& arg) {
    aux["caption"].value = PARAM_FILE;

    File param = FlashFS.open(PARAM_FILE, "w");
    if (param) {
      
      File pinfile = FlashFS.open(PIN_FILE, "w");
      pinfile.print(arg.arg("pin").toInt());
      pinfile.close();
      
      // Save as a loadable set for parameters.
      elementsAux.saveElement(param, { "text", "pin", "masterkey", "server", "invoice", "baseurl", "secret", "currency"});
      param.close();
      // Read the saved elements again to display.
      param = FlashFS.open(PARAM_FILE, "r");
      aux["echo"].value = param.readString();
      
      param.close();
    }
    else {
      aux["echo"].value = "Filesystem failed to open.";
    }

    return String();
  });
  

  portal.join({ elementsAux, saveAux });
  config.auth = AC_AUTH_BASIC;
  config.authScope = AC_AUTHSCOPE_AUX;
  config.ticker = true;
  config.autoReconnect = true;
  config.apid = "bitcoinPoS-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  config.psk = "ToTheMoon";
  config.menuItems = AC_MENUITEM_CONFIGNEW | AC_MENUITEM_OPENSSIDS | AC_MENUITEM_RESET;
  config.reconnectInterval = 1;
  portal.config(config);
  portal.begin();
}

void loop() {
  portal.handleClient();
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return;
    }

    Serial.println("- read from file:");
    while(file.available()){
        Serial.write(file.read());
    }
}
