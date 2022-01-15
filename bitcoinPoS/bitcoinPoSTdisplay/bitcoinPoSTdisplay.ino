#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
using WebServerClass = WebServer;
fs::SPIFFSFS &FlashFS = SPIFFS;
#define FORMAT_ON_FAIL true

#include <Keypad.h>
#include <AutoConnect.h>
#include <SPI.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <Hash.h>
#include <ArduinoJson.h>
#include "qrcode.h"
#include "Bitcoin.h"
#include "esp_adc_cal.h"

#define PARAM_FILE "/elements.json"
#define KEY_FILE "/thekey.txt"

//Variables
String inputs;
String thePin;
String nosats;
String cntr = "0";
String lnurl;
String currency;
String lncurrency;
String key;
String preparedURL;
String baseURL;
String apPassword = "ToTheMoon1"; //default WiFi AP password
String masterKey;
String lnbitsServer;
String invoice;
String lnbitsBaseURL;
String secret;
String dataIn = "0";
String amountToShow = "0.00";
String noSats = "0";
String qrData;
String dataId;
String addressNo;
String key_val;
bool onchainCheck = false;
bool lnCheck = false;
bool lnurlCheck = false;
bool unConfirmed = true;
int randomPin;
int calNum = 1;
int sumFlag = 0;
int converted = 0;

//Custom access point pages
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
      "name": "password",
      "type": "ACInput",
      "label": "Password for PoS AP WiFi",
      "value": "ToTheMoon1"
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
      "name": "lncurrency",
      "type": "ACInput",
      "label": "PoS Currency"
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
SHA256 h;
TFT_eSPI tft = TFT_eSPI();

const byte rows = 4; //four rows
const byte cols = 3; //three columns
char keys[rows][cols] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

byte rowPins[rows] = {21, 27, 26, 22}; //connect to the row pinouts of the keypad
byte colPins[cols] = {33, 32, 25}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);
int checker = 0;
char maxdig[20];

WebServerClass server;
AutoConnect portal(server);
AutoConnectConfig config;
AutoConnectAux elementsAux;
AutoConnectAux saveAux;

