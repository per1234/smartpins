![H4 Logo](/assets/pins.png)
## ESP8266 Arduino library to manage input/output HW pins
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
- "Latching" pins - allow you to turn a momentary "tact" button into something more like a toggle switch. pres and release once: it's - "on", Do it again, it's "off"
- "Retriggering" pins - similar to a PIR sensor: calls back once when triggered and the again once a certain time has elapsed. Any additional event in between restarts the timer
- "Encoder" pin pairs - Attach a rotary encoder to two pins and you get called when it clicks in either direction
- "Auto Encoder" - returns an absolute value when the position changes, e.g. like an old-fashioned volume control

# In detail / additional functionality

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
