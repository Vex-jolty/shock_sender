# Shock Sender README

## What this mod does

Simply put, every time Link takes damage, it triggers a shock. If Link dies, it sends the maximum value set by the player.

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

Please note that *this mod will not work under DirectX*. You *have* to use OpenGL instead.

MacOS is currently unsupported, and there are no plans to support it. If you're a Mac user, feel free to run this under Proton and report any issues back.

## FAQ

### **Q:** Why is there no Mac version?

**A:** Because I'm Brazilian and the cheapest Mac Mini costs about 7,500 BRL (roughly 1,360 USD at time of writing), and I'm not made of money.

### **Q:** Why does DirectX not work?

**A:** Because I compiled this on Arch Linux and cross-compilation is a bitch. MinGW just doesn't have a good DirectX implementation.

### **Q:** Why does the Windows version have so many DLLs?

**A:** Same reason as above. The only DLL I created myself is `shock_sender.dll`, all others come from MinGW. Since this wasn't compiled with MSVC, I can't just use the DLLs that come with Windows, so I decided to place them here for your convenience. The other options would be to (1) re-compile multiple libraries into static libraries, since some are only available as shared libraries in pacman or the AUR, and incorporate them into the .exe, which would take considerable time and make the executable significantly larger, or (2) have you hunt all these DLLs down online yourself. Doing it this way felt like the best choice.

### **Q:** Why did you compile this on Linux?

**A:** Because that's what I use on both my personal desktop and my work laptop, and there's no way in hell I'd consider switching just to compile a gag project like this.

### **Q:** Do I need to restart the game when I change the intensity of the shocks, the max value or the duration?

**A:** For the most part, no. The only situation where you'll need to restart is if you want to set the max above 100.

### **Q:** Wait, I can set the max value above 100?

**A:** Yep! But you'll be warned beforehand and have to assume all the risks. I'm not liable if you decide to set your skin on fire.

### **Q:** How many shockers does this connect to?

**A:** Every single one you have, all at once. The duration and intensity aren't customizable per shocker, though. All of them will use the same values.