void setup()
{

  Serial.begin(115200);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  //Load screen
  tft.init();
  tft.invertDisplay(false);
  tft.setRotation(1);
  tft.invertDisplay(true);
  logo();

  //Load buttons
  h.begin();
  FlashFS.begin(FORMAT_ON_FAIL);
  SPIFFS.begin(true);

  //Get the saved details and store in global variables
  File paramFile = FlashFS.open(PARAM_FILE, "r");
  if (paramFile)
  {
    StaticJsonDocument<2000> doc;
    DeserializationError error = deserializeJson(doc, paramFile.readString());

    JsonObject passRoot = doc[0];
    const char *apPasswordChar = passRoot["value"];
    const char *apNameChar = passRoot["name"];
    if (String(apPasswordChar) != "" && String(apNameChar) == "password")
    {
      apPassword = apPasswordChar;
    }
    JsonObject maRoot = doc[1];
    const char *masterKeyChar = maRoot["value"];
    masterKey = masterKeyChar;
    if (masterKey != "")
    {
      onchainCheck = true;
    }
    JsonObject serverRoot = doc[2];
    const char *serverChar = serverRoot["value"];
    lnbitsServer = serverChar;
    JsonObject invoiceRoot = doc[3];
    const char *invoiceChar = invoiceRoot["value"];
    invoice = invoiceChar;
    if (invoice != "")
    {
      lnCheck = true;
    }
    JsonObject lncurrencyRoot = doc[4];
    const char *lncurrencyChar = lncurrencyRoot["value"];
    lncurrency = lncurrencyChar;
    JsonObject baseURLRoot = doc[5];
    const char *baseURLChar = baseURLRoot["value"];
    lnbitsBaseURL = baseURLChar;
    JsonObject secretRoot = doc[6];
    const char *secretChar = secretRoot["value"];
    secret = secretChar;
    JsonObject currencyRoot = doc[7];
    const char *currencyChar = currencyRoot["value"];
    currency = currencyChar;
    if (secret != "")
    {
      lnurlCheck = true;
    }
  }
  paramFile.close();

  //Handle access point traffic
  server.on("/", []() {
    String content = "<h1>bitcoinPoS</br>Free open-source bitcoin PoS</h1>";
    content += AUTOCONNECT_LINK(COG_24);
    server.send(200, "text/html", content);
  });

  elementsAux.load(FPSTR(PAGE_ELEMENTS));
  elementsAux.on([](AutoConnectAux &aux, PageArgument &arg) {
    File param = FlashFS.open(PARAM_FILE, "r");
    if (param)
    {
      aux.loadElement(param, {"password", "masterkey", "server", "invoice", "lncurrency", "baseurl", "secret", "currency"});
      param.close();
    }
    if (portal.where() == "/posconfig")
    {
      File param = FlashFS.open(PARAM_FILE, "r");
      if (param)
      {
        aux.loadElement(param, {"password", "masterkey", "server", "invoice", "lncurrency", "baseurl", "secret", "currency"});
        param.close();
      }
    }
    return String();
  });

  saveAux.load(FPSTR(PAGE_SAVE));
  saveAux.on([](AutoConnectAux &aux, PageArgument &arg) {
    aux["caption"].value = PARAM_FILE;
    File param = FlashFS.open(PARAM_FILE, "w");
    if (param)
    {
      // Save as a loadable set for parameters.
      elementsAux.saveElement(param, {"password", "masterkey", "server", "invoice", "lncurrency", "baseurl", "secret", "currency"});
      param.close();
      // Read the saved elements again to display.
      param = FlashFS.open(PARAM_FILE, "r");
      aux["echo"].value = param.readString();
      param.close();
    }
    else
    {
      aux["echo"].value = "Filesystem failed to open.";
    }
    return String();
  });

  config.auth = AC_AUTH_BASIC;
  config.authScope = AC_AUTHSCOPE_AUX;
  config.ticker = true;
  config.autoReconnect = true;
  config.apid = "bitcoinPoS-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  config.psk = apPassword;
  config.menuItems = AC_MENUITEM_CONFIGNEW | AC_MENUITEM_OPENSSIDS | AC_MENUITEM_RESET;
  config.reconnectInterval = 1;
  int timer = 0;

  //Give few seconds to trigger portal
  while (timer < 2000)
  {
    char key = keypad.getKey();
    if (key != NO_KEY)
    {
      portalLaunch();
      config.immediateStart = true;
      portal.join({elementsAux, saveAux});
      portal.config(config);
      portal.begin();
      while (true)
      {
        portal.handleClient();
      }
    }
    timer = timer + 200;
    delay(200);
  }
  if (lnCheck)
  {
    portal.join({elementsAux, saveAux});
    config.autoRise = false;
    portal.config(config);
    portal.begin();
  }
}

void loop()
{
  noSats = "0";
  dataIn = "0";
  amountToShow = "0";
  unConfirmed = true;
  String choices[3][2] = {{"onchain", masterKey}, {"ln", lnbitsServer}, {"lnurl", lnbitsBaseURL}};
  int menuItem;
  int menuItems = 0;
  for (int i = 0; i < 3; i++)
  {
    if (choices[i][1] != "")
    {
      menuItem = i;
      menuItems++;
    }
  }
  //If only one payment method available skip menu
  if (menuItems < 1)
  {
    error("NO METHODS", "RESTART, RUN PORTAL");
    delay(100000);
  }
  else if (menuItems == 1)
  {
    if (choices[menuItem][0] == "onchain")
    {
      onchainMain();
    }
    if (choices[menuItem][0] == "ln")
    {
      lnMain();
    }
    if (choices[menuItem][0] == "lnurl")
    {
      lnurlMain();
    }
  }
  //If more than one payment method available trigger menu
  else
  {
    choiceMenu();

    while (unConfirmed)
    {
      char key = keypad.getKey();
      if (key != NO_KEY)
      {
        if (String(key) == "1" && onchainCheck)
        {
          onchainMain();
        }
        
        if (String(key) == "2" && lnCheck && WiFi.status() == WL_CONNECTED)
        {
          lnMain();
        }
        if (String(key) == "3" && lnurlCheck)
        {
          lnurlMain();
        }
      }
      delay(100);
    }
  }
}

