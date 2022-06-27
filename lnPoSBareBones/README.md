<img src="../images/cheap.png?raw=true">

<h2 align="center">
⚡⚡⚡⚡LNPoSBareBones ($10)⚡⚡⚡⚡
</h2>

This is the cheap version of the project, with a litte bulk buying you can easily get the price down to $8ish off the shelf:

- <a href="https://www.aliexpress.com/item/1005002363389886.html">ESP32 dev board</a>
- <a href="https://www.aliexpress.com/item/1005003505032992.html">TFT 1,4inch</a>
- <a href="https://www.aliexpress.com/item/32993999306.html">keypad membrane</a>
- Case:
  - <a href="https://www.cryptocloaks.com/download/9142/">Quickening-Box</a> from <a href="https://www.cryptocloaks.com/file-factory/">CryptoCloaks FileFactory</a>

* Install <a href="https://www.arduino.cc/en/software">Arduino IDE 1.8.19</a>
* Install ESP32 boards, using <a href="https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html#installing-using-boards-manager">boards manager</a>
* Download this repo
* Copy these <a href="libraries">libraries</a> into your Arduino install "libraries" folder
* Open this <a href="LNPoSBareBones.ino">LNPoSBareBones.ino</a> file in the Arduino IDE
* Select "ESP32 Dev Module" from tools>board
* build device
> TFT PIN MAP: 
> * VCC - 5V
> * GND - GND
> * CS - GPIO5
> * Reset - GPIO16
> * AO (DC) - GPI17
> * SDA (MOSI) - GPIO23
> * SCK - GPIO18
> * LED - 3.3V
> Keypad 
> * 12-32
* Upload to device

> _Note: If using MacOS, you will need the CP210x USB to UART Bridge VCP Drivers available here https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers_
