# Anamorphic-Lens-Slide
This code is for an Arduino that is connected to a stepper motor. The Arduino uses inputs from an IR sensor and two limit switches to determine how to move the slide.

Parts Used
- 400mm Travel Length Linear Stage Actuator (Part number Sfu1605). This comes with a 3.0A 1.8deg 24V stepper motor. I'm sure you could also build one using some extruded aluminium and an off-the-shelf motor.
- Arduino Uno. I bought the super starter kit with the various extra little parts, like the IR sensor and remote. I highly recommend getting a kit if you don't already have all the stuff for this.
- TB6600 stepper motor driver. There are a lot of these available from manufacturers in China.
- Meanwell 24V 6.0A power supply. This is for powering the stepper motor through the motor driver. I imagine many others will work fine as long as it has enough power for your particular stepper motor. I found mine for a pretty good price on eBay. You'll need to strip a power cord to hook it up to the connections on the PSU. Just use whatever you have around
- Limit switches. Tons of options for these. Just get a bag of them from Amazon or something.
- LM2596S DC-DC convertor. This steps down the power from the PSU to something appropriate for the Arduino
- Sensor shield v5. Not absolutely necessarily, but makes wiring much easier and cleaner.

Here are some things to keep in mind before adapting this to your setup.
- In my setup left moves it front of the projector and right moves it away from the projector.
- The IR remote codes are hard coded in the program. These would need to change based on whatever remote you use. You will likely need to use an IR code capture program of some kind to figure out what the codes are if you're using something different. There are many examples for this online. I used the remote that came with my kit and simply added the codes to my projector in my Harmony hub setup.
- The pause between motor pulses may differ based on your choice of motor. If you're having problems, this may be causing it.
- Different stepper motors may have different pin setups or power needs, so the TB6600 may not work for certain motors.
- I used a three-prong IR sensor that came with my kit. I don't know if different sensors have different pins or communicate differently.
- My limit switches were wired as normally open (NO). You will need to reverse the states in the program if you wire them as normally closed (NC).

Here are some hurdles I ran into while working on this.
- On many TB6600 drivers the A and B pins are swapped. Before plugging everything in open up your driver and confirm the pins are correct.
- For some reason running the serial print out causes some weird behavior with the driver and motor. It almost seems like it's changing the delay between pauses somehow. If you want to use the serial output, just disconnect the power from the driver/motor.
- If I set my motor enable pin to high while the motor isn't moving, the motor makes a slight buzzing noise. Turning the pin off when not moving fixes this.
- The limit switches need some somewhat special wiring. The common pin needs to be wired to both the communication pin on the Arduino and a 5V source. You also need to put a resistor in between this pin and the 5V. 5Kohm worked for me. The NO or NC pin needs to be wired to ground.
