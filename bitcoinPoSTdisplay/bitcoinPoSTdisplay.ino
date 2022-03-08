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
//#include <Wire.h>
#include <TFT_eSPI.h>
#include <Hash.h>
#include <ArduinoJson.h>
#include "qrcoded.h"
#include "Bitcoin.h"
//#include "esp_adc_cal.h"

#define PARAM_FILE "/elements.json"
#define KEY_FILE "/thekey.txt"

// variables
String inputs;
String thePin;
String spiffing;
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
const char menuItems[4][12] = {"LNPoS", "LNURLPoS", "OnChain", "LNURLATM"};
int menuItemCheck[4] = {0, 0, 0, 0};
String selection;
int menuItemNo = 0;
int randomPin;
int calNum = 1;
int sumFlag = 0;
int converted = 0;
int qrScreenBrightness = 180; // 0 = min, 255 = max
bool isSleepEnabled = true;
int sleepTimer = 5; // Time in seconds before the device goes to sleep
bool isPretendSleeping = false;
long timeOfLastInteraction = millis();
String key_val;
bool onchainCheck = false;
bool lnCheck = false;
bool lnurlCheck = false;
bool unConfirmed = true;
bool selected = false;
bool lnurlCheckPoS = false;
bool lnurlCheckATM = false;
String lnurlATMPin;
enum invoiceType {
  LNPOS,
  LNURLPOS,
  ONCHAIN,
  LNURLATM
};

// custom access point pages
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
TFT_eSPI tft = TFT_eSPI();

uint16_t qrScreenBgColour = tft.color565(qrScreenBrightness, qrScreenBrightness, qrScreenBrightness);

const byte rows = 4; //four rows
const byte cols = 3; //three columns
char keys[rows][cols] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

