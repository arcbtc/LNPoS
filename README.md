
# bitcoinPoS 
## Free and open-source bitcoin point-of-sale (includes device portal for easy setup)

bitcoinPoS includes: 
* LNPoS (for online LN payments, original <a href="https://github.com/arcbtc/LNPoS">project</a>)
* LNURLPoS (for offline LN payments, original <a href="https://github.com/arcbtc/LNURLPoS">project</a> ) 
* OnChain (for onchain payments)
* LNURLATM (for making offline LN withdraw links).

### LNPoS – Online Lightning Network payments
Invoices are generated and checked from LNbits install.
#### Setting up
Launch portal and enter an LNbits endpoint (ie.legend.lnbits.com), invoice key, and a fiat currency to make things easier for product pricing.
<img src="images/lnpos.png?raw=true" width="75%">

### LNURLPoS – Offline Lightning Network payments
A random pin generated on the device is encrypted and passed trough the payee. When the payment has been made the unencrypted pin is sent as a receipt. This function makes use of LNURL-pay protocol.

#### Setting up
Launch portal and enter the string from the LNURLDevice extension on LNbits
<img src="images/lnurlpos.png?raw=true" width="75%">

### OnChain – Generate fresh addresses using an xPub
Use an xPub to generate a fresh address for every payment. Useful for large purchases. Includes a mempool.space QR so the payment can be verified. 

#### Setting up
Launch portal and enter BIP39 xPub and a pin, to make the function secure.
<img src="images/onchain.png?raw=true" width="75%">

### LNURLATM – Meatbag ATM, give refunds, accept cash for sats.
Create withdraw/faucet links. Uses more-or-les. This function makes use of LNURL-withdraw protocol.

#### Setting up
Launch portal and enter the string from the LNURLDevice extension on LNbits
<img src="images/lnurlatm.png?raw=true" width="75%">

## Hardware Options
### T-Display ($10-$20), either bare bones using a membrane keypad, and thanks to the success of LNURLPoS an official board.
#### <a href="bitcoinPoSTdisplay">bitcoinPoSTdisplay details</a>
![Alt text](images/tdisplay.png?raw=true "tdisplay")
---
### M5Stack ($50), tried and tested in the wild and comes with useful charging dock!
#### <a href="bitcoinPoSM5Stack">bitcoinPoSM5Stack details</a>
![Alt text](images/m5stack.png?raw=true "tdisplay")
---
### Super cheap bare bones ESP32/TFT/Keypad ($6), the cheapest of all the options, only costing around $8
#### <a href="bitcoinPoSBareBones">bitcoinPoSBareBones details</a>
![Alt text](images/cheap.png?raw=true "tdisplay")

## Software