void getKeypad(bool isLN)
{

          dataIn += key_val;
          if (isLN)
          {
            isLNMoneyNumber(false);
          }
          else
          {
            isLNURLMoneyNumber(false);
          }
}

//Onchain payment method
void onchainMain()
{
  File file = SPIFFS.open(KEY_FILE);
  if (file)
  {
    addressNo = file.readString();
    addressNo = String(addressNo.toInt() + 1);
    file.close();
    file = SPIFFS.open(KEY_FILE, FILE_WRITE);
    file.print(addressNo);
    file.close();
  }
  else
  {
    file.close();
    file = SPIFFS.open(KEY_FILE, FILE_WRITE);
    addressNo = "1";
    file.print(addressNo);
    file.close();
  }
  Serial.println(addressNo);
  inputScreenOnChain();
  while (unConfirmed)
  {
    char key = keypad.getKey();
    if (key != NO_KEY)
    {
      key_val = String(key);
      if (key_val == "*")
      {
        unConfirmed = false;
      }
      if (key_val == "#")
      {
        HDPublicKey hd(masterKey);
        String path = String("m/0/") + addressNo;
        qrData = hd.derive(path).address();
        qrShowCodeOnchain(true, "  A CANCEL       C CHECK");
        while (unConfirmed)
        {
          char key = keypad.getKey();
          if (key != NO_KEY)
          {
            key_val = String(key);
            if (key_val == "*")
            {
              unConfirmed = false;
            }
            if (key_val == "#")
            {
              while (unConfirmed)
              {
                qrData = "https://mempool.space/address/" + qrData;
                qrShowCodeOnchain(false, "  A CANCEL");
                while (unConfirmed)
                {
                  char key = keypad.getKey();
                  if (key != NO_KEY)
                  {
                    key_val = String(key);
                    if (key_val == "*")
                    {
                      unConfirmed = false;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}
void lnMain()
{
  if (converted == 0)
  {
    choiceMenu();
    getSats();
  }
  inputScreen(true);
  while (unConfirmed)
  {
    char key = keypad.getKey();
    if (key != NO_KEY)
    {
      key_val = String(key);
      if (key_val == "*")
      {
        unConfirmed = false;
      }
      if (key_val == "#")
      {
        processing("FETCHING INVOICE");
        getInvoice();
        delay(1000);
        qrShowCodeln();
        while (unConfirmed)
        {
          int timer = 0;
          unConfirmed = checkInvoice();
          if (!unConfirmed)
          {
            complete();
            timer = 5000;
            delay(3000);
          }
          while (timer < 4000)
          {
            char key = keypad.getKey();
            if (key != NO_KEY)
            {
              key_val = String(key);
              if (key_val == "*")
              {
                noSats = "0";
                dataIn = "0";
                amountToShow = "0";
                unConfirmed = false;
                timer = 5000;
              }
            }
            delay(200);
            timer = timer + 100;
          }
        }
        noSats = "0";
        dataIn = "0";
        amountToShow = "0";
      }
      getKeypad(true);
    }
    delay(100);
  }
}
void lnurlMain()
{
  inputScreen(false);
  inputs = "";
  while (unConfirmed)
  {
    char key = keypad.getKey();
    if (key != NO_KEY)
    {
      key_val = String(key);
      if (key_val == "*")
      {
        unConfirmed = false;
      }
      else if (key_val == "#")
      {
        makeLNURL();
        qrShowCodeLNURL("   CLEAR       SHOW PIN");
        while (unConfirmed)
        {
          char key = keypad.getKey();
          if (key != NO_KEY)
          {
            key_val = String(key);
            if (key_val == "#")
            {
              showPin();
              while (unConfirmed)
              {
                char key = keypad.getKey();
                if (key != NO_KEY)
                {
                  key_val = String(key);
                  if (key_val == "*")
                  {
                    unConfirmed = false;
                  }
                }
              }
            }
            if (key_val == "*")
            {
              unConfirmed = false;
            }
          }
        }
      }
      getKeypad(false);
    }
    
    delay(100);
  }
}

void portalLaunch()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_PURPLE, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(15, 50);
  tft.println("AP LAUNCHED");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(0, 100);
  tft.setTextSize(2);
  tft.println("WHEN FINISHED RESET");
  
}

void isLNMoneyNumber(bool cleared)
{
  if (!cleared)
  {
    amountToShow = String(dataIn.toFloat() / 100);
    noSats = String((converted / 100) * dataIn.toFloat());
  }
  else
  {
    noSats = "0";
    dataIn = "0";
    amountToShow = "0";
  }
  tft.setTextSize(3);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setCursor(88, 10);
  tft.println(amountToShow);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(105, 35);
  tft.println(noSats.toInt());
}

void isLNURLMoneyNumber(bool cleared)
{
  if (!cleared)
  {
    amountToShow = String(dataIn.toFloat() / 100);
  }
  else
  {
    dataIn = "0";
    amountToShow = "0";
  }
  tft.setTextSize(3);
  tft.setCursor(100, 120);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(88, 40);
  tft.println(amountToShow);
}

///////////DISPLAY///////////////
/////Lightning//////

void inputScreen(bool online)
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(0, 10);
  if (online)
  {
    tft.println(" " + String(lncurrency) + ": ");
    tft.println(" SATS: ");
    tft.println("");
    tft.println("");
  }
  else{
    tft.println(" " + String(currency) + ": ");
    tft.println("");
  }
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 80);
  tft.println("*. Reset");
  tft.println("#. Generate invoice");
}

void inputScreenOnChain()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(0, 40);
  tft.println("XPUB ENDING IN " + masterKey.substring(masterKey.length() - 5));
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 80);
  tft.println("*. Reset");
  tft.println("#. Generate fresh address");
}

void qrShowCodeln()
{
  tft.fillScreen(TFT_WHITE);
  qrData.toUpperCase();
  const char *qrDataChar = qrData.c_str();
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(20)];
  qrcode_initText(&qrcode, qrcodeData, 11, 0, qrDataChar);
  for (uint8_t y = 0; y < qrcode.size; y++)
  {
    // Each horizontal module
    for (uint8_t x = 0; x < qrcode.size; x++)
    {
      if (qrcode_getModule(&qrcode, x, y))
      {
        tft.fillRect(65 + 2 * x, 5 + 2 * y, 2, 2, TFT_BLACK);
      }
      else
      {
        tft.fillRect(65 + 2 * x, 5 + 2 * y, 2, 2, TFT_WHITE);
      }
    }
  }
  tft.setCursor(0, 220);
  tft.setTextSize(2);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.print("  A CANCEL");
}

void qrShowCodeOnchain(bool anAddress, String message)
{
  tft.fillScreen(TFT_WHITE);
  if (anAddress)
  {
    qrData.toUpperCase();
  }
  const char *qrDataChar = qrData.c_str();
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(20)];
  int pixSize = 0;
  tft.setCursor(0, 200);
  tft.setTextSize(2);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  if (anAddress)
  {
    qrcode_initText(&qrcode, qrcodeData, 2, 0, qrDataChar);
    pixSize = 3;
    tft.println("     onchain address");
  }
  else
  {
    qrcode_initText(&qrcode, qrcodeData, 6, 0, qrDataChar);
    pixSize = 2;
    tft.println("     mempool.space link");
  }
  for (uint8_t y = 0; y < qrcode.size; y++)
  {
    // Each horizontal module
    for (uint8_t x = 0; x < qrcode.size; x++)
    {
      if (qrcode_getModule(&qrcode, x, y))
      {
        tft.fillRect(65 + pixSize * x, 5 + pixSize * y, pixSize, pixSize, TFT_BLACK);
      }
      else
      {
        tft.fillRect(65 + pixSize * x, 5 + pixSize * y, pixSize, pixSize, TFT_WHITE);
      }
    }
  }
  tft.println(message);
}

void qrShowCodeLNURL(String message)
{
  tft.fillScreen(TFT_WHITE);
  qrData.toUpperCase();
  const char *qrDataChar = qrData.c_str();
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(20)];
  qrcode_initText(&qrcode, qrcodeData, 6, 0, qrDataChar);
  for (uint8_t y = 0; y < qrcode.size; y++)
  {
    // Each horizontal module
    for (uint8_t x = 0; x < qrcode.size; x++)
    {
      if (qrcode_getModule(&qrcode, x, y))
      {
        tft.fillRect(65 + 3 * x, 5 + 3 * y, 3, 3, TFT_BLACK);
      }
      else
      {
        tft.fillRect(65 + 3 * x, 5 + 3 * y, 3, 3, TFT_WHITE);
      }
    }
  }
  tft.setCursor(0, 220);
  tft.setTextSize(2);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.println(message);
}

void error(String message, String additional)
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(20, 50);
  tft.println(message);
  if (additional != "")
  {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(0, 60);
    tft.setTextSize(2);
    tft.println(additional);
  }
}

void processing(String message)
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 50);
  tft.println(message);
}