byte rowPins[rows] = {21, 27, 26, 22}; //connect to the row pinouts of the keypad
byte colPins[cols] = {33, 32, 25};     //connect to the column pinouts of the keypad

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

  // load screen
  tft.init();
  tft.setRotation(1);
  tft.invertDisplay(true);

  logo();

  // load buttons
  h.begin();
  FlashFS.begin(FORMAT_ON_FAIL);
  SPIFFS.begin(true);

  // get the saved details and store in global variables
  File paramFile = FlashFS.open(PARAM_FILE, "r");
  if (paramFile)
  {
    StaticJsonDocument<2500> doc;
    DeserializationError error = deserializeJson(doc, paramFile.readString());

    const JsonObject passRoot = doc[0];
    const char *apPasswordChar = passRoot["value"];
    const char *apNameChar = passRoot["name"];
    if (String(apPasswordChar) != "" && String(apNameChar) == "password")
    {
      apPassword = apPasswordChar;
    }

    const JsonObject maRoot = doc[1];
    const char *masterKeyChar = maRoot["value"];
    masterKey = masterKeyChar;
    if (masterKey != "")
    {
      menuItemCheck[2] = 1;
    }

    const JsonObject serverRoot = doc[2];
    const char *serverChar = serverRoot["value"];
    lnbitsServer = serverChar;

    const JsonObject invoiceRoot = doc[3];
    const char *invoiceChar = invoiceRoot["value"];
    invoice = invoiceChar;
    if (invoice != "")
    {
      menuItemCheck[0] = 1;
    }

    const JsonObject lncurrencyRoot = doc[4];
    const char *lncurrencyChar = lncurrencyRoot["value"];
    lncurrency = lncurrencyChar;

    const JsonObject lnurlPoSRoot = doc[5];
    const char *lnurlPoSChar = lnurlPoSRoot["value"];
    const String lnurlPoS = lnurlPoSChar;
    baseURLPoS = getValue(lnurlPoS, ',', 0);
    secretPoS = getValue(lnurlPoS, ',', 1);
    currencyPoS = getValue(lnurlPoS, ',', 2);
    if (secretPoS != "")
    {
      menuItemCheck[1] = 1;
    }

    const JsonObject lnurlATMRoot = doc[6];
    const char *lnurlATMChar = lnurlATMRoot["value"];
    const String lnurlATM = lnurlATMChar;
    baseURLATM = getValue(lnurlATM, ',', 0);
    secretATM = getValue(lnurlATM, ',', 1);
    currencyATM = getValue(lnurlATM, ',', 2);
    if (secretATM != "")
    {
      menuItemCheck[3] = 1;
    }

    const JsonObject lnurlATMMSRoot = doc[7];
    const char *lnurlATMMSChar = lnurlATMMSRoot["value"];
    lnurlATMMS = lnurlATMMSChar;

    const JsonObject lnurlATMPinRoot = doc[8];
    const char *lnurlATMPinChar = lnurlATMPinRoot["value"];
    lnurlATMPin = lnurlATMPinChar;
  }

  paramFile.close();

  // general WiFi setting
  config.autoReset = false;
  config.autoReconnect = true;
  config.reconnectInterval = 1; // 30s
  config.beginTimeout = 10000UL;

  // start portal (any key pressed on startup)
  const char key = keypad.getKey();
  if (key != NO_KEY)
  {
    // handle access point traffic
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
        // save as a loadable set for parameters.
        elementsAux.saveElement(param, {"password", "masterkey", "server", "invoice", "lncurrency", "lnurlpos", "lnurlatm", "lnurlatmms", "lnurlatmpin"});
        param.close();

        // read the saved elements again to display.
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

    // start access point
    portalLaunch();

    config.immediateStart = true;
    config.ticker = true;
    config.apid = "bitcoinPoS-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    config.psk = apPassword;
    config.menuItems = AC_MENUITEM_CONFIGNEW | AC_MENUITEM_OPENSSIDS | AC_MENUITEM_RESET;
    config.title = "bitcoinPoS";

    portal.join({elementsAux, saveAux});
    portal.config(config);
    portal.begin();
    while (true)
    {
      portal.handleClient();
    }
  }

  // connect to configured WiFi
  if (menuItemCheck[0])
  {
    config.autoRise = false;

    portal.join({elementsAux, saveAux});
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

  // check wifi status
  if (menuItemCheck[0] == 1 && WiFi.status() != WL_CONNECTED)
  {
    menuItemCheck[0] = -1;
  }
  else if (menuItemCheck[0] == -1 && WiFi.status() == WL_CONNECTED)
  {
    menuItemCheck[0] = 1;
  }

  // count menu items
  int menuItemsAmount = 0;

  for (int i = 0; i < sizeof(menuItems) / sizeof(menuItems[0]); i++)
  {
    if (menuItemCheck[i] == 1)
    {
      menuItemsAmount++;
      selection = menuItems[i];
    }
  }

  // no methods available
  if (menuItemsAmount < 1)
  {
    error("  NO METHODS", "RESTART & RUN PORTAL");
    delay(10000000);
  }

  // select menu item
  while (unConfirmed)
  {
    if (menuItemsAmount > 1) {
      menuLoop();
    }

    if (selection == "LNPoS")
    {
      lnMain();
    }
    else if (selection == "OnChain")
    {
      onchainMain();
    }
    else if (selection == "LNURLPoS")
    {
      lnurlPoSMain();
    }
    else if (selection == "LNURLATM")
    {
      lnurlATMMain();
    }

    if (menuItemsAmount == 1) {
      unConfirmed = false;
    }
  }
}

// on-chain payment method
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
    else if (key_val == "#")
    {
      HDPublicKey hd(masterKey);
      qrData = hd.derive(String("m/0/") + addressNo).address();
      qrShowCodeOnchain(true, " *MENU #CHECK");

      while (unConfirmed)
      {
        key_val = "";
        getKeypad(false, true, false, false);

        if (key_val == "*")
        {
          unConfirmed = false;
        }
        else if (key_val == "#")
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
        handleBrightnessAdjust(key_val, ONCHAIN);
      }
    }
  }
}

