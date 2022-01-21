<img src="../images/tdisplay.png?raw=true">

<h2 align="center">
⚡⚡⚡⚡bitcoinPoSTdisplay⚡⚡⚡⚡
</h2>

The Lilygo <a href="https://www.aliexpress.com/item/33048962331.html">Tdisplay</a> is a cheap amd small ESP32/screen developement board. You can either attach a <a href="https://www.aliexpress.com/item/32993999306.html">keypad membrane</a>, or use the <a href="https://www.aliexpress.com/item/1005003607005382.html">breakout board</a> Lilygo specifically made for the LNURLPoS!

- Install <a href="https://www.arduino.cc/en/software">Arduino IDE 1.8.19</a>
- Install ESP32 boards, using <a href="https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html#installing-using-boards-manager">boards manager</a>
- Download this repo
- Copy these <a href="libraries">libraries</a> into your Arduino install "libraries" folder
- Open this <a href="bitcoinPoSTdisplay.ino">bitcoinPoSTdisplay.ino</a> file in the Arduino IDE
- Select "TTGO-LoRa32-OLED-V1" from tools>board
- Upload to device

> *Note: If using MacOS, you will need the CP210x USB to UART Bridge VCP Drivers available here https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers*

> *Note: You may need to roll your ESP32 boards back to an earlier version in the Arduino IDE, by using tools>boards>boards manager, searching for esp. I use v1.0.5(rc6), and have also used v1.0.4 which worked.*
