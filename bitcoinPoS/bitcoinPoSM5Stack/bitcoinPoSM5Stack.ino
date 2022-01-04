
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
using WebServerClass = WebServer;
fs::SPIFFSFS& FlashFS = SPIFFS;
#define FORMAT_ON_FAIL  true

#include <AutoConnect.h>

#include <M5Stack.h>
#define KEYBOARD_I2C_ADDR     0X08
#define KEYBOARD_INT          5

#include <string.h>
#include "Bitcoin.h"
#include <Hash.h>
#include <Conversion.h>
#include <WiFi.h>
#include "esp_adc_cal.h"

#define PARAM_FILE "/elements.json"
#define DEVICENAME "bitcoinPoS"
#define PASSWORD "satoshi1" //Must be 8 characters


//CHANGE TO HIGHER IF USING TOR
int QRCodeComplexity = 8;

//////////////VARIABLES///////////////////
String dataId = "";
bool paid = false;
bool shouldSaveConfig = false;
bool down = false;
const char *spiffcontent = "";
String spiffing;
String lnurl;
String choice;
String payhash;
String key_val;
String cntr = "0";
String inputs;
int keysdec;
int keyssdec;
float temp;
String fiat;
float satoshis;
String nosats;
float conversion;
String virtkey;
String payreq;
int randomPin;
bool settle = false;
String preparedURL;
RTC_DATA_ATTR int bootCount = 0;
long timeOfLastInteraction = millis();
bool isPretendSleeping = false;


static const char PAGE_ELEMENTS[] PROGMEM = R"(
{
  "uri": "/posconfig",
  "title": "PoS Options",
  "menu": true,
  "element": [
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
      "style": "font-family:Arial;font-size:16px;font-weight:400;color:#191970;padding:15px;"
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
      "style": "font-family:Arial;font-size:16px;font-weight:400;color:#191970;padding:15px;"
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
      "style": "font-family:Arial;font-size:16px;font-weight:400;color:#191970;padding:15px;"
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
  M5.begin();
  Wire.begin();
  
  logo();

  FlashFS.begin(FORMAT_ON_FAIL);
  server.on("/", []() {
    String content = "Place the root page with the sketch application.&ensp;";
    content += AUTOCONNECT_LINK(COG_24);
    server.send(200, "text/html", content);
  });

  elementsAux.load(FPSTR(PAGE_ELEMENTS));
  elementsAux.on([] (AutoConnectAux& aux, PageArgument& arg) {
    if (portal.where() == "/posconfig") {
      Serial.println("cunt");
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
    return String();
  });

  portal.join({ elementsAux, saveAux });
  config.auth = AC_AUTH_BASIC;
  config.authScope = AC_AUTHSCOPE_AUX;
  config.ticker = true;
  config.title = DEVICENAME;
  config.apid = DEVICENAME;
  config.psk  = PASSWORD;
  config.autoReconnect = true;
  config.reconnectInterval = 1;
  config.immediateStart = true;
  portal.config(config);
}

void loop() {
  int count = 0;
  while (count < 3000){
    M5.update();
    get_keypad(); 
    if (M5.BtnA.wasReleased()) {
      accessPoint();
      portal.begin();
      while(true){
        portal.handleClient();
      }
    }
    count = count + 100;
    delay(100);
  }
  Serial.println("cunttyty");
}

void get_keypad()
{
   if(digitalRead(KEYBOARD_INT) == LOW) {
    Wire.requestFrom(KEYBOARD_I2C_ADDR, 1);  // request 1 byte from keyboard
    while (Wire.available()) { 
       uint8_t key = Wire.read();                  // receive a byte as character
       key_val = key;
       if(key != 0) {
        if(key >= 0x20 && key < 0x7F) { // ASCII String
          if (isdigit((char)key)){
          key_val = ((char)key);
          }
          else {
          key_val = "";
        } 
        }
      }
    }
  }
}

///////////DISPLAY///////////////

void logo()
{
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK); // White characters on black background
  M5.Lcd.setTextSize(5);
  M5.Lcd.setCursor(40, 100);  // To be compatible with Adafruit_GFX the cursor datum is always bottom left
  M5.Lcd.print("LNURLPoS"); // Using tft.print means text background is NEVER rendered
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(42, 140);          // To be compatible with Adafruit_GFX the cursor datum is always bottom left
  M5.Lcd.print("Powered by LNbits"); // Using tft.print means text background is NEVER rendered
}

void accessPoint()
{
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK); // White characters on black background
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(40, 100);  // To be compatible with Adafruit_GFX the cursor datum is always bottom left
  M5.Lcd.print("Enter Pin"); // Using tft.print means text background is NEVER rendered
}