void lnMain()
{
  if (converted == 0)
  {
    processing("FETCHING FIAT RATE");
    if (!getSats()) {
      error("FETCHING FIAT RATE FAILED");
      delay(3000);
      return;
    }
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
    else if (key_val == "#")
    {
      // request invoice
      processing("FETCHING INVOICE");
      if (!getInvoice()) {
        unConfirmed = false;
        error("ERROR FETCHING INVOICE");
        delay(3000);
        break;
      }

      // show QR
      qrShowCodeln();

      // check invoice
      bool isFirstRun = true;
      while (unConfirmed)
      {
        int timer = 0;

        if (!isFirstRun) {
          unConfirmed = checkInvoice();
          if (!unConfirmed)
          {
            paymentSuccess();
            timer = 5000;

            while (key_val != "*") {
              key_val = "";
              getKeypad(false, true, false, false);

              if (key_val != "*") {
                delay(100);
              }
            }
          }
        }

        // abort on * press
        while (timer < (isFirstRun ? 6000 : 2000))
        {
          getKeypad(false, true, false, false);

          if (key_val == "*")
          {
            noSats = "0";
            dataIn = "0";
            amountToShow = "0";
            unConfirmed = false;
            timer = 5000;
            break;
            
          } else {
            delay(100);
          }
          handleBrightnessAdjust(key_val, LNPOS);
          timer = timer + 100;
        }

        isFirstRun = false;
      }

      noSats = "0";
      dataIn = "0";
      amountToShow = "0";
    }
    else
    {
      delay(100);
    }
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
        else if (key_val == "*")
        {
          unConfirmed = false;
        }
        handleBrightnessAdjust(key_val, LNURLPOS);
      }
    }
    else
    {
      delay(100);
    }
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
    else if (key_val == "#")
    {
      isATMMoneyPin(true);
    }

    if (pinToShow.length() == lnurlATMPin.length() && pinToShow != lnurlATMPin)
    {
      error("  WRONG PIN");
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
        else if (key_val == "#")
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
    else
    {
      delay(100);
    }
  }
}

void getKeypad(bool isATMPin, bool justKey, bool isLN, bool isATMNum)
{
  const char key = keypad.getKey();
  if (key == NO_KEY)
  {
    return;
  }

  key_val = String(key);

  if (dataIn.length() < 9) {
    dataIn += key_val;
  }

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

///////////DISPLAY///////////////
void portalLaunch()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_PURPLE, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(20, 50);
  tft.println("AP LAUNCHED");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(0, 75);
  tft.setTextSize(2);
  tft.println(" WHEN FINISHED RESET");
}

void isLNMoneyNumber(bool cleared)
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 20);
  tft.print("  - ENTER AMOUNT -");
  tft.setTextSize(3);
  tft.setCursor(0, 50);
  tft.println(String(lncurrency) + ": ");
  tft.println("SAT: ");
  tft.setCursor(0, 120);
  tft.setTextSize(2);
  tft.println(" *MENU #INVOICE");

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
  tft.setCursor(75, 50);
  tft.println(amountToShow);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(75, 75);
  tft.println(noSats.toInt());
}

void isLNURLMoneyNumber(bool cleared)
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 20);
  tft.print("  - ENTER AMOUNT -");
  tft.setTextSize(3);
  tft.setCursor(0, 50);
  tft.println(String(currencyPoS) + ": ");
  tft.setCursor(0, 120);
  tft.setTextSize(2);
  tft.println(" *MENU #INVOICE");
  tft.setTextSize(3);

  if (!cleared)
  {
    amountToShow = String(dataIn.toFloat() / 100);
  }
  else
  {
    dataIn = "0";
    amountToShow = "0.00";
  }

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(75, 50);
  tft.println(amountToShow);
}

void isATMMoneyNumber(bool cleared)
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 20);
  tft.print("  - ENTER AMOUNT -");
  tft.setTextSize(3);
  tft.setCursor(0, 50);
  tft.println(String(currencyATM) + ": ");
  tft.setCursor(0, 120);
  tft.setTextSize(2);
  tft.println(" *MENU #WITHDRAW");
  tft.setTextSize(3);

  if (!cleared)
  {
    amountToShow = String(dataIn.toFloat() / 100);
  }
  else
  {
    dataIn = "0";
    amountToShow = "0.00";
  }

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(75, 50);
  tft.println(amountToShow);
}

void isATMMoneyPin(bool cleared)
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 20);
  tft.print(" ENTER SECRET PIN");
  tft.setTextSize(3);
  tft.setCursor(0, 50);
  tft.println("PIN:");
  tft.setCursor(0, 120);
  tft.setTextSize(2);
  tft.println(" *MENU #CLEAR");

  pinToShow = dataIn;
  tft.setTextSize(3);
  if (cleared)
  {
    pinToShow = "";
    dataIn = "";
  }

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(75, 50);
  tft.println(pinToShow);
}

