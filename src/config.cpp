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
#include "config.h"

namespace config {

#define _(a,b,c,d) char a[c] = d;
#include "menu.h"
#undef _

	char lnbits_amount[500] = "1000";
	char high_pin[5] = "16";
	char time_pin[20] = "3000";
	char static_ip[16] = "10.0.1.56";
	char static_gw[16] = "10.0.1.1";
	char static_sn[16] = "255.255.255.0";

	bool shouldSaveConfig = false;
	const char* spiffcontent = "";
	String spiffing; 

	DynamicJsonDocument json(2*1024); 
	WiFiManager wm;

	void init() {
		mount_file_system();
	}

	void mount_file_system() {
		Serial.println("mounting file system...");
		while(!SPIFFS.begin(true)){
			Serial.println("failed to mount file system");
			delay(200);
		}

	}

	void erase_config () {
		Serial.println("Erasing config file.");
		File file = SPIFFS.open("/config.txt", FILE_WRITE);
		file.print("placeholder");
	}

	void read_config() {
		File file = SPIFFS.open("/config.txt");
		spiffing = file.readStringUntil('\n');
		spiffcontent = spiffing.c_str();
		deserializeJson(json, spiffcontent);

		if(String(spiffcontent) != "placeholder") {
#define _(a,b,c,d) strcpy(a, json[#a]);
#include "menu.h"
#undef _
		}
	}

	void start_wifi() {
		wm.setSaveConfigCallback(saveConfigCallback);

		if (!wm.autoConnect("⚡LNPoS⚡", wifi_password)) {
			Serial.println("failed to connect and hit timeout");
			delay(3000);
			ESP.restart();
			delay(5000);
		}
		Serial.println("local ip:");
		Serial.println(WiFi.localIP());
		Serial.println(WiFi.gatewayIP());
		Serial.println(WiFi.subnetMask());
	}

	void save_config() {
		Serial.println("saving config");
#define _(a,b,c,d) json[#a] = a;
#include "menu.h"
#undef _

		File configFile = SPIFFS.open("/config.txt", "w");
		if (!configFile) {
			Serial.println("failed to open config file for writing");
		}
		serializeJsonPretty(json, Serial);
		serializeJson(json, configFile);
		configFile.close();
	}

	void wifi_manager_portal()
	{
		wm.resetSettings();


#define _(a,b,c,d) WiFiManagerParameter custom_##a(#a,#b,a,c);
#include "menu.h"
#undef _

#define _(a,b,c,d) wm.addParameter(&custom_##a);
#include "menu.h"
#undef _

		start_wifi();

#define _(a,b,c,d) strcpy(a, custom_##a.getValue());
#include "menu.h"
#undef _

		if (shouldSaveConfig) {
			save_config();
			shouldSaveConfig = false;
		}

	}

	void saveConfigCallback ()
	{
		Serial.println("Should save config");
		shouldSaveConfig = true;
	}

	// define functions to get parameters
#define _(a,b,c,d) char* get_##a() {return a;}
#include "menu.h"
#undef _

}
