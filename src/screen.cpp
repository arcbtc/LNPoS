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
#include "screen.h"
#include "network.h"

#include "logo.c"

namespace screen {

	void init() {
		M5.begin();
		M5.Lcd.drawBitmap(0, 0, 320, 240, (uint8_t *)logo_map);
	}

	void add_button_titles(const char* titles){
		M5.Lcd.setTextSize(2);
		M5.Lcd.setCursor(30, 220);
		M5.Lcd.println(titles);
	}

	void clear() {
		M5.Lcd.fillScreen(BLACK);
		M5.Lcd.setCursor(20, 80);
		M5.Lcd.setTextSize(3);
		M5.Lcd.setTextColor(TFT_WHITE);
	}

	void notice(const char* notice) { 
		clear();
		M5.Lcd.println(notice);
	}

	void input_screen(const char* currency)
	{
		clear();
		M5.Lcd.setCursor(0, 88);
		M5.Lcd.println(String(currency) + ": ");
		if (network::hasExchangeRate()){
			M5.Lcd.setCursor(0, 136);
			M5.Lcd.println("SATS: ");
		}
		add_button_titles(" RESET   +10%     OK");
	}

	void update_fiat(const char* currency, const char* amount_fiat){
		clear();
		M5.Lcd.setCursor(0, 88);
		M5.Lcd.println(String(currency) + ": ");

		M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
		M5.Lcd.setCursor(92, 88);
		M5.Lcd.println(amount_fiat);

		add_button_titles(" RESET   +10%     OK");
	}

	void update_sats(const char* amount_sats){
		clear();
		M5.Lcd.setCursor(0, 136);
		M5.Lcd.println("SATS: ");

		M5.Lcd.setTextSize(3);
		M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
		M5.Lcd.setCursor(92, 136);
		M5.Lcd.println(amount_sats);

		add_button_titles(" RESET   +10%     OK");
	}


	void update_amounts(const char* amount_fiat, const char* amount_sats){


		M5.Lcd.setTextSize(3);
		M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
		M5.Lcd.setCursor(92, 88);
		M5.Lcd.println(amount_fiat);
		if (network::hasExchangeRate()){
			M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
			M5.Lcd.setCursor(92, 136);
			M5.Lcd.println(amount_sats);
		}
	}

	void qrdisplay_screen(std::string data)
	{  
		M5.Lcd.fillScreen(BLACK); 
		M5.Lcd.qrcode(data.c_str(),45,0,240,14);
		delay(100);
	}


}
