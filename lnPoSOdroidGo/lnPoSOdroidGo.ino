#include <odroid_go.h>
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
#include <Hash.h>
#include <ArduinoJson.h>
#include "qrcoded.h"
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
String baseURLPoS;
String secretPoS;
String currencyPoS;
String baseURLATM;
String secretATM;
String currencyATM;
String lnurlATMMS;
String dataIn = "0";
String amountToShow = "0.00";
String noSats = "0";
String qrData;
String dataId;
String addressNo;
String pinToShow;
char menuItems[4][12] = {"LNPoS", "LNURLPoS", "OnChain", "LNURLATM"};
int menuItemCheck[4] = {0, 0, 0, 0};
String selection;
int menuItemNo = 0;
int randomPin;
int calNum = 1;
int sumFlag = 0;
int converted = 0;
String key_val;
bool onchainCheck = false;
bool lnCheck = false;
bool lnurlCheck = false;
bool unConfirmed = true;
bool selected = false;
bool lnurlCheckPoS = false;
bool lnurlCheckATM = false;
String lnurlATMPin;

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
      "value": "LNPoS options",
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
      "name": "heading1",
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
      "label": "PoS Currency ie EUR"
    },
    {
      "name": "heading2",
      "type": "ACText",
      "value": "Offline Lightning *optional",
      "style": "font-family:Arial;font-size:16px;font-weight:400;color:#191970;margin-botom:15px;"
    },
    {
      "name": "lnurlpos",
      "type": "ACInput",
      "label": "LNURLPoS String"
    },
    {
      "name": "heading3",
      "type": "ACText",
      "value": "Offline Lightning *optional",
      "style": "font-family:Arial;font-size:16px;font-weight:400;color:#191970;margin-botom:15px;"
    },
    {
      "name": "lnurlatm",
      "type": "ACInput",
      "label": "LNURLATM String"
    },
    {
      "name": "lnurlatmms",
      "type": "ACInput",
      "value": "mempool.space",
      "label": "mempool.space server"
    },
    {
      "name": "lnurlatmpin",
      "type": "ACInput",
      "value": "878787",
      "label": "LNURLATM pin String"
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

const byte rows = 4; //four rows
const byte cols = 3; //three columns
char keys[rows][cols] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

byte rowPins[rows] = {12, 14, 27, 26}; //connect to the row pinouts of the keypad
byte colPins[cols] = {25, 33, 32};     //connect to the column pinouts of the keypad

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
  GO.begin();

  // Serial.begin(115200);
  // pinMode(4, OUTPUT);
  // digitalWrite(4, HIGH);

  //Load screen
  // tft.init();
  // tft.invertDisplay(false);
  // tft.setRotation(1);
  logo();

  //Load buttons
  h.begin();
  FlashFS.begin(FORMAT_ON_FAIL);
  SPIFFS.begin(true);
  //Get the saved details and store in global variables
  File paramFile = FlashFS.open(PARAM_FILE, "r");
  if (paramFile)
  {
    StaticJsonDocument<2500> doc;
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
      menuItemCheck[2] = 1;
    }
    JsonObject serverRoot = doc[2];
    const char *serverChar = serverRoot["value"];
    lnbitsServer = serverChar;
    JsonObject invoiceRoot = doc[3];
    const char *invoiceChar = invoiceRoot["value"];
    invoice = invoiceChar;
    if (invoice != "")
    {
      menuItemCheck[0] = 1;
    }
    JsonObject lncurrencyRoot = doc[4];
    const char *lncurrencyChar = lncurrencyRoot["value"];
    lncurrency = lncurrencyChar;
    JsonObject lnurlPoSRoot = doc[5];
    const char *lnurlPoSChar = lnurlPoSRoot["value"];
    String lnurlPoS = lnurlPoSChar;
    baseURLPoS = getValue(lnurlPoS, ',', 0);
    secretPoS = getValue(lnurlPoS, ',', 1);
    currencyPoS = getValue(lnurlPoS, ',', 2);
    if (secretPoS != "")
    {
      menuItemCheck[1] = 1;
    }
    JsonObject lnurlATMRoot = doc[6];
    const char *lnurlATMChar = lnurlATMRoot["value"];
    String lnurlATM = lnurlATMChar;
    baseURLATM = getValue(lnurlATM, ',', 0);
    secretATM = getValue(lnurlATM, ',', 1);
    currencyATM = getValue(lnurlATM, ',', 2);
    if (secretATM != "")
    {
      menuItemCheck[3] = 1;
    }
    JsonObject lnurlATMMSRoot = doc[7];
    const char *lnurlATMMSChar = lnurlATMMSRoot["value"];
    lnurlATMMS = lnurlATMMSChar;
    JsonObject lnurlATMPinRoot = doc[8];
    const char *lnurlATMPinChar = lnurlATMPinRoot["value"];
    lnurlATMPin = lnurlATMPinChar;
  }
  paramFile.close();

  //Handle access point traffic
  server.on("/", []() {
    String content = "<h1>LNPoS</br>Free open-source bitcoin PoS</h1>";
    content += AUTOCONNECT_LINK(COG_24);
    server.send(200, "text/html", content);
  });

  elementsAux.load(FPSTR(PAGE_ELEMENTS));
  elementsAux.on([](AutoConnectAux &aux, PageArgument &arg) {
    File param = FlashFS.open(PARAM_FILE, "r");
    if (param)
    {
      aux.loadElement(param, {"password", "masterkey", "server", "invoice", "lncurrency", "lnurlpos", "lnurlatm", "lnurlatmms", "lnurlatmpin"});
      param.close();
    }
    if (portal.where() == "/posconfig")
    {
      File param = FlashFS.open(PARAM_FILE, "r");
      if (param)
      {
        aux.loadElement(param, {"password", "masterkey", "server", "invoice", "lncurrency", "lnurlpos", "lnurlatm", "lnurlatmms", "lnurlatmpin"});
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
      elementsAux.saveElement(param, {"password", "masterkey", "server", "invoice", "lncurrency", "lnurlpos", "lnurlatm", "lnurlatmms", "lnurlatmpin"});
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
  config.apid = "PoS-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  config.psk = apPassword;
  config.menuItems = AC_MENUITEM_CONFIGNEW | AC_MENUITEM_OPENSSIDS | AC_MENUITEM_RESET;
  config.reconnectInterval = 1;
  config.title = "LNPoS";
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
  if (menuItemCheck[0])
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
  key_val = "";
  int menuItemsAmount = 0;
  if (WiFi.status() != WL_CONNECTED)
  {
    menuItemCheck[0] = 0;
  }
  for (int i = 0; i < sizeof(menuItems) / sizeof(menuItems[0]); i++)
  {
    if (menuItemCheck[i] == 1)
    {
      menuItemsAmount++;
      selection = menuItems[i];
    }
  }

  //If no methods available
  if (menuItemsAmount < 1)
  {
    error("  NO METHODS", "RESTART & RUN PORTAL");
    delay(10000000);
  }
  //If only one payment method available skip menu
  Serial.println(menuItemsAmount);
  if (menuItemsAmount == 1)
  {
    if (selection == "OnChain")
    {
      onchainMain();
    }
    if (selection == "LNPoS")
    {
      lnMain();
    }
    if (selection == "LNURLPoS")
    {
      lnurlPoSMain();
    }
    if (selection == "LNURLATM")
    {
      lnurlATMMain();
    }
  }
  //If more than one payment method available trigger menu
  else
  {
    while (unConfirmed)
    {
      menuLoop();
      if (selection == "LNPoS")
      {
        lnMain();
      }
      if (selection == "OnChain")
      {
        onchainMain();
      }
      if (selection == "LNURLPoS")
      {
        lnurlPoSMain();
      }
      if (selection == "LNURLATM")
      {
        lnurlATMMain();
      }
      delay(100);
    }
  }
}

// Onchain payment method
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
    key_val = "";
    getKeypad(false, true, false, false);
    if (key_val == "*")
    {
      unConfirmed = false;
    }
    if (key_val == "#")
    {
      HDPublicKey hd(masterKey);
      String path = String("m/0/") + addressNo;
      qrData = hd.derive(path).address();
      qrShowCodeOnchain(true, " *MENU #CHECK");
      while (unConfirmed)
      {
        key_val = "";
        getKeypad(false, true, false, false);
        if (key_val == "*")
        {
          unConfirmed = false;
        }
        if (key_val == "#")
        {
          while (unConfirmed)
          {
            qrData = "https://" + lnurlATMMS + "/address/" + qrData;
            qrShowCodeOnchain(false, " *MENU");
            while (unConfirmed)
            {
              key_val = "";
              getKeypad(false, true, false, false);
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
void lnMain()
{
  if (converted == 0)
  {
    processing("FETCHING FIAT RATE");
    getSats();
  }
  isLNMoneyNumber(true);
  while (unConfirmed)
  {
    key_val = "";
    getKeypad(false, false, true, false);
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
          key_val = "";
          getKeypad(false, true, false, false);
          if (key_val == "*")
          {
            noSats = "0";
            dataIn = "0";
            amountToShow = "0";
            unConfirmed = false;
            timer = 5000;
          }
          delay(200);
          timer = timer + 100;
        }
      }
      noSats = "0";
      dataIn = "0";
      amountToShow = "0";
    }
    delay(100);
  }
}
void lnurlPoSMain()
{
  inputs = "";
  pinToShow = "";
  dataIn = "";
  isLNURLMoneyNumber(true);
  while (unConfirmed)
  {
    key_val = "";
    getKeypad(false, false, false, false);
    if (key_val == "*")
    {
      unConfirmed = false;
    }
    else if (key_val == "#")
    {
      makeLNURL();
      qrShowCodeLNURL(" *MENU #SHOW PIN");
      while (unConfirmed)
      {
        key_val = "";
        getKeypad(false, true, false, false);
        if (key_val == "#")
        {
          showPin();
          while (unConfirmed)
          {
            key_val = "";
            getKeypad(false, true, false, false);
            if (key_val == "*")
            {
              unConfirmed = false;
            }
          }
        }
        if (key_val == "*")
        {
          unConfirmed = false;
        }
      }
    }
    delay(100);
  }
}

void lnurlATMMain()
{
  pinToShow = "";
  dataIn = "";
  isATMMoneyPin(true);
  while (unConfirmed)
  {
    key_val = "";
    getKeypad(true, false, false, false);
    if (key_val == "*")
    {
      unConfirmed = false;
    }
    if (key_val == "#")
    {
      isATMMoneyPin(true);
    }
    if (pinToShow.length() == lnurlATMPin.length() && pinToShow != lnurlATMPin)
    {
      error("  WRONG PIN", "");
      delay(1500);
      pinToShow = "";
      dataIn = "";
      isATMMoneyPin(true);
    }
    else if (pinToShow == lnurlATMPin)
    {
      isATMMoneyNumber(true);
      inputs = "";
      dataIn = "";
      while (unConfirmed)
      {
        key_val = "";
        getKeypad(false, false, false, true);
        if (key_val == "*")
        {
          unConfirmed = false;
        }
        if (key_val == "#")
        {
          makeLNURL();
          qrShowCodeLNURL(" *MENU");
          while (unConfirmed)
          {
            key_val = "";
            getKeypad(false, true, false, false);
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

void getKeypad(bool isATMPin, bool justKey, bool isLN, bool isATMNum)
{
  char key = keypad.getKey();
  if (key != NO_KEY)
  {
    key_val = String(key);
    dataIn += key_val;
    if (isLN)
    {
      isLNMoneyNumber(false);
    }
    else if (isATMPin)
    {
      isATMMoneyPin(false);
    }
    else if (justKey)
    {
    }
    else if (isATMNum)
    {
      isATMMoneyNumber(false);
    }
    else
    {
      isLNURLMoneyNumber(false);
    }
  }
}

void portalLaunch()
{
  GO.lcd.clearDisplay();
  GO.lcd.setTextColor(PURPLE);
  GO.lcd.setTextSize(2);
  GO.lcd.setCursor(15, 50);
  GO.lcd.println("AP LAUNCHED");
  GO.lcd.setTextColor(WHITE);
  GO.lcd.setCursor(0, 100);
  GO.lcd.setTextSize(1);
  GO.lcd.println("WHEN FINISHED RESET");
}

void isLNMoneyNumber(bool cleared)
{
  GO.lcd.clearDisplay();
  GO.lcd.setTextColor(WHITE);
  GO.lcd.setTextSize(1);
  GO.lcd.setCursor(0, 20);
  GO.lcd.print("    ENTER AMOUNT");
  GO.lcd.setTextSize(2);
  GO.lcd.setCursor(0, 50);
  GO.lcd.println(String(lncurrency) + ": ");
  GO.lcd.println("SATS: ");
  GO.lcd.setCursor(0, 120);
  GO.lcd.setTextSize(1);
  GO.lcd.println(" *MENU #INVOICE");
  GO.lcd.setTextSize(2);
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
  GO.lcd.setTextSize(2);
  GO.lcd.setTextColor(RED);
  GO.lcd.setCursor(55, 50);
  GO.lcd.println(amountToShow);
  GO.lcd.setTextColor(GREEN);
  GO.lcd.setCursor(70, 75);
  GO.lcd.println(noSats.toInt());
}

void isLNURLMoneyNumber(bool cleared)
{
  GO.lcd.clearDisplay();
  GO.lcd.setTextColor(WHITE);
  GO.lcd.setTextSize(1);
  GO.lcd.setCursor(0, 20);
  GO.lcd.print("    ENTER AMOUNT");
  GO.lcd.setTextSize(2);
  GO.lcd.setCursor(0, 50);
  GO.lcd.println(String(currencyPoS) + ": ");
  GO.lcd.setCursor(0, 120);
  GO.lcd.setTextSize(1);
  GO.lcd.println(" *MENU #INVOICE");
  GO.lcd.setTextSize(2);
  if (!cleared)
  {
    amountToShow = String(dataIn.toFloat() / 100);
  }
  else
  {
    dataIn = "0";
    amountToShow = "0.00";
  }
  GO.lcd.setTextColor(GREEN);
  GO.lcd.setCursor(55, 50);
  GO.lcd.println(amountToShow);
}

void isATMMoneyNumber(bool cleared)
{
  GO.lcd.clearDisplay();
  GO.lcd.setTextColor(WHITE);
  GO.lcd.setTextSize(1);
  GO.lcd.setCursor(0, 20);
  GO.lcd.print("    ENTER AMOUNT");
  GO.lcd.setTextSize(2);
  GO.lcd.setCursor(0, 50);
  GO.lcd.println(String(currencyATM) + ": ");
  GO.lcd.setCursor(0, 120);
  GO.lcd.setTextSize(1);
  GO.lcd.println(" *MENU #WITHDRAW");
  GO.lcd.setTextSize(2);
  if (!cleared)
  {
    amountToShow = String(dataIn.toFloat() / 100);
  }
  else
  {
    dataIn = "0";
    amountToShow = "0.00";
  }
  GO.lcd.setTextColor(GREEN);
  GO.lcd.setCursor(55, 50);
  GO.lcd.println(amountToShow);
}

void isATMMoneyPin(bool cleared)
{
  GO.lcd.clearDisplay();
  GO.lcd.fillScreen(BLACK);
  GO.lcd.setTextColor(WHITE);
  GO.lcd.setTextSize(1);
  GO.lcd.setCursor(0, 20);
  GO.lcd.print(" ENTER SECRET PIN");
  GO.lcd.setTextSize(2);
  GO.lcd.setCursor(0, 50);
  GO.lcd.println("PIN:");
  GO.lcd.setCursor(0, 120);
  GO.lcd.setTextSize(1);
  GO.lcd.println(" *MENU #CLEAR");
  pinToShow = dataIn;
  GO.lcd.setTextSize(2);
  if (cleared)
  {
    pinToShow = "";
    dataIn = "";
  }
  GO.lcd.setTextColor(GREEN);
  GO.lcd.setCursor(55, 50);
  GO.lcd.println(pinToShow);
}

///////////DISPLAY///////////////
/////Lightning//////

void inputScreenOnChain()
{
  GO.lcd.clearDisplay();
  // tft.fillScreen(BLACK);

  GO.lcd.setTextColor(WHITE);
  GO.lcd.setTextSize(1);
  GO.lcd.setCursor(0, 40);
  GO.lcd.println("XPUB ENDING " + masterKey.substring(masterKey.length() - 5));
  // GO.lcd.setTextColor(WHITE, BLACK);
  GO.lcd.setTextSize(1);
  GO.lcd.setCursor(0, 120);
  GO.lcd.println("*MENU #ADDRESS");
}

void qrShowCodeln()
{
  // tft.fillScreen(TFT_WHITE);
  qrData.toUpperCase();
  const char *qrDataChar = qrData.c_str();
  QRCode qrcoded;
  uint8_t qrcodeData[qrcode_getBufferSize(20)];
  qrcode_initText(&qrcoded, qrcodeData, 11, 0, qrDataChar);
  for (uint8_t y = 0; y < qrcoded.size; y++)
  {
    // Each horizontal module
    for (uint8_t x = 0; x < qrcoded.size; x++)
    {
      if (qrcode_getModule(&qrcoded, x, y))
      {
        GO.lcd.fillRect(20 + 2 * x, 2 + 2 * y, 2, 2, BLACK);
      }
      else
      {
        GO.lcd.fillRect(20 + 2 * x, 2 + 2 * y, 2, 2, WHITE);
      }
    }
  }

  GO.lcd.setCursor(0, 220);
  GO.lcd.setTextSize(1);
  GO.lcd.setTextColor(BLACK);
  GO.lcd.print(" *MENU");
}

void qrShowCodeOnchain(bool anAddress, String message)
{
  // tft.fillScreen(TFT_WHITE);
  if (anAddress)
  {
    qrData.toUpperCase();
  }
  const char *qrDataChar = qrData.c_str();
  QRCode qrcoded;
  uint8_t qrcodeData[qrcode_getBufferSize(20)];
  int pixSize = 0;
  GO.lcd.setCursor(0, 100);
  GO.lcd.setTextSize(1);
  GO.lcd.setTextColor(BLACK);
  if (anAddress)
  {
    qrcode_initText(&qrcoded, qrcodeData, 2, 0, qrDataChar);
    pixSize = 4;
  }
  else
  {
    qrcode_initText(&qrcoded, qrcodeData, 4, 0, qrDataChar);
    pixSize = 3;
  }
  for (uint8_t y = 0; y < qrcoded.size; y++)
  {
    // Each horizontal module
    for (uint8_t x = 0; x < qrcoded.size; x++)
    {
      if (qrcode_getModule(&qrcoded, x, y))
      {
        GO.lcd.fillRect(25 + pixSize * x, 2 + pixSize * y, pixSize, pixSize, BLACK);
      }
      else
      {
        GO.lcd.fillRect(25 + pixSize * x, 2 + pixSize * y, pixSize, pixSize, WHITE);
      }
    }
  }
  GO.lcd.setCursor(0, 120);
  GO.lcd.println(message);
}

void qrShowCodeLNURL(String message)
{
  // tft.fillScreen(TFT_WHITE);
  qrData.toUpperCase();
  const char *qrDataChar = qrData.c_str();
  QRCode qrcoded;
  uint8_t qrcodeData[qrcode_getBufferSize(20)];
  qrcode_initText(&qrcoded, qrcodeData, 6, 0, qrDataChar);
  for (uint8_t y = 0; y < qrcoded.size; y++)
  {
    // Each horizontal module
    for (uint8_t x = 0; x < qrcoded.size; x++)
    {
      if (qrcode_getModule(&qrcoded, x, y))
      {
        GO.lcd.fillRect(20 + 3 * x, 2 + 3 * y, 3, 3, BLACK);
      }
      else
      {
        GO.lcd.fillRect(20 + 3 * x, 2 + 3 * y, 3, 3, WHITE);
      }
    }
  }
  GO.lcd.setCursor(0, 220);
  GO.lcd.setTextSize(1);
  GO.lcd.setTextColor(BLACK);
  GO.lcd.println(message);
}

void error(String message, String additional)
{
  GO.lcd.clearDisplay();
  GO.lcd.setTextColor(RED);
  GO.lcd.setTextSize(2);
  GO.lcd.setCursor(0, 30);
  GO.lcd.println(message);
  if (additional != "")
  {
    GO.lcd.setTextColor(WHITE);
    GO.lcd.setCursor(0, 120);
    GO.lcd.setTextSize(1);
    GO.lcd.println(additional);
  }
}

void processing(String message)
{
  GO.lcd.clearDisplay();
  // tft.fillScreen(TFT_BLACK);
  GO.lcd.setTextColor(WHITE);
  GO.lcd.setTextSize(1);
  GO.lcd.setCursor(20, 50);
  GO.lcd.println(message);
}

void complete()
{
  GO.lcd.clearDisplay();
  // GO.lcd.fillScreen(BLACK);
  GO.lcd.setTextColor(GREEN);
  GO.lcd.setTextSize(2);
  GO.lcd.setCursor(45, 30);
  GO.lcd.println("COMPLETE");
}

void showPin()
{
  GO.lcd.clearDisplay();
  // tft.fillScreen(TFT_BLACK);
  GO.lcd.setTextColor(WHITE);
  GO.lcd.setTextSize(2);
  GO.lcd.setCursor(40, 5);
  GO.lcd.println("PROOF PIN");
  GO.lcd.setCursor(50, 60);
  GO.lcd.setTextColor(GREEN);
  GO.lcd.setTextSize(3);
  GO.lcd.println(randomPin);
}

void lnurlInputScreen()
{
  GO.lcd.clearDisplay();
  // GO.lcd.fillScreen(BLACK);
  GO.lcd.setTextColor(WHITE); // White characters on black background
  GO.lcd.setTextSize(2);
  GO.lcd.setCursor(0, 0);
  GO.lcd.println("AMOUNT THEN #");
  GO.lcd.setCursor(50, 110);
  GO.lcd.setTextSize(1);
  GO.lcd.println("TO RESET PRESS *");
  GO.lcd.setTextSize(2);
  GO.lcd.setCursor(0, 30);
  GO.lcd.print(String(currency) + ":");
}

void logo()
{
  GO.lcd.clearDisplay();
  GO.lcd.setCursor(0, 40);
  GO.lcd.setTextWrap(1);
  GO.lcd.setTextFont(8);
  // GO.lcd.setTextSize(2);
  GO.lcd.setTextColor(ORANGE); 
  GO.lcd.println("bitcoin");
  GO.lcd.setTextColor(PURPLE); 
  GO.lcd.println("PoS");
  GO.lcd.setTextSize(1);
  GO.lcd.setCursor(0, 60);
  GO.lcd.setTextColor(WHITE); 
  GO.lcd.println("Powered by LNbits");
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

void menuLoop()
{
  GO.lcd.clearDisplay();
  // tft.fillScreen(TFT_BLACK);
  GO.lcd.setTextColor(WHITE);
  GO.lcd.setTextSize(1);
  GO.lcd.setCursor(0, 0);
  GO.lcd.print("   PAYMENT METHODS");
  GO.lcd.setCursor(0, 120);
  GO.lcd.setTextSize(1);
  GO.lcd.println(" *NEXT #SELECT");
  selection = "";
  selected = true;
  while (selected)
  {
    if (menuItemCheck[0] == 0 && menuItemNo == 0)
    {
      menuItemNo = menuItemNo + 1;
    }
    GO.lcd.setCursor(0, 30);
    GO.lcd.setTextSize(2);
    int current = 0;
    for (int i = 0; i < sizeof(menuItems) / sizeof(menuItems[0]); i++)
    {
      if (menuItemCheck[i] == 1)
      {
        if (menuItems[i] == menuItems[menuItemNo])
        {
          GO.lcd.setTextColor(GREEN);
          GO.lcd.println(menuItems[i]);
          selection = menuItems[i];
        }
        else
        {
          GO.lcd.setTextColor(WHITE);
          GO.lcd.println(menuItems[i]);
        }
      }
    }
    bool btnloop = true;
    while (btnloop)
    {
      key_val = "";
      getKeypad(false, true, false, false);
      if (key_val == "*")
      {
        menuItemNo = menuItemNo + 1;
        if (menuItemCheck[menuItemNo] == 0)
        {
          menuItemNo = menuItemNo + 1;
        }
        if (menuItemNo >= (sizeof(menuItems) / sizeof(menuItems[0])))
        {
          menuItemNo = 0;
        }
        btnloop = false;
      }
      if (key_val == "#")
      {
        selected = false;
        btnloop = false;
      }
    }
  }
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
  // client.setInsecure(); //Some versions of WiFiClientSecure need this
  const char *lnbitsServerChar = lnbitsServer.c_str();
  const char *invoiceChar = invoice.c_str();
  const char *lncurrencyChar = lncurrency.c_str();

  if (!client.connect(lnbitsServerChar, 443))
  {
    Serial.println("failed");
    error("SERVER DOWN", "");
    delay(3000);
  }

  String toPost = "{\"amount\" : 1, \"from\" :\"" + String(lncurrencyChar) + "\"}";
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
  String line = client.readString();
  StaticJsonDocument<150> doc;
  DeserializationError error = deserializeJson(doc, line);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  converted = doc["sats"];
}

void getInvoice()
{
  WiFiClientSecure client;
  lnbitsServer.toLowerCase();
  if (lnbitsServer.substring(0, 8) == "https://")
  {
    lnbitsServer = lnbitsServer.substring(8, lnbitsServer.length());
  }
  // client.setInsecure(); //Some versions of WiFiClientSecure need this
  const char *lnbitsServerChar = lnbitsServer.c_str();
  const char *invoiceChar = invoice.c_str();

  if (!client.connect(lnbitsServerChar, 443))
  {
    Serial.println("failed");
    error("SERVER DOWN", "");
    delay(3000);
    return;
  }

  String toPost = "{\"out\": false,\"amount\" : " + String(noSats.toInt()) + ", \"memo\" :\"LNPoS-" + String(random(1, 1000)) + "\"}";
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
  // client.setInsecure(); //Some versions of WiFiClientSecure need this
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

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
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
  if (selection == "LNURLPoS")
  {
    size_t payload_len = xor_encrypt(payload, sizeof(payload), (uint8_t *)secretPoS.c_str(), secretPoS.length(), nonce, sizeof(nonce), randomPin, dataIn.toInt());
    preparedURL = baseURLPoS + "?p=";
    preparedURL += toBase64(payload, payload_len, BASE64_URLSAFE | BASE64_NOPADDING);
  }
  else
  {
    size_t payload_len = xor_encrypt(payload, sizeof(payload), (uint8_t *)secretATM.c_str(), secretATM.length(), nonce, sizeof(nonce), randomPin, dataIn.toInt());
    preparedURL = baseURLATM + "?atm=1&p=";
    preparedURL += toBase64(payload, payload_len, BASE64_URLSAFE | BASE64_NOPADDING);
  }

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