void complete()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(4);
  tft.setCursor(60, 100);
  tft.println("COMPLETE");
}
void choiceMenu()
{
  tft.setTextSize(2);
  tft.fillScreen(TFT_BLACK);
  if (!onchainCheck)
  {
    tft.setTextColor(TFT_RED, TFT_BLACK);
tft.setCursor(0, 35);
    tft.println("1. Onchain");
  }
  else
  {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.setCursor(0, 35);
    tft.println("1. Onchain");
  }
  if (!lnCheck)
  {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("2. LN (WiFi needed)");
  }
  else if (lnCheck && WiFi.status() != WL_CONNECTED)
  {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("2. LN (WiFi needed)");
  }
  else
  {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("2. LN");
  }
  if (!lnurlCheck)
  {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("3. LN Offline");
  }
  else
  {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("3. LN Offline");
  }
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(0, 220);
  tft.setTextSize(2);
  tft.println("     1       2       3");
}
void showPin()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(40, 5);
  tft.println("PROOF PIN");
  tft.setCursor(70, 60);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(4);
  tft.println(randomPin);
}

void lnurlInputScreen()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // White characters on black background
  tft.setTextSize(3);
  tft.setCursor(0, 50);
  tft.println("AMOUNT THEN #");
  tft.setCursor(50, 220);
  tft.setTextSize(2);
  tft.println("TO RESET PRESS *");
  tft.setTextSize(3);
  tft.setCursor(0, 130);
  tft.print(String(currency) + ":");
}

