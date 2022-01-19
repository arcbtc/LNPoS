
# bitcoinPoS 
## Free and open-source bitcoin point-of-sale (includes device portal for easy setup)

BitcoinPoS includes: 
* <a href="https://github.com/arcbtc/LNPoS">LNPoS</a> (for online LN payments)
* <a href="https://github.com/arcbtc/LNURLPoS">LNURLPoS</a> (for offline LN payments) 
* OnChain (for onchain payments)
* LNURLATM (for making offline LN withdraw links).

## LNPoS – Online Lightning Network payments
The LNPoS project has been incorporated into bitcoinPoS for online bitcoin lightning network payments. 

#### Setting up
Launch portal and enter an LNbits endpoint and invoice key.
![Alt text](images/lnpos.png?raw=true "LNPoS")

## LNURLPoS – Offline Lightning Network payments
The LNURLPoS project has been incorporated into bitcoinPoS. A random pin generated on the device is encrypted and passed trough the payee. When the payment has been made the unencrypted pin is sent as a receipt. This function makes use of LNURL-pay protocol.

#### Setting up
Launch portal and enter the string from the LNURLDevice extension on LNbits
![Alt text](images/lnurlpos.png?raw=true "LNPoS")

## OnChain – Generate fresh address using an xPub
Use an xPub to generate a fresh address for every payment. Useful for large purchases. Includes a mempool.space QR so the payment can be verified. 

#### Setting up
Launch portal and enter  a Bip39 xPub
![Alt text](images/onchain.png?raw=true "LNPoS")

## LNURLATM – Meatbag ATM, give refunds, accept cash for sats.
Create withdraw/faucet links. Uses more-or-les. This function makes use of LNURL-withdraw protocol.

#### Setting up
Launch portal and enter the string from the LNURLDevice extension on LNbits
![Alt text](images/lnurlatm.png?raw=true "LNPoS")

## Hardware



## Software
