#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
using WebServerClass = WebServer;
fs::SPIFFSFS& FlashFS = SPIFFS;
#define FORMAT_ON_FAIL  true

#include <AutoConnect.h>

#define PARAM_FILE      "/elements.json"
#define USERNAME        "bitcoinPoS"
#define PASSWORD        "ToTheMoon"

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
  Serial.println();

  FlashFS.begin(FORMAT_ON_FAIL);
  server.on("/", []() {
    String content = "Place the root page with the sketch application.&ensp;";
    content += AUTOCONNECT_LINK(COG_24);
    server.send(200, "text/html", content);
  });

  elementsAux.load(FPSTR(PAGE_ELEMENTS));
  elementsAux.on([] (AutoConnectAux& aux, PageArgument& arg) {
    if (portal.where() == "/posconfig") {
      File param = FlashFS.open(PARAM_FILE, "r");
      if (param) {
        aux.loadElement(param, { "pin", "masterkey", "server", "invoice", "baseurl", "secret", "currency"} );
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
      // Save as a loadable set for parameters.
      elementsAux.saveElement(param, { "pin", "masterkey", "server", "invoice", "baseurl", "secret", "currency"});
      param.close();
      // Read the saved elements again to display.
      param = FlashFS.open(PARAM_FILE, "r");
      aux["echo"].value = param.readString();
      
      param.close();
    }
    else {
      aux["echo"].value = "Filesystem failed to open.";
    }
    Serial.println(String());
    return String();
  });
  poo();
  Serial.println(PAGE_ELEMENTS[1]);
  portal.join({ elementsAux, saveAux });
  config.auth = AC_AUTH_BASIC;
  config.authScope = AC_AUTHSCOPE_AUX;
  config.username = USERNAME;
  config.password = PASSWORD;
  config.ticker = true;
  config.autoReconnect = true;
  config.reconnectInterval = 1;
  portal.config(config);
  portal.begin();
  

  Serial.println("cunt");
}

void loop() {
  portal.handleClient();
}

void poo(){
  Serial.println("mother-fucker");
}
