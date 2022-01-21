<img src="../images/m5stack.png?raw=true">

<h2 align="center">
⚡⚡⚡⚡bitcoinPoSM5Stack⚡⚡⚡⚡
</h2>

The <a href="https://shop.m5stack.com/products/face?variant=17290437623898">M5Stack</a> is an excellent ESP32 based off the shelf device that comes with a charging dock. We had a bitcoin PoS running, in the iconic Room77 bar in Berlin, for a year, and it performed very well.

* Install <a href="https://www.arduino.cc/en/software">Arduino IDE 1.8.19</a> 
* Install ESP32 boards, using <a href="https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html#installing-using-boards-manager">boards manager</a>
* Download this repo
* Copy these <a href="libraries">libraries</a> into your Arduino install "libraries" folder
* Open this <a href="bitcoinPoSM5Stack.ino">bitcoinPoSM5Stack.ino</a> file in the Arduino IDE
* Select "M5Stack-Core-ESP32" from tools>board
* Upload to device

> *Note: If using MacOS, you will need the CP210x USB to UART Bridge VCP Drivers available here https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers*

> *Note: You may need to roll your ESP32 boards back to an earlier version in the Arduino IDE, by using tools>boards>boards manager, searching for esp. I use v1.0.5(rc6), and have also used v1.0.4 which worked.*

> Drunk Germans giving a terrible demo https://twitter.com/rootzoll/status/1162351526867800064

