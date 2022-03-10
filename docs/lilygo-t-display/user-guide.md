# LNPoS LilyGo TTGO Userguide

## Installation

Following the installation guide in the main README.md file.

**Membrane Keypad**
If you want to use the membrane keypad with a TTGO board, uncomment the `KeyPadTypes keypadType = MEMBRANE_4X4;` around line 510 of the Arduino sketch.

## Set up

Boot into AP mode by pressing `*` when powering on the device. Scan the QR code with a mobile device to connect to the LNPoS access point

1. Set up wifi connections if needed
1. Configure LNPoS settings
1. Reboot

## Use

**Menu selection**
Press `1`, `4` or `*` to navigate through the main menu.
Press `#` to select a menu item

**Brightness control**
When displaying a QR code, press `1` or `4` to increase or decrease the screen brightness

**Sleep mode**
The device will go into a deep sleep mode after 30 seconds. Press `*` to wake from this sleep mode.

**Non-compatible wallets**
The following wallets do no support LNBits' LNURLw format.

+ Muun
+ ...

## Cases

Files for printing the devices cases can be found in the `cases` directory of the repository.

### Case Installation

TODO

Switches to use

### Powering the device by battery

TODO

1. Battery type - 3.7v LiPO with charge protection
2. Connectors - JST 1.25mm
3. Soldering the switch harness - Photo
4. Stick battery in place with double sided tape to stop is whizzing around the case
5. Hot gluing wires - To prevent wire breakages and shorting
