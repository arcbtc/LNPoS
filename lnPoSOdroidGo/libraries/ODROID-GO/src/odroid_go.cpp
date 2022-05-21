// Copyright (c) M5Stack. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "odroid_go.h"

void ODROID_GO::begin(unsigned long baud) {

    // UART
    Serial.begin(baud);

    Serial.flush();
    Serial.print("ODROID_GO initializing...");

    Speaker.begin();

    pinMode(BUTTON_A_PIN, INPUT_PULLUP);
    pinMode(BUTTON_B_PIN, INPUT_PULLUP);
    pinMode(BUTTON_MENU, INPUT_PULLUP);
    pinMode(BUTTON_SELECT, INPUT_PULLUP);
    pinMode(BUTTON_START, INPUT_PULLUP);
    pinMode(BUTTON_VOLUME, INPUT_PULLUP);
    pinMode(BUTTON_JOY_Y, INPUT_PULLDOWN);
    pinMode(BUTTON_JOY_X, INPUT_PULLDOWN);

    // ODROID_GO LCD INIT
    lcd.begin();
    lcd.setRotation(1);
    lcd.fillScreen(BLACK);
    lcd.setCursor(0, 0);
    lcd.setTextColor(WHITE);
    lcd.setTextSize(1);
    lcd.setBrightness(255);

    // Battery
    battery.begin();

    Serial.println("OK");
}

void ODROID_GO::update() {

    //Button update
    BtnA.read();
    BtnB.read();
    BtnMenu.read();
    BtnVolume.read();
    BtnSelect.read();
    BtnStart.read();
    JOY_Y.readAxis();
    JOY_X.readAxis();

    //Speaker update
    Speaker.update();
    battery.update();
}
ODROID_GO GO;
