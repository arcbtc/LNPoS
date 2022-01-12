#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
using WebServerClass = WebServer;
fs::SPIFFSFS& FlashFS = SPIFFS;
#define FORMAT_ON_FAIL  true

#include <JC_Button.h>
#include <AutoConnect.h>
#include <SPI.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <Hash.h>
//#include <Conversion.h>
#include <ArduinoJson.h>
#include "qrcode.h"
#include "Bitcoin.h"
#include "esp_adc_cal.h"

#define KEYBOARD_I2C_ADDR     0X08
#define KEYBOARD_INT          5

#define PARAM_FILE "/elements.json"

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
uint8_t key_val;
bool onchainCheck = false;
bool lnCheck = false;
bool lnurlCheck = false;
bool unConfirmed = true;
int randomPin;
int calNum = 1;
int sumFlag = 0;
int converted = 0;

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

const byte
    BUTTON_PIN_A(39), BUTTON_PIN_B(38), BUTTON_PIN_C(37);
Button BTNA(BUTTON_PIN_A);
Button BTNB(BUTTON_PIN_B);
Button BTNC(BUTTON_PIN_C);

WebServerClass  server;
AutoConnect portal(server);
AutoConnectConfig config;
AutoConnectAux  elementsAux;
AutoConnectAux  saveAux;