void logo()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextSize(4);
  tft.setCursor(0, 30);
  tft.print("bitcoin");
  tft.setTextColor(TFT_PURPLE, TFT_BLACK);
  tft.print("PoS");
  tft.setTextSize(2);
  tft.setCursor(0, 80);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print("Powered by LNbits");
}

void to_upper(char *arr)
{
  for (size_t i = 0; i < strlen(arr); i++)
  {
    if (arr[i] >= 'a' && arr[i] <= 'z')
    {
      arr[i] = arr[i] - 'a' + 'A';
    }
  }
}

void callback()
{
}
//////////LIGHTNING//////////////////////
void getSats()
{
  WiFiClientSecure client;
  lnbitsServer.toLowerCase();
  Serial.println(lnbitsServer);
  if (lnbitsServer.substring(0, 8) == "https://")
  {
    Serial.println(lnbitsServer.substring(8, lnbitsServer.length()));
    lnbitsServer = lnbitsServer.substring(8, lnbitsServer.length());
  }
  //client.setInsecure(); //Some versions of WiFiClientSecure need this
  const char *lnbitsServerChar = lnbitsServer.c_str();
  const char *invoiceChar = invoice.c_str();
  const char *lncurrencyChar = lncurrency.c_str();

  if (!client.connect(lnbitsServerChar, 443))
  {
    Serial.println("failed");
    error("SERVER DOWN", "");
    delay(3000);
  }

  String toPost = "{\"amount\" : 1, \"unit\" :\"" + String(lncurrencyChar) + "\"}";
  String url = "/api/v1/conversion";
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + String(lnbitsServerChar) + "\r\n" +
               "User-Agent: ESP32\r\n" +
               "X-Api-Key: " + String(invoiceChar) + " \r\n" +
               "Content-Type: application/json\r\n" +
               "Connection: close\r\n" +
               "Content-Length: " + toPost.length() + "\r\n" +
               "\r\n" +
               toPost + "\n");

  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    if (line == "\r")
    {
      break;
    }
    if (line == "\r")
    {
      break;
    }
  }
  String convertedStr = client.readString();
  converted = convertedStr.toInt();
}