void inputScreenOnChain()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(0, 40);
  tft.println("XPUB ENDING " + masterKey.substring(masterKey.length() - 5));
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 120);
  tft.println(" *MENU #ADDRESS");
}

void qrShowCodeln()
{
  tft.fillScreen(qrScreenBgColour);
  qrData.toUpperCase();
  const char *qrDataChar = qrData.c_str();
  QRCode qrcoded;
  uint8_t qrcodeData[qrcode_getBufferSize(20)];

  qrcode_initText(&qrcoded, qrcodeData, 11, 0, qrDataChar);

  for (uint8_t y = 0; y < qrcoded.size; y++)
  {
    for (uint8_t x = 0; x < qrcoded.size; x++)
    {
      if (qrcode_getModule(&qrcoded, x, y))
      {
        tft.fillRect(65 + 2 * x, 5 + 2 * y, 2, 2, TFT_BLACK);
      }
      else
      {
        tft.fillRect(65 + 2 * x, 5 + 2 * y, 2, 2, qrScreenBgColour);
      }
    }
  }

  tft.setCursor(0, 220);
  tft.setTextSize(2);
  tft.setTextColor(TFT_BLACK, qrScreenBgColour);
  tft.print(" *MENU");
}

void qrShowCodeOnchain(bool anAddress, String message)
{
  tft.fillScreen(qrScreenBgColour);
  if (anAddress)
  {
    qrData.toUpperCase();
  }

  const char *qrDataChar = qrData.c_str();
  QRCode qrcoded;
  uint8_t qrcodeData[qrcode_getBufferSize(20)];
  int pixSize = 0;

  tft.setCursor(0, 100);
  tft.setTextSize(2);
  tft.setTextColor(TFT_BLACK, qrScreenBgColour);
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
    for (uint8_t x = 0; x < qrcoded.size; x++)
    {
      if (qrcode_getModule(&qrcoded, x, y))
      {
        tft.fillRect(70 + pixSize * x, 5 + pixSize * y, pixSize, pixSize, TFT_BLACK);
      }
      else
      {
        tft.fillRect(70 + pixSize * x, 5 + pixSize * y, pixSize, pixSize, qrScreenBgColour);
      }
    }
  }

  tft.setCursor(0, 120);
  tft.println(message);
}

void qrShowCodeLNURL(String message)
{
  tft.fillScreen(qrScreenBgColour);
  qrData.toUpperCase();
  const char *qrDataChar = qrData.c_str();
  QRCode qrcoded;
  uint8_t qrcodeData[qrcode_getBufferSize(20)];
  qrcode_initText(&qrcoded, qrcodeData, 6, 0, qrDataChar);

  for (uint8_t y = 0; y < qrcoded.size; y++)
  {
    for (uint8_t x = 0; x < qrcoded.size; x++)
    {
      if (qrcode_getModule(&qrcoded, x, y))
      {
        tft.fillRect(65 + 3 * x, 5 + 3 * y, 3, 3, TFT_BLACK);
      }
      else
      {
        tft.fillRect(65 + 3 * x, 5 + 3 * y, 3, 3, qrScreenBgColour);
      }
    }
  }

  tft.setCursor(0, 220);
  tft.setTextSize(2);
  tft.setTextColor(TFT_BLACK, qrScreenBgColour);
  tft.println(message);
}

void error(String message)
{
  error(message, "");
}

void error(String message, String additional)
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(0, 30);
  tft.println(message);
  if (additional != "")
  {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(0, 120);
    tft.setTextSize(2);
    tft.println(additional);
  }
}

void processing(String message)
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 60);
  tft.println(message);
}

void paymentSuccess()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(70, 50);
  tft.println("PAYED");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.println("  PRESS * FOR MENU");
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
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(0, 0);
  tft.println("AMOUNT THEN #");
  tft.setCursor(50, 110);
  tft.setTextSize(2);
  tft.println("TO RESET PRESS *");
  tft.setTextSize(3);
  tft.setCursor(0, 30);
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