void setup() {

  Serial.begin(115200);
  
  tft.init();
  tft.invertDisplay(false);
  tft.setRotation(1);
  logo();
  delay(1500);

  Wire.begin();
  pinMode(KEYBOARD_INT, INPUT_PULLUP);
  
  h.begin();
  BTNA.begin();
  BTNB.begin();
  BTNC.begin();
  FlashFS.begin(FORMAT_ON_FAIL);

//Get the saved details 
  File paramFile = FlashFS.open(PARAM_FILE, "r");
  if (paramFile) {
    StaticJsonDocument<2000> doc;
    DeserializationError error = deserializeJson(doc, paramFile.readString());
  
    JsonObject passRoot = doc[0];
    const char* apPasswordChar = passRoot["value"];
    const char* apNameChar = passRoot["name"];
    if (String(apPasswordChar) != "" && String(apNameChar) == "password") {
      apPassword = apPasswordChar;
    }
    JsonObject maRoot = doc[1];
    const char* masterKeyChar = maRoot["name"];
    masterKey = masterKeyChar;
    if(masterKey != ""){
      onchainCheck = true;
    }
    JsonObject serverRoot = doc[2];
    const char* serverChar = serverRoot["value"]; 
    lnbitsServer  = serverChar;
    JsonObject invoiceRoot = doc[3];
    const char* invoiceChar = invoiceRoot["value"]; 
    invoice  = invoiceChar;
    if(invoice != ""){
      lnCheck = true;
    }
    JsonObject lncurrencyRoot = doc[4];
    const char* lncurrencyChar = lncurrencyRoot["value"]; 
    lncurrency  = lncurrencyChar;
    JsonObject baseURLRoot = doc[5];
    const char* baseURLChar = baseURLRoot["value"]; 
    lnbitsBaseURL  = baseURLChar;
    JsonObject secretRoot = doc[6];
    const char* secretChar = secretRoot["value"]; 
    secret  = secretChar;
    JsonObject currencyRoot = doc[7];
    const char* currencyChar = currencyRoot["value"]; 
    currency  = currencyChar;
    if(secret != ""){
      lnurlCheck = true;
    }
  }
  paramFile.close();
  
//Handle AP traffic
  server.on("/", []() {
    String content = "<h1>bitcoinPoS</br>Free open-source bitcoin PoS</h1>";
    content += AUTOCONNECT_LINK(COG_24);
    server.send(200, "text/html", content);
  });
  
  elementsAux.load(FPSTR(PAGE_ELEMENTS));
  elementsAux.on([] (AutoConnectAux& aux, PageArgument& arg) {
    File param = FlashFS.open(PARAM_FILE, "r");
      if (param) {
        aux.loadElement(param, { "password", "masterkey", "server", "invoice", "lncurrency", "baseurl", "secret", "currency"} );
        param.close();
      }
    if (portal.where() == "/posconfig") {
      File param = FlashFS.open(PARAM_FILE, "r");
      if (param) {
        aux.loadElement(param, { "password", "masterkey", "server", "invoice", "lncurrency", "baseurl", "secret", "currency"} );
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
      elementsAux.saveElement(param, { "password", "masterkey", "server", "invoice", "lncurrency", "baseurl", "secret", "currency"});
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

  config.auth = AC_AUTH_BASIC;
  config.authScope = AC_AUTHSCOPE_AUX;
  config.ticker = true;
  config.autoReconnect = true;
  config.apid = "bitcoinPoS-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  config.psk = apPassword;
  config.menuItems = AC_MENUITEM_CONFIGNEW | AC_MENUITEM_OPENSSIDS | AC_MENUITEM_RESET;
  config.reconnectInterval = 1;
  
  portal.join({ elementsAux, saveAux });
  portal.config(config);
  portal.begin();
  
  int timer = 0;
  while(timer < 2000){
    BTNA.read();   
    BTNB.read();
    BTNC.read();
    if (BTNA.wasReleased() || BTNB.wasReleased() || BTNC.wasReleased() || (!onchainCheck && !lnCheck && !lnurlCheck)){
      portalLaunch();
      while(true){
        portal.handleClient();
      }
    }
  timer = timer + 200;
  delay(200);
  }
}

void loop() {
  noSats = "0";
  dataIn = "0";
  amountToShow = "0";
  unConfirmed = true;
  String choices[3][2] = {{"onchain", masterKey},{"ln", lnbitsServer},{"lnurl", lnbitsBaseURL}};
  int menuItem;
  int menuItems = 0;
  for(int i = 0;i < 3;i++){
    if(choices[i][1] != ""){
      menuItem = i;
      menuItems++;
    }
  }

//If only one thing configured skip the menu
  if(menuItems == 1){
    if(choices[menuItem][0] == "onchain"){
      onchainMain();
    }
    if(choices[menuItem][0] == "ln"){
      lnMain();
    }
    if(choices[menuItem][0] == "lnurl"){
      lnurlMain();
    }
  }
  else{
    choiceMenu("SELECT A PAYMENT METHOD");
    while(unConfirmed){
      BTNA.read();   
      BTNB.read();
      BTNC.read();
      if (BTNA.wasReleased() && onchainCheck){
        onchainMain();
      }
      if (BTNB.wasReleased() && lnCheck && WiFi.status() == WL_CONNECTED){
        lnMain();
      }
      if (BTNC.wasReleased() && lnurlCheck){
        lnurlMain();
      }
      delay(100);
    }
  }
  delay(3000);
}
void onchainMain(){
  Serial.println("onchain");
}
void lnMain(){
  if(converted == 0){
    choiceMenu("   FETCHING FIAT RATE");
    getSats();
  }
  inputScreen();
  while(unConfirmed){
    BTNA.read();   
    BTNB.read();
    BTNC.read();  
    if (BTNA.wasReleased() && onchainCheck){
      unConfirmed = false;
    }
    if (BTNB.wasReleased() && lnCheck){
      inputScreen();
      isLNMoneyNumber(true);
    }
    if(BTNC.wasReleased()){
      processing("FETCHING INVOICE");
      getInvoice();
      delay(1000);
      qrShowCodeln();
      delay(5000);
      while(unConfirmed){
        int timer = 0;
        unConfirmed = checkInvoice();
        if(!unConfirmed){
          complete();
          timer = 5000;
          delay(3000);
        }
        while(timer < 5000){
          BTNB.read();
          if (BTNB.wasReleased() && lnCheck){
            noSats = "0";
            dataIn = "0";
            amountToShow = "0";
            unConfirmed = false;
            timer = 5000;
          }
          delay(100);
          timer = timer + 100;
        }
      }
      noSats = "0";
      dataIn = "0";
      amountToShow = "0";
    }
    getKeypad(false, true);
    delay(100);
  }
  
}
void lnurlMain(){
  inputScreen();
  inputs = "";
  while (unConfirmed){
    getKeypad(false, false);
    if (BTNA.wasReleased()){
      unConfirmed = false;
    }
    else if (BTNC.wasReleased()) {
      makeLNURL();
      qrShowCode();
      while (unConfirmed){
        if (BTNA.wasReleased()) {
          showPin();
          if (BTNB.wasReleased()) {
            unConfirmed = false;
          }
        }
        if (BTNB.wasReleased()) {
          unConfirmed = false;
        }
      }
    }
    else if (BTNB.wasReleased()){
      inputScreen();
      isLNURLMoneyNumber(true);
    }
    delay(100);
  }
}
void getKeypad(bool isPin, bool isLN)
{
  if(digitalRead(KEYBOARD_INT) == LOW) 
   {
    Wire.requestFrom(KEYBOARD_I2C_ADDR, 1);
    while (Wire.available()) 
     { 
      uint8_t key_val = Wire.read();
      if(key_val != 0) 
       {    
        if(isDigit(key_val) || key_val == '.')
         {
          dataIn += (char)key_val;
          if(isLN){
            isLNMoneyNumber(false); 
          }
          else{
            isLNURLMoneyNumber(); 
          }
         }
      }
    }
  }
}

void portalLaunch()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_PURPLE, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(25, 100);
  tft.println("PORTAL LAUNCHED");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(0, 220);
  tft.setTextSize(2);
  tft.println("RESET DEVICE WHEN FINISHED");
}

void isLNMoneyNumber( bool cleared)
 { 
   if(!cleared){
     amountToShow = String(dataIn.toFloat() / 100);
     noSats = String((converted / 100) * dataIn.toFloat());
   }
   else{
    noSats = "0";
    dataIn = "0";
    amountToShow = "0";
   }
   tft.setTextSize(3);
   tft.setTextColor(TFT_RED, TFT_BLACK);
   tft.setCursor(88, 40);
   tft.println(amountToShow);
   tft.setTextColor(TFT_GREEN, TFT_BLACK);
   tft.setCursor(105, 88);
   tft.println(noSats.toInt());
 }

void isLNURLMoneyNumber( bool cleared )
 {
   if(!cleared){
     amountToShow = String(dataIn.toFloat() / 100);
   }
   else{
    dataIn = "0";
    amountToShow = "0";
   }
   tft.setTextSize(3);
   tft.setCursor(100, 120);
   tft.setTextColor(TFT_GREEN, TFT_BLACK);
   tft.setCursor(70, 88);
   tft.println(amountToShow);
 }
 
///////////DISPLAY///////////////
/////Lightning//////

void inputScreen()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(0, 40);
  tft.println(" " + String(lncurrency) + ": ");
  tft.println("");
  tft.println(" SATS: ");
  tft.println("");
  tft.println("");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawLine(0, 135, 400, 135, TFT_WHITE);
  tft.setCursor(0, 150);
  tft.println(" A. Back to menu");
  tft.println(" B. Clear");    
  tft.println(" C. Generate invoice");
  tft.setCursor(0, 220);
  tft.println("     A       B       C");
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
        tft.fillRect(65 + 3 * x, 20 + 3 * y, 3, 3, TFT_BLACK);
      }
      else
      {
        tft.fillRect(65 + 3 * x, 20 + 3 * y, 3, 3, TFT_WHITE);
      }
    }
  }
}

