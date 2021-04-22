/*
	Copyright (C) 2021 Arc, tomichec (Tomas Stary)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <M5Stack.h>
#include "FS.h"
#include "SPIFFS.h"

#include "screen.h"
#include "config.h"
#include "network.h"
#include "cashback.h"

#define POS 1
#define CASHBACK 2

// flag to enable keypad -- comment to use the build in buttons
// #define KEYPAD

String key_val;

#ifdef KEYPAD
#define KEYBOARD_I2C_ADDR     0X08
#define KEYBOARD_INT          5

void get_keypad();


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

#endif

int operation_mode;


/////////////////////SETUP////////////////////////

void setup() 
{
  M5.begin();
  Serial.println("LNPoS Launching");


#ifdef KEYPAD
  Wire.begin();
#endif

  screen::init();
  screen::add_button_titles("PORTAL   POS   CASHBACK");
  config::init();

  while (true) {
	  if (M5.BtnA.wasPressed()){
		  screen::notice("Portal Launched");
		  config::erase_config();
		  config::wifi_manager_portal();
		  break;
	  } else if ( M5.BtnB.wasPressed() ) {
		  operation_mode = POS;
		  screen::notice("Loading PoS");
		  config::read_config();
		  config::start_wifi();
		  network::fetchExchangeRate(config::get_currency());
		  break;
	  } else if ( M5.BtnC.wasPressed() ) {
		  operation_mode = CASHBACK;
		  screen::notice("Loading Cashback");
		  config::read_config();
		  // config::start_wifi();
		  // network::fetchExchangeRate(config::get_currency());
		  cashback::init();
		  // config::start_wifi();
		  break;
	  }
	  delay(50);
	  M5.update();
  }
  Serial.println("Setup complete");
}

///////////////////MAIN LOOP//////////////////////

void loop() 
{
	const char* currency = config::get_currency();
	double conversion = network::get_exchange_rate();
	std::string qrcode_data = "";
	String amount_sats;
	String amount_fiat;

	String inputs;
	float temp;  
	float satoshis;


	screen::input_screen(currency);

#ifndef KEYPAD
  temp = 0.01;
#endif
  while (true) {
    M5.update();
#ifdef KEYPAD
    get_keypad(); 
#endif
    if (M5.BtnC.wasPressed()) {
		screen::notice("PROCESSING");
		delay(500);

		switch (operation_mode) {
		case POS:
			qrcode_data = network::getinvoice(amount_sats);
			screen::qrdisplay_screen(qrcode_data);
			while(!network::checkinvoice()){
				delay(3000);
			}
			break;
		case CASHBACK:
			qrcode_data = cashback::get_lnurl(atof(amount_fiat.c_str()));
			screen::qrdisplay_screen(qrcode_data);
			delay(60*1000);
			break;
		}

      M5.Lcd.fillScreen(BLACK);
      key_val = "";
      inputs = "";
    }
    else if (M5.BtnB.wasReleased()) {
	  temp *= 1.1;
    }
    else if (M5.BtnA.wasReleased()) {
	  screen::input_screen(currency);
      key_val = "";
      inputs = "";  
      amount_sats = "";
#ifndef KEYPAD
      temp = 1;
#endif
    }
#ifdef KEYPAD
    inputs += key_val;
    temp = inputs.toInt();
    temp = temp / 100;
#endif
    amount_fiat = temp;
    satoshis = temp/conversion;
    int intsats = (int) round(satoshis*100000000.0);
	
    amount_sats = String(intsats);

	screen::update_amounts(amount_fiat.c_str(),amount_sats.c_str());

    delay(100);
    key_val = "";
  }
}


