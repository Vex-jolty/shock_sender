# Shock Sender README

## What this does

Simply put, whenever `sendShock` is called, it sends a shock of a specific intensity to every PiShock owned by the user.

This was originally designed for Ship of Harkinian (hence the `intensity_per_quarter` variable, referring to a quarter of a heart), but is now being independently published so anyone can modify it for their own projects. Feel free to fork it for your own mods :3

Please note that you *must* call the `run` function *before* running `sendShock`, as that will start the Websocket client.

## How to set it up

In the `shock_config.ini` file, you'll find 5 variables:

```ini
max_shock #the maximum intensity to be sent to PiShock

intensity_per_quarter #the intensity for each quarter-heart lost

username #your PiShock username

api_key #your PiShock API key

duration_ms #the duration of the shocks, in milliseconds
```

Just set all these up with their respective values, and you should be good to go!

MacOS is currently unsupported, and there are no plans to support it. If you're a Mac user, feel free to run this under Proton and report any issues back.

## FAQ

### **Q:** Why is there no Mac version?

**A:** Because I'm Brazilian and the cheapest Mac Mini costs about 7,500 BRL (roughly 1,360 USD at time of writing), and I'm not made of money.

### **Q:** How many shockers does this connect to?

**A:** Every single one you have, all at once. The duration and intensity aren't customizable per shocker, though. All of them will use the same values.