long int lastBatteryCheck = 0;
void updateBatteryStatus(bool force = false)
{
  // throttle
  if(!force && lastBatteryCheck != 0 && millis() - lastBatteryCheck < 5000) {
    return;
  }

  lastBatteryCheck = millis();

  // update
  const int batteryPercentage = getBatteryPercentage();

  String batteryPercentageText = "";
  if (batteryPercentage == NULL) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    batteryPercentageText = " USB";

  } else {
    if(batteryPercentage >= 60) {
      tft.setTextColor(TFT_GREEN, TFT_BLACK);

    } else if (batteryPercentage >= 20) {
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);

    } else {
      tft.setTextColor(TFT_RED, TFT_BLACK);
    }

    if(batteryPercentage != 100) {
      batteryPercentageText += " ";
      
      if (batteryPercentage < 10) {
        batteryPercentageText += " ";
      }
    }

    batteryPercentageText += String(batteryPercentage) + "%";
  }

  tft.setCursor(190, 120);
  tft.print(batteryPercentageText);
}

void menuLoop()
{
  timeOfLastInteraction = millis();
  Serial.println("menuLoop");

  // footer/header
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 10);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.print("      - MENU -");
  tft.setCursor(0, 120);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print(" *NEXT #SELECT");

  updateBatteryStatus(true);

  // menu items
  selection = "";
  selected = true;

  while (selected)
  {
    maybeSleepDevice();
    if (menuItemCheck[0] <= 0 && menuItemNo == 0)
    {
      menuItemNo++;
    }

    tft.setCursor(0, 40);
    tft.setTextSize(2);

    int current = 0;
    int menuItemCount = 0;

    for (int i = 0; i < sizeof(menuItems) / sizeof(menuItems[0]); i++)
    {
      if (menuItemCheck[i] == 1)
      {
        if (menuItems[i] == menuItems[menuItemNo])
        {
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          selection = menuItems[i];
        }
        else
        {
          tft.setTextColor(TFT_WHITE, TFT_BLACK);
        }

        tft.print("  ");
        tft.println(menuItems[i]);
        menuItemCount++;
      }
    }

    bool btnloop = true;
    while (btnloop)
    {
      maybeSleepDevice();
      key_val = "";
      getKeypad(false, true, false, false);

      if (key_val == "*")
      {
        isPretendSleeping = false;
        timeOfLastInteraction = millis();
        
        menuItemNo++;
        if (menuItemCheck[menuItemNo] < 1)
        {
          menuItemNo++;
        }

        if (menuItemNo > menuItemCount)
        {
          menuItemNo = 0;
          break;
        }

        btnloop = false;
      }
      else if (key_val == "#")
      {
        isPretendSleeping = false;
        timeOfLastInteraction = millis();
        
        selected = false;
        btnloop = false;
      }
      else
      {
        updateBatteryStatus();
        delay(100);
      }
    }
  }
}

//////////LIGHTNING//////////////////////
bool getSats()
{
  WiFiClientSecure client;
  client.setInsecure(); //Some versions of WiFiClientSecure need this

  lnbitsServer.toLowerCase();
  if (lnbitsServer.substring(0, 8) == "https://")
  {
    lnbitsServer = lnbitsServer.substring(8, lnbitsServer.length());
  }
  const char *lnbitsServerChar = lnbitsServer.c_str();
  const char *invoiceChar = invoice.c_str();
  const char *lncurrencyChar = lncurrency.c_str();

  Serial.println("connecting to LNbits server " + lnbitsServer);
  if (!client.connect(lnbitsServerChar, 443))
  {
    Serial.println("failed to connect to LNbits server " + lnbitsServer);
    return false;
  }

  const String toPost = "{\"amount\" : 1, \"from\" :\"" + String(lncurrencyChar) + "\"}";
  const String url = "/api/v1/conversion";
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
    const String line = client.readStringUntil('\n');
    if (line == "\r")
    {
      break;
    }
  }

  const String line = client.readString();
  StaticJsonDocument<150> doc;
  DeserializationError error = deserializeJson(doc, line);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return false;
  }

  converted = doc["sats"];
  return true;
}

