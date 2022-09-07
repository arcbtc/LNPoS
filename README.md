![Alt text](images/banner.png?raw=true "banner")

# LNPoS

## Free and open-source bitcoin point-of-sale (includes device portal for easy setup!)

LNPoS includes:

- LNPoS (for online LN payments, original <a href="https://github.com/arcbtc/LNPoS">project</a>)
- LNURLPoS (for offline LN payments, original <a href="https://github.com/arcbtc/LNURLPoS">project</a> )
- OnChain (for onchain payments)
- LNURLATM (for making offline LN withdraw links).
  <br></br>

Original <a href="https://twitter.com/arcbtc/status/1484942260013838336">demo</a>

Join our telegram group <a href="https://t.me/makerbits">MakerBits</a>

## Configuring

> Press/hold any button on the keypad during startup for TDisplay, or any top button on M5Stack during logo screen, to trigger access portal.
> Default password is "ToTheMoon1" (without the quotes)

### LNPoS – Online Lightning Network payments

Invoices are generated and checked from LNbits install.

#### Setting up

Launch portal and enter an LNbits endpoint (ie.legend.lnbits.com), invoice key, and a fiat currency to make things easier for product pricing.

<img src="images/lnpos.png?raw=true" width="75%">

### LNURLPoS – Offline Lightning Network payments

A random pin generated on the device is encrypted and passed trough the payee. When the payment has been made the unencrypted pin is sent as a receipt. This function makes use of LNURL-pay protocol. Uses LNbits LNURLDevice extension.

#### Setting up

Launch portal and enter the string from the LNURLDevice extension on LNbits

<img src="images/lnurlpos.png?raw=true" width="75%">

### OnChain – Generate fresh addresses using an xPub

Use an xPub to generate a fresh address for every payment. Useful for large purchases. Includes a mempool.space QR so the payment can be verified.

#### Setting up

Launch portal and enter BIP39 xPub and a pin, to make the function secure.

<img src="images/onchain.png?raw=true" width="75%">

### LNURLATM – Meatbag ATM, give refunds, accept cash for sats.

Create withdraw/faucet links. Uses more-or-les. This function makes use of LNURL-withdraw protocol. Uses LNbits LNURLDevice extension.

#### Setting up

Launch portal and enter the string from the LNURLDevice extension on LNbits

<img src="images/lnurlatm.png?raw=true" width="75%">

<br></br>

<h2>Compilation</h2>

To compile the code:

- Install <a href="https://www.arduino.cc/en/software">Arduino IDE 1.8.19</a>
- Install ESP32 boards, using <a href="https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html#installing-using-boards-manager">boards manager</a>
- Role back ESP32 boards to 2.0.1 in boards manager
![image](https://user-images.githubusercontent.com/33088785/161862832-1269a12e-16ce-427c-9a92-df3ee573a1fb.png)
- Select the appropriate device in the Arduino IDE -> Tools -> Board -> ESP32 Arduino
  - For a generic ESP32 dev board, choose "ESP32 Dev Module"
  - For M5Stack, choose "M5Stack-Core-ESP32"
  - For Lilygo Tdisplay, choose "TTGO-LoRa32-OLED-V1" 
- Clone or download this code repository
- Copy (or symlink) <a href="lnPoS/libraries">lnPoS/libraries</a> into your Arduino install "libraries" folder
- Open <a href="lnPoS/LNPoS.ino">lnPoS/LNPoS.ino</a> in the Arduino IDE
- Set "#define HARDWARE_DEVICE 1" at the top of the file to the right value (read the comments)
- Compile and upload the code to device

> _Note: If using MacOS, you will need the CP210x USB to UART Bridge VCP Drivers available here https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers_
> If you are using **MacOS Big Sur or an Mac with M1 chip**, you might encounter the issue `A fatal error occurred: Failed to write to target RAM (result was 0107)`, this is related to the chipsest used by TTGO, you can find the correct driver and more info in this <a href="https://github.com/Xinyuan-LilyGO/LilyGo-T-Call-SIM800/issues/139#issuecomment-904390716">GitHub issue</a>


## LNPoS Options

> Currently, the hardware version <a href="/lnPoSTdisplay">TDisplay</a> is the most maintained and recommended.

- <a href="lnPoSBareBones">LNPoSBareBones</a> $8
- <a href="lnPoSM5Stack">LNPoSM5Stack</a> $70
- <a href="lnPoSTdisplay">LNPoSTdisplay</a> $15
