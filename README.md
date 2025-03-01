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
I bought my displays directly on Aliexpress: https://de.aliexpress.com/item/1005001856921229.html
In case the above link stops working, search for items like "XABL 0.87 Inch OLED"

I personally tested SSD1336 based display. EEVBlog User Miti bought a display with a CH1115 controller. This controller has a different command set. We jointly have added support for CH1115 displays to the code. In case you have such a SSD1336 you will need to rebuild the software with the `#define DISPLAYTYPE_CH1115` being enabled within oled.c. Otherwise the original SSD1336 based OLED displays will be supported.

The prebuilt binaries provided in this repository are built for SSD1336 based displays.


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

<img src="https://github.com/xyphro/HP3457-OLED-display/blob/main/photos/PowerConnection.jpg?raw=true" width="40%"/>

The overall current consumption @12V supply is low and only about 30mA.

After mounting it looks like this from the internal side (the red supply header is unconnected here):

<img src="https://github.com/xyphro/HP3457-OLED-display/blob/main/photos/DisplayInstalled_backside.jpg?raw=true" width="40%"/>

Enjoy your shiny new Display :-)

# Update 14th February 2025

## Experience after 3+ years

After 3 years of continuous use the Display still works very well. I have the feeling that the OLEDs got a tiny bit dimmer, but it is still tremendously better as the original LC-Display.

## Command coverage

Before this update (14th February 2025) only commands sent by HP3457a were supported. Though new commands were discovered, which triggered more searching for commands. 
I could find a few more and implemented it. See below table for an overview.

| Command code (12 bit HEX) | Command mnemonic | Implementation status / comment|
|--------------------|----------------------|----------------------|
|028 |	WRA12L   | Tested myself on HP3457a. |
|068 |	WRB12L   | Tested myself on HP3457a. |
|0a8 |	WRC12L   | Tested myself on HP3457a. |
|0e8 |	WRAB6L   | Tested with playback of eevblog user Kawal's logic analyzer recording.|
|128 |	WRABC4L  | Not tested with HP instrument, have sent the command manually.|
|168 |	WRAB6R   | Not tested with HP instrument, have sent the command manually.|
|1a8 |	WRABC4R  | Not tested with HP instrument, have sent the command manually.|
|1e8 |	WRA1L    | Not tested with HP instrument, have sent the command manually.|
|228 |	WRB1L    | Not tested with HP instrument, have sent the command manually.|
|268 |	WRC1L    | Not tested with HP instrument, have sent the command manually.|
|2a8 |	WRA1R    | Not tested with HP instrument, have sent the command manually.|
|2e8 |	WRB1R    | Not tested with HP instrument, have sent the command manually.|
|328 |	WRC1R    | Not tested with HP instrument, have sent the command manually.|
|368 |	WRAB1R   | Not tested with HP instrument, have sent the command manually.|
|3a8 |	WRABC1L  | Not tested with HP instrument, have sent the command manually.|
|3e8 |	WRABC1R  | Not tested with HP instrument, have sent the command manually.|
|230 |  DISOFF   | Implemented, but disabled. It results in wrong behavior on HP3457a. This command is supposed to turn the display OFF. It never gets sent on HP3457A.|
|320 |  DISTOG   | Implemented, but disabled. It results in wrong behavior on HP3457a. This command is supposed to turn the display OFF. It gets sent on HP3457A, BUT turns the display continously on and off. Before this command always command code 0x2E0 is sent and it is unknown what this command does.|
|3FC |  DISCMP   | Not implemented, because it is unknown what this command is supposed to do. It does not get sent by HP3457a.|
|3F0 | PERSCLT | Tested myself on HP3457a. Note that in previous FW versions it was not enabled.|
|2F0 | WRITAN  | Tested myself on HP3457a. Updates the annuciators.|
|2E0 | ??????  | Unknown command. It gets sent by HP3457a every time before the DISTOG command is sent.|


In case you have ANY information about the unknown HP LCD display commands let me know please. Or if you have a FW image of an original LCD display for analysis :-)

# Further links / Information

An eevblog thread on this project exists with lots of other users HW variant, experience and updates. E.g. HW variants can be found without isolator IC and using LDOs.

Have a look here: <a href="https://www.eevblog.com/forum/testgear/hp3548-hp3457-oled-display/" target="_blank">https://www.eevblog.com/forum/testgear/hp3548-hp3457-oled-display/</a>

In case you build the HW in this project note that it makes use of the 5V supply on the Display connector to supply the isolator IC. HP seems to put series resistors on the main PCB to limit the display IC current. You will very likely need to bridge these resistors - or build one of the eevblog variants with Isolator IC.

# Compatibility with other HP devices

The design might be compatible with other HP devices, but I personally only tested it with HP3547a. 
It is in the meantime confirmed, that the display works on HP3548 HP3478a and HP3852A too.

Chances are there that it might work for HP3468 and the HP66xx "tank" power supplies too. This is not tested by me (yet). If anybody is looking for adventures and tries this out, please feedback information to me, so that I can mark it is compatible here or share the required SW updates with a wider audience.