bool getInvoice()
{
  WiFiClientSecure client;
  client.setInsecure(); //Some versions of WiFiClientSecure need this

  lnbitsServer.toLowerCase();
  if (lnbitsServer.substring(0, 8) == "https://")
  {
    lnbitsServer = lnbitsServer.substring(8, lnbitsServer.length());
  }
  const char *lnbitsServerChar = lnbitsServer.c_str();
  const char *invoiceChar = invoice.c_str();

  if (!client.connect(lnbitsServerChar, 443))
  {
    Serial.println("failed");
    error("SERVER DOWN");
    delay(3000);
    return false;
  }

  const String toPost = "{\"out\": false,\"amount\" : " + String(noSats.toInt()) + ", \"memo\" :\"bitcoinPoS-" + String(random(1, 1000)) + "\"}";
  const String url = "/api/v1/payments";
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
    const String line = client.readStringUntil('\n');

    if (line == "\r")
    {
      break;
    }
  }
  const String line = client.readString();

  StaticJsonDocument<1000> doc;
  DeserializationError error = deserializeJson(doc, line);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return false;
  }

  const char *payment_hash = doc["checking_id"];
  const char *payment_request = doc["payment_request"];
  qrData = payment_request;
  dataId = payment_hash;

  Serial.println(qrData);
  return true;
}

bool checkInvoice()
{
  WiFiClientSecure client;
  client.setInsecure(); //Some versions of WiFiClientSecure need this

  const char *lnbitsServerChar = lnbitsServer.c_str();
  const char *invoiceChar = invoice.c_str();
  if (!client.connect(lnbitsServerChar, 443))
  {
    error("SERVER DOWN");
    delay(3000);
    return false;
  }

  const String url = "/api/v1/payments/";
  client.print(String("GET ") + url + dataId + " HTTP/1.1\r\n" +
               "Host: " + lnbitsServerChar + "\r\n" +
               "User-Agent: ESP32\r\n" +
               "X-Api-Key:" + invoiceChar + "\r\n" +
               "Content-Type: application/json\r\n" +
               "Connection: close\r\n\r\n");
  while (client.connected())
  {
    const String line = client.readStringUntil('\n');
    if (line == "\r")
    {
      break;
    }
  }

  const String line = client.readString();
  Serial.println(line);
  StaticJsonDocument<500> doc;

  DeserializationError error = deserializeJson(doc, line);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return false;
  }
  if (doc["paid"])
  {
    unConfirmed = false;
  }

  return unConfirmed;
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  const int maxIndex = data.length() - 1;

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

//////////UTILS///////////////
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
  int res = convert_bits(data, &len, 5, (byte *) url, strlen(url), 8, 1);
  char *charLnurl = (char *) calloc(strlen(url) * 2, sizeof(byte));
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
  h.write((uint8_t *) "Round secret:", 13);
  h.write(nonce, nonce_len);
  h.endHMAC(hmacresult);
  for (int i = 0; i < payload_len; i++)
  {
    payload[i] = payload[i] ^ hmacresult[i];
  }

  // add hmac to authenticate
  h.beginHMAC(key, keylen);
  h.write((uint8_t *) "Data:", 5);
  h.write(output, cur);
  h.endHMAC(hmacresult);
  memcpy(output + cur, hmacresult, 8);
  cur += 8;

  // return number of bytes written to the output
  return cur;
}

unsigned int getBatteryPercentage()
{
  const float batteryMaxVoltage = 4.2;
  const float batteryMinVoltage = 3.73;

  const float batteryAllowedRange = batteryMaxVoltage - batteryMinVoltage;
  const float batteryCurVAboveMin = getInputVoltage() - batteryMinVoltage;

  const int batteryPercentage = (int) (batteryCurVAboveMin / batteryAllowedRange * 100);
  if (batteryPercentage > 150) {
    return NULL;
  }

  return max(min(batteryPercentage, 100), 0);
}

float getInputVoltage()
{
  delay(100);
  const uint16_t v1 = analogRead(34);
  return ((float) v1 / 4095.0f) * 2.0f * 3.3f * (1100.0f / 1000.0f);
}

