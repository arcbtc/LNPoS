#ifndef _LNPOS_H
#define _LNPOS_H

#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
using WebServerClass = WebServer;
fs::SPIFFSFS &FlashFS = SPIFFS;
#define FORMAT_ON_FAIL true

#include <AutoConnect.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Hash.h>
#include <ArduinoJson.h>
#include "qrcoded.h"
#include "Bitcoin.h"


#if HARDWARE_DEVICE == 1 // For BareBones
  #include <Keypad.h>
  void complete();
  void getInvoice();
  void getKeypad(bool isATMPin, bool justKey, bool isLN, bool isATMNum);
  void getSats();
#elif HARDWARE_DEVICE == 2 // For M5Stack
  #include <JC_Button.h>
  #include <Wire.h>
  #include "esp_adc_cal.h"
  void inputScreen(bool online, bool ATM);
  void complete();
  void getKeypad(bool isATMPin, bool isLNPin, bool isLN);
  void getSats();
  void getInvoice();
#elif HARDWARE_DEVICE == 3 // For TDisplay
  #include <Keypad.h>
  void getKeypad(bool isATMPin, bool justKey, bool isLN, bool isATMNum);
  bool getInvoice();
  bool getSats();
#endif

enum InvoiceType {
  LNPOS,
  LNURLPOS,
  ONCHAIN,
  LNURLATM,
  PORTAL
};

bool checkInvoice();
void callback();
void error(String message);
void error(String message, String additional);
float getInputVoltage();
String getValue(String data, char separator, int index);
void handleBrightnessAdjust(String keyVal, InvoiceType invoiceType);
void inputScreenOnChain();
unsigned int getBatteryPercentage();
void isATMMoneyNumber(bool cleared);
void isATMMoneyPin(bool cleared);
boolean isLilyGoKeyboard();
void isLNMoneyNumber(bool cleared);
void isLNURLMoneyNumber(bool cleared);
bool isPoweredExternally();
void lnMain();
void lnurlATMMain();
void lnurlPoSMain();
void logo();
//void loop_device();
void makeLNURL();
void maybeSleepDevice();
void menuLoop();
void onchainMain();
void paymentSuccess();
void portalLaunch();
void printSleepAnimationFrame(String text, int wait);
void processing(String message);
void qrShowCodeln();
void qrShowCodeLNURL(String message);
void qrShowCodeOnchain(bool anAddress, String message);
//void setup_device();
void showPin();
void sleepAnimation();
int xor_encrypt(uint8_t *output, size_t outlen, uint8_t *key, size_t keylen, uint8_t *nonce, size_t nonce_len, uint64_t pin, uint64_t amount_in_cents);

#endif // #ifndef _LNPOS_H
