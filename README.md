
# bitcoinPoS 
## Free and open-source bitcoin point-of-sale (includes device portal for easy setup)

bitcoinPoS includes: 
* LNPoS (for online LN payments, original <a href="https://github.com/arcbtc/LNPoS">project</a>)
* LNURLPoS (for offline LN payments, original <a href="https://github.com/arcbtc/LNURLPoS">project</a> ) 
* OnChain (for onchain payments)
* LNURLATM (for making offline LN withdraw links).

### LNPoS – Online Lightning Network payments
The LNPoS project has been incorporated into bitcoinPoS for online bitcoin lightning network payments. 

#### Setting up
Launch portal and enter an LNbits endpoint (ie.legend.lnbits.com), invoice key, and a fiat currency to make things easier for product pricing.
![Alt text](images/lnpos.png?raw=true "LNPoS")

### LNURLPoS – Offline Lightning Network payments
The LNURLPoS project has been incorporated into bitcoinPoS. A random pin generated on the device is encrypted and passed trough the payee. When the payment has been made the unencrypted pin is sent as a receipt. This function makes use of LNURL-pay protocol.

#### Setting up
Launch portal and enter the string from the LNURLDevice extension on LNbits
![Alt text](images/lnurlpos.png?raw=true "LNURLPoS")

### OnChain – Generate fresh addresses using an xPub
Use an xPub to generate a fresh address for every payment. Useful for large purchases. Includes a mempool.space QR so the payment can be verified. 

#### Setting up
Launch portal and enter BIP39 xPub and a pin, to make the function secure.
![Alt text](images/onchain.png?raw=true "OnChain")

### LNURLATM – Meatbag ATM, give refunds, accept cash for sats.
Create withdraw/faucet links. Uses more-or-les. This function makes use of LNURL-withdraw protocol.

#### Setting up
Launch portal and enter the string from the LNURLDevice extension on LNbits
![Alt text](images/lnurlatm.png?raw=true "ATM")

## Hardware OPtions
#### T-Display ($10-$20), either bare bones using a membrane keypad, and thanks to the success of LNURLPoS an official board <a href="https://www.aliexpress.com/item/1005003607005382.html">LilyGo AliExpress shop</a>
![Alt text](images/tdisplay.png?raw=true "tdisplay")
#### M5Stack ($50), tried and tested in the wild and comes with useful charging dock!
![Alt text](images/m5stack.png?raw=true "tdisplay")
#### Super cheap bare bones ESP32/TFT/Keypad ($6), the cheapest of all the options, only costing around $8
#### <a href="">Project page</a>
![Alt text](images/cheap.png?raw=true "tdisplay")

## Software