void getInvoice()
{
  WiFiClientSecure client;
  lnbitsServer.toLowerCase();
  if (lnbitsServer.substring(0, 8) == "https://")
  {
    lnbitsServer = lnbitsServer.substring(8, lnbitsServer.length());
  }
  //client.setInsecure(); //Some versions of WiFiClientSecure need this
  const char *lnbitsServerChar = lnbitsServer.c_str();
  const char *invoiceChar = invoice.c_str();

  if (!client.connect(lnbitsServerChar, 443))
  {
    Serial.println("failed");
    error("SERVER DOWN", "");
    delay(3000);
    return;
  }

  String toPost = "{\"out\": false,\"amount\" : " + String(noSats.toInt()) + ", \"memo\" :\"bitcoinPoS-" + String(random(1, 1000)) + "\"}";
  String url = "/api/v1/payments";
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + lnbitsServerChar + "\r\n" +
               "User-Agent: ESP32\r\n" +
               "X-Api-Key: " + invoiceChar + " \r\n" +
               "Content-Type: application/json\r\n" +
               "Connection: close\r\n" +
               "Content-Length: " + toPost.length() + "\r\n" +
               "\r\n" +
               toPost + "\n");

  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    Serial.println(line);
    if (line == "\r")
    {
      break;
    }
    if (line == "\r")
    {
      break;
    }
  }
  String line = client.readString();

  StaticJsonDocument<1000> doc;
  DeserializationError error = deserializeJson(doc, line);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  const char *payment_hash = doc["checking_id"];
  const char *payment_request = doc["payment_request"];
  qrData = payment_request;
  dataId = payment_hash;
  Serial.println(qrData);
}

