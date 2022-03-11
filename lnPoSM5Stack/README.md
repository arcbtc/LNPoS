<img src="../images/m5stack.png?raw=true">

<h2 align="center">
⚡⚡⚡⚡LNPoSM5Stack⚡⚡⚡⚡
</h2>

The <a href="https://shop.m5stack.com/products/face?variant=17290437623898">M5Stack</a> is an excellent ESP32 based off the shelf device that comes with a charging dock. We had a bitcoin PoS running, in the iconic Room77 bar in Berlin, for a year, and it performed very well.

<a href="https://www.youtube.com/watch?v=d0kivRdwWCU"><img style="width:300px" src="https://user-images.githubusercontent.com/33088785/157879012-54d2daa2-067f-4b9e-a922-22191e404015.png"></a>

- Install <a href="https://www.arduino.cc/en/software">Arduino IDE 1.8.19</a>
- Install ESP32 boards, using <a href="https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html#installing-using-boards-manager">boards manager</a>
- Download this repo
- Copy these <a href="libraries">libraries</a> into your Arduino install "libraries" folder
- Open this <a href="LNPoSM5Stack.ino">LNPoSM5Stack.ino</a> file in the Arduino IDE
- Select "M5Stack-Core-ESP32" from tools>board
- Upload to device

> _Note: If using MacOS, you will need the CP210x USB to UART Bridge VCP Drivers available here https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers_

#### Example arduino Settings

Working Arduino settings in "Tools":
- Board: M5Stack-Core-ESP32
- Upload Speed: 921600
- Flash Frequency: 80Mhz
- FlashMode: QIO
- Partition Scheme: Default
- Core Debug Level: None or Error
- Port: /dev/ttyUSB0 (could very well be different, such as /dev/cu.SLAB_USBtoUART) 

Common port locating/debug method: 
  - checking available ports
  - unplugging M5Stack
  - check which Port is not listed anymore
  - plug M5Stack back in and use that Port

> If your **M5Stack-screen stays black**  after flashing, check if Tools/Flash Mode is set to **QIO** and the partition scheme to "default/standard"
> 
#### Demo
> Drunk Germans giving a terrible demo https://twitter.com/rootzoll/status/1162351526867800064
