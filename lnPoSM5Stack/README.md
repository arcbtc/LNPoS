
<h2 align="center">
⚡⚡⚡⚡LNPoSM5Stack ($70)⚡⚡⚡⚡
</h2>

Currently M5Stack list this product as EOL, but they promise a new version will be out soon https://twitter.com/m5stack/status/1505411398008213510?s=21

Video tutorial <a href="https://www.youtube.com/watch?v=d0kivRdwWCU">https://www.youtube.com/watch?v=d0kivRdwWCU</a>

The <a href="https://shop.m5stack.com/products/face?variant=17290437623898">M5Stack</a> is an excellent ESP32 based off the shelf device that comes with a charging dock. We had a bitcoin PoS running, in the iconic Room77 bar in Berlin, for a year, and it performed very well.

General compilation instructions are in the <a href="../README.md">main README</a>.

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
