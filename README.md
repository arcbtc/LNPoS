![Alt text](images/banner.png?raw=true "banner")

<h1>
LNPoS
</h1>

## Free and open-source bitcoin point-of-sale (includes device portal for easy setup!)

> Currently (until a merge of all versions into one codebase), the hardware version <a href="/lnPoSTdisplay">TDisplay</a> is the most maintained and recommended.

LNPoS includes:

- LNPoS (for online LN payments, original <a href="https://github.com/arcbtc/LNPoS">project</a>)
- LNURLPoS (for offline LN payments, original <a href="https://github.com/arcbtc/LNURLPoS">project</a> )
- OnChain (for onchain payments)
- LNURLATM (for making offline LN withdraw links).
  <br></br>

Original <a href="https://twitter.com/arcbtc/status/1484942260013838336">demo</a>

Join our telegram group <a href="https://t.me/makerbits">MakerBits</a>

<h2>
Configuring
</h2>

> Press/hold any button on startup for TDisplay, or any top button on M5Stack during logo screen, to trigger access portal.
> Default password is "ToTheMoon1"

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

<h2>
LNPoS Options
</h2>
<h3>
 <ul>
  <li><a href="lnPoSM5Stack">LNPoSM5Stack</a> $70</li>
  <li><a href="lnPoSTdisplay">LNPoSTdisplay</a> $15</li>
  <li><a href="lnPoSBareBones">LNPoSBareBones</a> $8</li>
  <li><a href="lnPoSOdroidGo">LNPoSOdroidGo</a></li>
 </ul>
</h3>