bool checkInvoice()
{
  WiFiClientSecure client;
  //client.setInsecure(); //Some versions of WiFiClientSecure need this
  const char *lnbitsServerChar = lnbitsServer.c_str();
  const char *invoiceChar = invoice.c_str();
  if (!client.connect(lnbitsServerChar, 443))
  {
    error("SERVER DOWN", "");
    delay(3000);
    return false;
  }

  String url = "/api/v1/payments/";
  client.print(String("GET ") + url + dataId + " HTTP/1.1\r\n" +
               "Host: " + lnbitsServerChar + "\r\n" +
               "User-Agent: ESP32\r\n" +
               "X-Api-Key:" + invoiceChar + "\r\n" +
               "Content-Type: application/json\r\n" +
               "Connection: close\r\n\r\n");
  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    if (line == "\r")
    {
      break;
    }
    if (line == "\r")
    {
      break;
    }
  }
  String line = client.readString();
  Serial.println(line);
  StaticJsonDocument<500> doc;
  DeserializationError error = deserializeJson(doc, line);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return false;
  }
  bool boolPaid = doc["paid"];
  if (boolPaid)
  {
    unConfirmed = false;
  }
  return unConfirmed;
}

//////////LNURL AND CRYPTO///////////////

void makeLNURL()
{
  randomPin = random(1000, 9999);
  byte nonce[8];
  for (int i = 0; i < 8; i++)
  {
    nonce[i] = random(256);
  }
  byte payload[51]; // 51 bytes is max one can get with xor-encryption
  size_t payload_len = xor_encrypt(payload, sizeof(payload), (uint8_t *)secret.c_str(), secret.length(), nonce, sizeof(nonce), randomPin, dataIn.toInt());
  preparedURL = lnbitsBaseURL + "?p=";
  preparedURL += toBase64(payload, payload_len, BASE64_URLSAFE | BASE64_NOPADDING);
  Serial.println(preparedURL);
  char Buf[200];
  preparedURL.toCharArray(Buf, 200);
  char *url = Buf;
  byte *data = (byte *)calloc(strlen(url) * 2, sizeof(byte));
  size_t len = 0;
  int res = convert_bits(data, &len, 5, (byte *)url, strlen(url), 8, 1);
  char *charLnurl = (char *)calloc(strlen(url) * 2, sizeof(byte));
  bech32_encode(charLnurl, "lnurl", data, len);
  to_upper(charLnurl);
  qrData = charLnurl;
  Serial.println(qrData);
}

int xor_encrypt(uint8_t *output, size_t outlen, uint8_t *key, size_t keylen, uint8_t *nonce, size_t nonce_len, uint64_t pin, uint64_t amount_in_cents)
{
  // check we have space for all the data:
  // <variant_byte><len|nonce><len|payload:{pin}{amount}><hmac>
  if (outlen < 2 + nonce_len + 1 + lenVarInt(pin) + 1 + lenVarInt(amount_in_cents) + 8)
  {
    return 0;
  }
  int cur = 0;
  output[cur] = 1; // variant: XOR encryption
  cur++;
  // nonce_len | nonce
  output[cur] = nonce_len;
  cur++;
  memcpy(output + cur, nonce, nonce_len);
  cur += nonce_len;
  // payload, unxored first - <pin><currency byte><amount>
  int payload_len = lenVarInt(pin) + 1 + lenVarInt(amount_in_cents);
  output[cur] = (uint8_t)payload_len;
  cur++;
  uint8_t *payload = output + cur;                                 // pointer to the start of the payload
  cur += writeVarInt(pin, output + cur, outlen - cur);             // pin code
  cur += writeVarInt(amount_in_cents, output + cur, outlen - cur); // amount
  cur++;
  // xor it with round key
  uint8_t hmacresult[32];
  SHA256 h;
  h.beginHMAC(key, keylen);
  h.write((uint8_t *)"Round secret:", 13);
  h.write(nonce, nonce_len);
  h.endHMAC(hmacresult);
  for (int i = 0; i < payload_len; i++)
  {
    payload[i] = payload[i] ^ hmacresult[i];
  }
  // add hmac to authenticate
  h.beginHMAC(key, keylen);
  h.write((uint8_t *)"Data:", 5);
  h.write(output, cur);
  h.endHMAC(hmacresult);
  memcpy(output + cur, hmacresult, 8);
  cur += 8;
  // return number of bytes written to the output
  return cur;
}
