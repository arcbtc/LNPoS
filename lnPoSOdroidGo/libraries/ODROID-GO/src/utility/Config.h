#ifndef _CONFIG_H_
#define _CONFIG_H_

// TFT LCD via SPI
#define TFT_DC 21            // Data/Control pin
#define TFT_CS 5             // Chip Select
#define TFT_LED_PIN 14       // Backlight enable
#define TFT_MOSI 23
#define TFT_MISO 19
#define TFT_SCLK 18
#define TFT_RST -1           // (not connected)

// microSD Card (TF) via SPI
#define SD_CARD_CS 22

// Digital Buttons
#define BUTTON_A_PIN 32
#define BUTTON_B_PIN 33

#define BUTTON_MENU 13
#define BUTTON_SELECT 27
#define BUTTON_VOLUME 0
#define BUTTON_START 39

// Analog Buttons
#define BUTTON_JOY_Y 35
#define BUTTON_JOY_X 34

// Speaker
#define SPEAKER_ENABLE_PIN 25
#define SPEAKER_PIN 26
#define TONE_PIN_CHANNEL 0   // PWM Channel

// Status LED (blue)
#define LED_STATUS_PIN 2

#endif
