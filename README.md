![smartpins Logo](/assets/pins.png)
## ESP8266 Arduino library to manage GPIO pins
# Introduction

smartPins manages your input hardware/sensors for you. It removes the need to worry about ISRs, debouncing, background tasks, asychronous events and many other common issues.
Put simply, smartPins takes care of pretty much anything you might want to do with an input pin and calls you back when something interesting happens. Its main purpose is to allow safe and robust hardware events in an asynchronous programming environment as is the ESP8266. That's a bit of a technical mouthful, it's much easier to understand with a simple code snippet. Imagine you have a very "bouncy" switch on pin 4. The following code is all you need to act upon switch events (cleanly) while also co-operating with ESP8266 background tasks:

```c++
#include <SmartPins.h>

SmartPins SP;
...
void fState(bool b){
  Serial.printf("BUTTON %s\n",b ? "UP":"DOWN",b);
}
...
void setup() {
  Serial.begin(74880);
  SP.Debounced(4,INPUT_PULLUP,10,fState); // 10=ms "debounce time"
}
...

void loop() {
  SP.loop();
}
```
It's about much more than just debouncing, it has numerous different input styles and - more importanly it puts all asynchronous input events into a single queue. This makes sure that all your code runs from the main loop thread, which reduces / prevents a number of common problems.

The additional handlers for the variety of things you can do with a hardware pin:

- "Raw" pins - which just callback whenever they change, without any filtering or other processing
- "Debounced" pins - as you see in the sample above, handy for "noisy" mechanical switches.
- "Polled" pins - return the value of the pin at regular intervals (ideal for reading the ADC for e.g. a temperature sensor)
- "Timed" pins - return with a value of how many milliseconds they were held down for
- "Reporting" pins - as for "Timed", but callback repeatedly while the pin is held down.
- "Latching" pins - allow you to turn a momentary "tact" button into something more like a toggle switch: press/release once: it's - "on"; Do it again, it's "off"
- "Retriggering" pins - similar to a PIR sensor: calls back once when triggered and the again once a certain time has elapsed. Any additional event in between restarts the timer
- "Encoder" pin pairs - Attach a rotary encoder to two pins and you get called when it clicks in either direction
- "Auto Encoder" - returns an absolute value when the position changes, e.g. like an old-fashioned volume control

It also includes "hooks" which allow the user to do things like this, monitoring the state of the pins in (near) real-time:

![esparto](/assets/esparto.png)

# In detail / additional functionality

While you can use **smartpins** "out-of-the-box" as in the example above, it helps to understand two concepts:

1) It is an extension of the H4 timer/scheduler library (https://github.com/philbowles/h4) so it provides all the functions of that library in addition to the pin management. These include fleibla and adaptable timing functions that can call class methods or std:;functions when they "fire"

2) It introduces the concept or "raw" and "cooked" pin states. The "raw" state is the actual physical state of the pin in a close as possible to real-time. All the managed pins are polled as fast as the processor allows - during testing on a variety of hardware (see list below) running at CPU speed 80MHz this is typically about 40000 times per second, or about every 25us. So while it is not as "instantaneous" as an interrupt, it works for the majority of popular hardware types. This is why the phrase "near real-time" is used here. On the plus side, it removes the need to worry about Interrupt Service Routines (ISR) and the complexities / common errors that they bring.

The "cooked" state has a different meaning for each different pin type

# "Raw" vs "cooked"

Perhaps the easiest pin type to explain this is the "Latching" pin. A "latch" is something that stays in one state until it is commanded to change to another. Imagine then that you only have a simple and cheap "tactile" or "tact" momentary push button. The first issue is that they "bounce" - usually rather a lot. Imagine also you want one press of the button to turn an LED on and a second (later) press to turn it off.

Forget the debouncing for a second (or about 15ms...a nerd joke) the "raw" state of the pin is on/off for each press. (it may also be off/on if it is "pulled-up" - **smartpins** doesn't care which, it reacts to *changes* in the state). So we have to show it is "latched" after one on/off sequence and then *un*latched after a subsequent on/off sequence. Hence we have four raw inputs, but only two meaningful outputs. The meaningful outputs are the "cooked" states.

Back to the real world: the above is only true with an ideal switch that never bounces. So in reality we may get 10 or 15 on/off transitions for the press followed by 7 or 8 for the release, depending on just how "bouncy" the switch is. **smartpins** irons all of that out and calls you once and only once, providing you with its own internal state "latched" (=1). Next time, it calls you back with "unlatched" (=0).

Referring back to the image of the LED monitor above, the top row of LEDs shows the "raw" state and the bottom row is "cooked". If you had a latching pin defined, the top "raw" LED would flicker on/off (usually at least once, due to the bouncing) as you press and release it, but the bottom "cooked" LED will stay on. Later, when you repeat this, the raw pin will again flicker on/off but now the cooked pin will go off.

RAW COOKED
1
0 1
1
0 0


# Getting Started

## Prerequisites

**smartPins** relies upon the H4 timer/scheduler library, which must be installed first:

* https://github.com/philbowles/h4

Numerous tutorials exists explaing how to intall libraries into your Arduino IDE. Here are a couple of examples:

* https://www.baldengineer.com/installing-arduino-library-from-github.html
* http://skaarhoj.com/wiki/index.php/Steps_to_install_an_Arduino_Library_from_GitHub

# API reference

N.B. You must call the smartPins `loop()` function from within the main loop of your program as often as possible



(C) 2017 **Phil Bowles**
philbowles2012@gmail.com