void qrShowCode()
{
  tft.fillScreen(TFT_WHITE);
  qrData.toUpperCase();
  const char *qrDataChar = qrData.c_str();
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(20)];
  qrcode_initText(&qrcode, qrcodeData, 8, 0, qrDataChar);
  for (uint8_t y = 0; y < qrcode.size; y++)
  {
    // Each horizontal module
    for (uint8_t x = 0; x < qrcode.size; x++)
    {
      if (qrcode_getModule(&qrcode, x, y))
      {
        tft.fillRect(65 + 3 * x, 20 + 3 * y, 3, 3, TFT_BLACK);
      }
      else
      {
        tft.fillRect(65 + 3 * x, 20 + 3 * y, 3, 3, TFT_WHITE);
      }
    }
  }
}

void error(String message, String additional)
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(75, 100);
  tft.println(message);
  if(additional != ""){
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(0, 220);
    tft.setTextSize(2);
    tft.println(additional);
  }
}

void processing(String message)
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(20, 100);
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
void choiceMenu(String message)
{
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 60);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.println(message);
  tft.setTextSize(2);
  tft.drawLine(0, 135, 400, 135, TFT_WHITE);
  if(!onchainCheck){
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(0, 150);
    tft.println(" A. Onchain");
  }
  else{
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(0, 150);
    tft.println(" A. Onchain");
  }
  if(!lnCheck){
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println(" B. Lightning");  
  }
  else if (lnCheck && WiFi.status() != WL_CONNECTED){
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println(" B. Lightning (needs WiFi)"); 
  }
  else{
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println(" B. Lightning");
  }
  if(!lnurlCheck){
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println(" C. Lightning Offline");
  }
  else{
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println(" C. Lightning Offline");
  }
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(0, 220);
  tft.setTextSize(2);
  tft.println("     A       B       C");
}
void showPin()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(0, 25);
  tft.println("PROOF PIN");
  tft.setCursor(100, 120);
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
  tft.setTextSize(5);
  tft.setCursor(10, 100);
  tft.print("bitcoin");
  tft.setTextColor(TFT_PURPLE, TFT_BLACK);
  tft.print("PoS");
  tft.setTextSize(2);
  tft.setCursor(12, 140);
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

