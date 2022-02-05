# HP3457-OLED-display
A modern replacement for the low contrast HP3457 Multimeter LCD

HP3457 is a great multimeter of the pre Keysight and pre Agilent era from HP.
If you do some research on it, you get to learn a lot of details of how much nice engineering work has been put into the development of this multimeter. It is certainly one of the things, that was developed by great experienced engineers and is a masterpiece in finding proper solutions for technical challenges to push accuracy but also measurement speed on the right level.

HOWEVER: The display is crap. It is a low contrast display with small characters and extremly hard to read under most conditions. When I got mine, I rarely used it just for that reason.

I researched a bit on possible options and found 2 solutions:
- Mount a backlight behind the LCD. This is a very risky task, but I tried it out: The contrast gets even worse and actually it felt even like a downgrade of the display.
- A great sophisticated replacement LED display done by an EEVBlog user called "Xi": [https://www.eevblog.com/forum/projects/led-display-for-hp-3457a-multimeter-i-did-it-)](https://www.eevblog.com/forum/projects/led-display-for-hp-3457a-multimeter-i-did-it-)). Unfortunately this solution requires some difficult production techniques (milling LED displays to insert dots/comma leds, milling PCBs, ...).

I looked for solutions that were simpler to realize, such as matching TFT IPS panels, big OLED screens but failed to find a matching display.

After long thinking I had an idea... Let's use the cheapest OLED display that have very good availability: 0.87" i2c displays from Aliexpress. They are smaller than the required 10mm digit distance and just fit also mechanically into the display housing, without the need to cut pieces of the housing away.

To explain the idea - here 2 pictures:

<img src="https://github.com/xyphro/HP3457-OLED-display/blob/main/photos/FirstDigitWorking.jpg?raw=true" width="40%"/>
<img src="https://github.com/xyphro/HP3457-OLED-display/blob/main/photos/OledMounting.jpg?raw=true" width="40%"/>

# The final result:

This video shows my final solution to the problem:

[![](https://img.youtube.com/vi/EYaLf55-z-o/0.jpg)](https://youtu.be/EYaLf55-z-o) 

(Well, not the most entertaining video in this world, but... It serves the job :-) )

# Building the display yourself:

This repositories contains sourcecode and hw design files in "raw" formats, but also the binary output.

I ordered the PCBs originally at JLCPCB. Both PCBs are located on a single panel and you can cut them in half after receiving them. The Gerber files are located under hardware/gerber.
In case you want to modify the PCB yourself, feel free to do so (but share it back) and use the files located under hardware/eagle to create your own design.

The software was created using Atmel Studio 7.0 and the project can be directly opened with it for editing if desired.

Don't forget to program the fuses of the ATMega8 MCU and select 8 MHz internal RC oscillator as clock source (otherwise it runs too slow with its default 1 MHz clock only).

NOTE: Ensure to buy the 0.87" variant of the OLEDS. There are also popular 0.91" variants available, but they are to wide!

# Annuciators:

Annuciators are the small triangles / TEXT labels underneath the display. They are used to signal state information of the multimeter.

There are 3 different flavors realized. To select them when building the software, edit this define in annuciatorsel.h:
#define ANNUCIATORS_PERSONALITY HP3457_ICONS

And select any of the 3 below ones:

- HP3457_ORIGINAL - These are the small triangles, that rely on the information printed on the frontpanel underneath the display:

<img src="https://github.com/xyphro/HP3457-OLED-display/blob/main/photos/HP3457A%20triangle.jpg?raw=true" width="40%"/>

- HP3457_ICONS  - Small textual labels on the OLEDS itself:

<img src="https://github.com/xyphro/HP3457-OLED-display/blob/main/photos/annuciators_icons.jpg?raw=true" width="40%"/>

- HP3478_ICONS - textual labels matching the HP3478 / 3468 display


The software/hex folder also contains precompiled .hex images for those 3 different annuciators versions.

# Putting it together

After soldering the PCBs and flashing the MCU, you can mount the PCB to your lovely HP3457 multimeter. The original press fit DIP socket can be directly plugged into the PCB.
The OLED display requires a second voltage source to power the OLEDs themselfes. This can be an AC supply, but I recommend for the HP3457 to use the 5V voltage regulator in TO220 housing and connect the AC lines to pin 1 and 2, which is 12 V.

The overall current consumption @12V supply is low and only about 30mA.

After mounting it looks like this from the internal side (the red supply header is unconnected here):

<img src="https://github.com/xyphro/HP3457-OLED-display/blob/main/photos/DisplayInstalled_backside.jpg?raw=true" width="40%"/>

Enjoy your shiny new Display :-)

# Compatibility with other HP devices

The design might be compatible with other HP devices, but I only tested it with HP3547. I see chances for HP3478, HP3468, but also the HP66xx "tank" power supplies. This is not tested by me (yet). If anybody is looking for adventures and tries this out, please feedback information to me, so that I can mark it is compatible here or share the required SW updates with a wider audience.

