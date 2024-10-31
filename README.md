# whack-a-mole-attack

Project files for a two-player whack-a-mole / button mash game

Original idea by https://github.com/DIYTechBros/ButtonSmack / https://www.youtube.com/watch?v=FXypfZi_UDI

[![Watch the video](https://img.youtube.com/vi/DrATmFxHTjc/hqdefault.jpg)](https://www.youtube.com/embed/DrATmFxHTjc)


Use a Teensy 4 to control the game, sending serial messages to a Matrix Portal for driving the graphics on a 32x32 LED Panel.

# Components

- [Teensy 4.1](https://www.pjrc.com/store/teensy41.html)
- [32x32 Adafruit LED Matrix](https://www.adafruit.com/product/2026)
- [Adafruit Matrix Portal S3](https://www.adafruit.com/product/5778)
- XL4015 5A DC Buck Stepdown converter (EBay)
- Batteries (I used 2 21700 batteries in series, but a high-current Lipo should work too)
- 10 60mm dome LED push-buttons
- 2 arcade buttons (player start)

# Parts

- 73x100mm PCB strip board
- spade terminals + quick connectors
- wire
- connectors
- 10K resistors
- M/F barrel connector
- 2x4s
- 1/2 inch finished plywood
- a few 3D printed pieces

# Code

The Teensy is programmed with the Arduino IDE. `WhackAMole.ino` just sets a few things up and then calls `Game::Update()` in the main loop.

`Game.cpp` defines the game states and state transitions, called Update on the players, and sends the Serial messages to the Matrix Portal for each state to drive the display.

`Player.cpp` defines the Player objects and their internal attributes needed to run the game. These attributes include player score, current button and points, input pins, button blink state, button debounce, etc.

`code.py` the CircuitPython code that runs on the Matrix Portal. Listen on the serial port and change the display based on the messages received. This is kinda gross code.

# What worked well

- The Teensy is an awesome little computer. Easy to code, super fast, really stable, and overall just worked all the time
- Using switches in series with 10K resistors in between is an awesome way to read in multiple distinct user inputs while only using one analog pin
- The serial communication between the teensy and the Portal Matrix worked better than I expected and was more than fast enough
- The physical dome buttons held up well to an hour of constant abuse from a whole pack of Cub Scouts
- The LED Matrix itself was nice and bright and refreshed quickly
- The 21700 batteries and buck converter worked great to power the whole thing

# What I would do different

- The LEDs on the dome buttons are not great, just not bright. The DIY TechBros design replaced the built-in resistor to improve brightness, but it didn't make much of an improvement for me.
- The wiring scheme I came up with worked but it was ugly. If I were going to rewire this, I'd use a 4-pin JST connector and ribbon cable per LED button. The inline resistors would need to be moved to the perf board.

# Improvements

- SOUNDS! There is an audio break-out board for the Teensy, adding some sounds would really make the game have an arcade feel
- Better button LEDs. I may try to cram a few WS2812B LEDs into the Dome button housing

# Circuit schematic

TODO

# Table design

I started from the playfield (16x36 inches) and then just built a table from and legs underneath that. I needed to 3d print a few spacer/bushings so that the buttons were snug in the holes I cut (I didn't have a perfectly-sized hole saw). I also 3d printed 21700 battery holders. The battery holders and perf-board are just screwed to the underside with wood screws. The PortalMatrix is piggy-backed onto the back of the LED panel on the 2x10 socket connector. The LED panel is inset into the middle of the playfield; I added supports underneath to keep it flush with the playfield surface. I also added a 3d-printed bezel around the LED panel in an attempt to make it look like a mole hill and to discourage players from bonking it.

