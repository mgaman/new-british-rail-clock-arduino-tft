# British Railways new style clock
In Autumn 2025 the management of British Rail proudly announced the introduction of a [new style clock](https://www.networkrail.co.uk/stories/a-new-timepiece-for-the-railway/).<p>
Reportedly this was achieved at a bargain price of £120,000.<br>
## My Version
Feeling the need for a similar clock, but lacking the necessary funds, I made my own for an estimated cost of less then $5. Instead of the BR 1.8m diameter display I used a 1.28" TFT display and an ESP32 development board for the code.<br>
You can see it [here](https://youtube.com/shorts/Q88IBHg2Aqc). While my displays 240x240 pixels is considerably less than the BR screen it lacks the crisp resolution of the big clock but retains the overall effect.<br>
## Methodology
I wrote the code as an Arduino project in the **VSCode** and **PlatformIO** framework, using the [TFT_eSPi](https://github.com/Bodmer/TFT_eSPI/) library to control the screen. It proved to be a valuable excersize in learning TFT_eSPI and how to use sprites. I hope my code is documented well enough for
others to learn from it.<br>
My project was setup for a 240x240 circular TFT screen using a GC9A01 chip. The choice of TFT is irrelevant to the code of the clock.
**Caveat** The time displayed is entirely fake. It is simple a counter initialized to an arbitary value and incremented by the ESP32 internal crystal.
## Sprites
The heart of the code is the management of 4 sprites. The two arrow sprites only get drawn once as they never get over-drawn. The time spite needs to be redrawn every time the time changes (every minute). The base sprite needs to be redrawn every clock tick as it has been over-drawn by the other sprites.
## Timer
The basic cycle of the clock is 1 minute. The two arrows complete a circuit of the clock which is a distance of 240 pixels up & down i.e 480 pixels in 60 seconds. The clock is therefore programmed to tick every 60M/480 microsecs.<br>