void adjustQrBrightness(bool shouldMakeBrighter, invoiceType invoiceType)
{
  if (shouldMakeBrighter && qrScreenBrightness >= 0)
  {
    qrScreenBrightness = qrScreenBrightness + 25;
    if (qrScreenBrightness > 255)
    {
      qrScreenBrightness = 255;
    }
  }
  else if (!shouldMakeBrighter && qrScreenBrightness <= 30)
  {
    qrScreenBrightness = qrScreenBrightness - 5;
  }
  else if (!shouldMakeBrighter && qrScreenBrightness <= 255)
  {
    qrScreenBrightness = qrScreenBrightness - 25;
  }
  
  if (qrScreenBrightness < 4)
  {
    qrScreenBrightness = 4;
  }
  
  qrScreenBgColour = tft.color565(qrScreenBrightness, qrScreenBrightness, qrScreenBrightness);

  switch(invoiceType) {
    case LNPOS:
      qrShowCodeln();
      break;
    case LNURLPOS:
      qrShowCodeLNURL(" *MENU #SHOW PIN");
      break;
    case ONCHAIN:
      qrShowCodeOnchain(true, " *MENU #CHECK");
      break;  
    case LNURLATM:
      qrShowCodeLNURL(" *MENU");
      break;  
    default:
      break;
  }
  
  File configFile = SPIFFS.open("/config.txt", "w");
  configFile.print(String(qrScreenBrightness));
  configFile.close();
}

/**
 * Load stored config values
 */
void loadConfig() {
  File file = SPIFFS.open("/config.txt");
   spiffing = file.readStringUntil('\n');
  String tempQrScreenBrightness = spiffing.c_str();
  int tempQrScreenBrightnessInt = tempQrScreenBrightness.toInt();
  Serial.println("spiffcontent " + String(tempQrScreenBrightnessInt));
  file.close();

  if(tempQrScreenBrightnessInt && tempQrScreenBrightnessInt > 3) {
    qrScreenBrightness = tempQrScreenBrightnessInt;
  }
  Serial.println("qrScreenBrightness from config " + String(qrScreenBrightness));
  qrScreenBgColour = tft.color565(qrScreenBrightness, qrScreenBrightness, qrScreenBrightness);
}

/**
 * Handle user inputs for adjusting the screen brightness
 */
void handleBrightnessAdjust(String keyVal, invoiceType invoiceType) {
  // Handle screen brighten on QR screen
  if (keyVal == "1"){
      Serial.println("Adjust bnrightness " + invoiceType);
    timeOfLastInteraction = millis();
    adjustQrBrightness(true, invoiceType);
  }
  // Handle screen dim on QR screen
  else if (keyVal == "4"){
      Serial.println("Adjust bnrightness " + invoiceType);
    timeOfLastInteraction = millis();
    adjustQrBrightness(false, invoiceType);
  }
}

/**
 * Check whether the device should be put to sleep and put it to sleep
 * if it should
 */
void maybeSleepDevice() {
  if(isSleepEnabled && !isPretendSleeping) {
    long currentTime = millis();
    if(currentTime > (timeOfLastInteraction + sleepTimer * 1000)) {
      sleepAnimation();
      // The device wont charge if it is sleeping, so when charging, do a pretend sleep
      if(isPoweredExternally()) {
        Serial.println("Pretend sleep now");
        isPretendSleeping = true;
        tft.fillScreen(TFT_BLACK);
      }
      else {
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,1); //1 = High, 0 = Low
        Serial.println("Going to sleep now");
        esp_deep_sleep_start();
      }
    }
  }
}

/**
 * Does the device have external or internal power?
 */
bool isPoweredExternally() {
  float inputVoltage = getInputVoltage();
  if(inputVoltage > 4.5)
  {
    return true;
  }
  else
  {
    return false;
  }
  
}

/**
 * Awww. Show the go to sleep animation
 */
void sleepAnimation() {
    printSleepAnimationFrame("(o.o)", 500);
    printSleepAnimationFrame("(-.-)", 500);
    printSleepAnimationFrame("(-.-)z", 250);
    printSleepAnimationFrame("(-.-)zz", 250);
    printSleepAnimationFrame("(-.-)zzz", 250);
    tft.fillScreen(TFT_BLACK);
}

void wakeAnimation() {
    printSleepAnimationFrame("(-.-)", 100);
    printSleepAnimationFrame("(o.o)", 200);
    tft.fillScreen(TFT_BLACK);
}

/**
 * Print the line of the animation
 */
void printSleepAnimationFrame(String text, int wait) {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(5, 80);
  tft.setTextSize(4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); 
  //tft.setFreeFont(BIGFONT);
  tft.println(text);
  delay(wait);
}
