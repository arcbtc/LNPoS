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
#include <WiFiManager.h> 
#include <ArduinoJson.h>

namespace config {
	void init();
	void mount_file_system();
	void erase_config();

	void wifi_manager_portal();
	void save_config();
	void saveConfigCallback ();
	void read_config();
	void start_wifi();

	/* functions to get parameters */
#define _(a,b,c,d) char* get_##a();
#include "menu.h"
#undef _
}
