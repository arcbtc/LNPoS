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
#include <Conversion.h>
#include <ArduinoJson.h>
#include "qrcode.h"
#include "Bitcoin.h"
#include "esp_adc_cal.h"

#define KEYBOARD_I2C_ADDR     0X08
#define KEYBOARD_INT          5

#define PARAM_FILE "/elements.json"

String key_val;
String inputs;
String thePin;
String nosats;
String cntr = "0";
String lnurl;
String currency;
String key;
String preparedURL;
String baseURL;
String apPin = "9735"; //default AP pin
String masterKey;
String lnbitsServer;
String invoice;
String lnbitsBaseURL;
String secret;
bool onchainCheck = false;
bool lnCheck = false;
bool lnurlCheck = false;
int randomPin;

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
  tft.invertDisplay(true);
  tft.setRotation(1);
  h.begin();
  BTNA.begin();
  BTNB.begin();
  BTNC.begin();
  logo();
  FlashFS.begin(FORMAT_ON_FAIL);

//Get the saved details 
  File paramFile = FlashFS.open(PARAM_FILE, "r");
  StaticJsonDocument<2000> doc;
  DeserializationError error = deserializeJson(doc, paramFile.readString());
  JsonObject pinRoot = doc[0];
  const char* apPinChar = pinRoot["value"]; 
  apPin  = apPinChar;
  if (apPinChar != "") {
    apPin = apPinChar;
  }
  JsonObject maRoot = doc[1];
  const char* masterKeyChar = maRoot["value"]; 
  masterKey  = masterKeyChar;
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
  JsonObject baseURLRoot = doc[4];
  const char* baseURLChar = baseURLRoot["value"]; 
  lnbitsBaseURL  = baseURLChar;
  JsonObject secretRoot = doc[5];
  const char* secretChar = secretRoot["value"]; 
  secret  = secretChar;
  JsonObject currencyRoot = doc[6];
  const char* currencyChar = currencyRoot["value"]; 
  currency  = currencyChar;
  if(secret != ""){
    lnurlCheck = true;
  }

//Handle AP traffic
  server.on("/", []() {
    String content = "<h1>bitcoinPoS</br>Free open-source bitcoin PoS</h1>";
    content += AUTOCONNECT_LINK(COG_24);
    server.send(200, "text/html", content);
  });
  
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
  int timer = 0;
  while(timer < 3000){
    BTNA.read();   
    BTNB.read();
    BTNC.read();
    if (BTNA.wasReleased() || BTNB.wasReleased() || BTNC.wasReleased())
    {
      portal.begin();
      Serial.println("portal launched!");
      while(true){
        portal.handleClient();
      }
    }
  timer = timer + 200;
  delay(200);
  }
}

void loop() {
  Serial.println("nothing pressed");
  delay(3000);
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

void clearScreen()
{
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_WHITE);
  key_val = "";
  inputs = "";  
  nosats = "";
  cntr = "0";
}

void qrShowCode()
{
  tft.fillScreen(TFT_WHITE);
  lnurl.toUpperCase();
  const char *lnurlChar = lnurl.c_str();
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(20)];
  qrcode_initText(&qrcode, qrcodeData, 6, 0, lnurlChar);
  for (uint8_t y = 0; y < qrcode.size; y++)
  {
    // Each horizontal module
    for (uint8_t x = 0; x < qrcode.size; x++)
    {
      if (qrcode_getModule(&qrcode, x, y))
      {
        tft.fillRect(60 + 3 * x, 5 + 3 * y, 3, 3, TFT_BLACK);
      }
      else
      {
        tft.fillRect(60 + 3 * x, 5 + 3 * y, 3, 3, TFT_WHITE);
      }
    }
  }
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

void inputScreen()
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
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // White characters on black background
  tft.setTextSize(5);
  tft.setCursor(40, 100);  // To be compatible with Adafruit_GFX the cursor datum is always bottom left
  tft.print("LNURLPoS"); // Using tft.print means text background is NEVER rendered
  tft.setTextSize(2);
  tft.setCursor(42, 140);          // To be compatible with Adafruit_GFX the cursor datum is always bottom left
  tft.print("Powered by LNbits"); // Using tft.print means text background is NEVER rendered
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
  size_t payload_len = xor_encrypt(payload, sizeof(payload), (uint8_t *)key.c_str(), key.length(), nonce, sizeof(nonce), randomPin, inputs.toInt());
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
  lnurl = charLnurl;
  Serial.println(lnurl);
}

/*
 * Fills output with nonce, xored payload, and HMAC.
 * XOR is secure for data smaller than the key size (it's basically one-time-pad). For larger data better to use AES.
 * Maximum length of the output in XOR mode is 1+1+nonce_len+1+32+8 = nonce_len+43 = 51 for 8-byte nonce.
 * Payload contains pin, currency byte and amount. Pin and amount are encoded as compact int (varint).
 * Currency byte is '$' for USD cents, 's' for satoshi, 'E' for euro cents.
 * Returns number of bytes written to the output, 0 if error occured.
 */
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