void callback(){
}
//////////LIGHTNING//////////////////////
void getSats(){
  WiFiClientSecure client;
  //client.setInsecure(); //Some versions of WiFiClientSecure need this
  const char* lnbitsServerChar = lnbitsServer.c_str();
  const char* invoiceChar = invoice.c_str();
  const char* lncurrencyChar = lncurrency.c_str();

  if (!client.connect(lnbitsServerChar, 443)){
    Serial.println("failed");
    error("SERVER DOWN", "");
    delay(3000);
    return;   
  }

  String toPost = "{\"amount\" : 1, \"unit\" :\"" + String(lncurrencyChar) + "\"}";
  String url = "/api/v1/conversion";
  client.print(String("POST ") + url +" HTTP/1.1\r\n" +
                "Host: " + String(lnbitsServerChar) + "\r\n" +
                "User-Agent: ESP32\r\n" +
                "X-Api-Key: "+ String(invoiceChar) +" \r\n" +
                "Content-Type: application/json\r\n" +
                "Connection: close\r\n" +
                "Content-Length: " + toPost.length() + "\r\n" +
                "\r\n" + 
                toPost + "\n");

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
    if (line == "\r") {
      break;
    }
  }
  String convertedStr = client.readString();
  converted = convertedStr.toInt();
}

void getInvoice() 
{
  WiFiClientSecure client;
  //client.setInsecure(); //Some versions of WiFiClientSecure need this
  const char* lnbitsServerChar = lnbitsServer.c_str();
  const char* invoiceChar = invoice.c_str();

  if (!client.connect(lnbitsServerChar, 443)){
    Serial.println("failed");
    error("SERVER DOWN", "");
    delay(3000);
    return;   
  }

  String toPost = "{\"out\": false,\"amount\" : " + String(noSats.toInt()) + ", \"memo\" :\"bitcoinPoS-" + String(random(1,1000)) + "\"}";
  String url = "/api/v1/payments";
  client.print(String("POST ") + url +" HTTP/1.1\r\n" +
                "Host: " + lnbitsServerChar + "\r\n" +
                "User-Agent: ESP32\r\n" +
                "X-Api-Key: "+ invoiceChar +" \r\n" +
                "Content-Type: application/json\r\n" +
                "Connection: close\r\n" +
                "Content-Length: " + toPost.length() + "\r\n" +
                "\r\n" + 
                toPost + "\n");

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
    if (line == "\r") {
      break;
    }
    if (line == "\r") {
      break;
    }
  }
  String line = client.readString();

  StaticJsonDocument<1000> doc;
  DeserializationError error = deserializeJson(doc, line);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  const char* payment_hash = doc["checking_id"];
  const char* payment_request = doc["payment_request"];
  qrData = payment_request;
  dataId = payment_hash;
  Serial.println(qrData);
}


