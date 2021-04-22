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

namespace screen {

	void init();
	void add_button_titles(const char* titles);
	void clear();
	void notice(const char* notice);
	void input_screen(const char* currency);
	void update_amounts(const char* amount_fiat, const char* amount_sats);
	void qrdisplay_screen(std::string data);
	void update_fiat(const char* currency, const char* amount_fiat);
	void update_sats(const char* amount_sats);
}