bool checkInvoice()
{
  WiFiClientSecure client;
  //client.setInsecure(); //Some versions of WiFiClientSecure need this
  const char* lnbitsServerChar = lnbitsServer.c_str();
  const char* invoiceChar = invoice.c_str();
  if (!client.connect(lnbitsServerChar, 443)){
    error("SERVER DOWN", "");
    delay(3000);
    return false;   
  }

  String url = "/api/v1/payments/";
  client.print(String("GET ") + url + dataId + " HTTP/1.1\r\n" +
                "Host: " + lnbitsServerChar + "\r\n" +
                "User-Agent: ESP32\r\n" +
                "X-Api-Key:"+ invoiceChar +"\r\n" +
                "Content-Type: application/json\r\n" +
                "Connection: close\r\n\r\n");
   while (client.connected()) {
   String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
    if (line == "\r") {
      break;
    }
  }
  String line = client.readString();
  Serial.println(line);
  StaticJsonDocument<500> doc;
  DeserializationError error = deserializeJson(doc, line);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return false;
  }
  bool boolPaid = doc["paid"];
  if(boolPaid){
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
  size_t payload_len = xor_encrypt(payload, sizeof(payload), (uint8_t *)key.c_str(), key.length(), nonce, sizeof(nonce), randomPin, dataIn.toInt());
  preparedURL = baseURL + "?p=";
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

int xor_encrypt(uint8_t * output, size_t outlen, uint8_t * key, size_t keylen, uint8_t * nonce, size_t nonce_len, uint64_t pin, uint64_t amount_in_cents){
  // check we have space for all the data:
  // <variant_byte><len|nonce><len|payload:{pin}{amount}><hmac>
  if(outlen < 2 + nonce_len + 1 + lenVarInt(pin) + 1 + lenVarInt(amount_in_cents) + 8){
    return 0;
  }
  int cur = 0;
  output[cur] = 1; // variant: XOR encryption
  cur++;
  // nonce_len | nonce
  output[cur] = nonce_len;
  cur++;
  memcpy(output+cur, nonce, nonce_len);
  cur += nonce_len;
  // payload, unxored first - <pin><currency byte><amount>
  int payload_len = lenVarInt(pin) + 1 + lenVarInt(amount_in_cents);
  output[cur] = (uint8_t)payload_len;
  cur++;
  uint8_t * payload = output+cur; // pointer to the start of the payload
  cur += writeVarInt(pin, output+cur, outlen-cur); // pin code
  cur += writeVarInt(amount_in_cents, output+cur, outlen-cur); // amount
  cur++;
  // xor it with round key
  uint8_t hmacresult[32];
  SHA256 h;
  h.beginHMAC(key, keylen);
  h.write((uint8_t *)"Round secret:", 13);
  h.write(nonce, nonce_len);
  h.endHMAC(hmacresult);
  for(int i=0; i < payload_len; i++){
    payload[i] = payload[i] ^ hmacresult[i];
  }
  // add hmac to authenticate
  h.beginHMAC(key, keylen);
  h.write((uint8_t *)"Data:", 5);
  h.write(output, cur);
  h.endHMAC(hmacresult);
  memcpy(output+cur, hmacresult, 8);
  cur += 8;
  // return number of bytes written to the output
  return cur;
}
