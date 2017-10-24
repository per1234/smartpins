![smartpins Logo](/assets/pins.png)
## ESP8266 Arduino library to manage GPIO pins
# Introduction

**smartPins** manages your input hardware/sensors for you. It removes the need to worry about ISRs, debouncing, background tasks, asychronous events and many other common issues.
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

## In detail / additional functionality

While you can use **smartpins** "out-of-the-box" as in the example above, it helps to understand two concepts:

1) It is an extension of the H4 timer/scheduler library (https://github.com/philbowles/h4) so it provides all the functions of that library in addition to the pin management. These include flexible and adaptable timing functions that can call class methods or `std::function`s when the timer "fires".

2) It introduces the concept or "raw" and "cooked" pin states. The "raw" state is the actual physical state of the pin in as close as possible to real-time. All the managed pins are polled as fast as the processor allows - during testing on a variety of hardware (see list below) running at CPU speed 80MHz this is typically about 40000 times per second, or about every 25us. So while it is not as "instantaneous" as an interrupt, it works for the majority of popular hardware types. This is why the phrase "near real-time" is used here. On the plus side, it removes the need to worry about Interrupt Service Routines (ISR) and the complexities / common errors that they bring.It is important to understand that all of this functionality adds an amount of time to the actual event, so users may notice some "lag" in very rapid systems. The added latency however is extremely small and is the price that one pays for robustness, ease-of-use and absence of WDT resets and/or other common problems. Nothing in life is free...

The "cooked" state has a different meaning for each different pin type.

# "Raw" vs "cooked"

Perhaps the easiest pin type to explain this is the "Latching" pin. A "latch" is something that stays in one state until it is commanded to change to another. Imagine then that you only have a simple and cheap "tactile" or "tact" momentary push button. The first issue is that they "bounce" - usually rather a lot. Imagine also you want one press of the button to turn an LED on and a second (later) press to turn it off.

Forget the debouncing for a second (or about 15ms...a nerd joke) the "raw" state of the pin is on/off for each press. (it may also be off/on if it is "pulled-up" - **smartpins** doesn't care which, it reacts to *changes* in the state). So we have to show it is "latched" after one on/off sequence and then *un*latched after a subsequent on/off sequence. Hence we have four raw inputs, but only two meaningful outputs. The meaningful outputs are the "cooked" states.

Back to the real world: the above is only true with an ideal switch that never bounces. So in reality we may get 10 or 15 on/off transitions for the press followed by 7 or 8 for the release, depending on just how "bouncy" the switch is. **smartpins** irons all of that out and calls you once and only once, providing you with its own internal state "latched" (=1). Next time, it calls you back with "unlatched" (=0).

Referring back to the image of the LED monitor above, the top row of LEDs shows the "raw" state and the bottom row is "cooked". If you had a latching pin defined, the top "raw" LED would flicker on/off (usually at least once, due to the bouncing) as you press and release it, but the bottom "cooked" LED will stay on. Later, when you repeat this, the raw pin will again flicker on/off but now the cooked pin will go off.

A "truth table" for a latching pin would look something like this:

RAW| COOKED
---|---
1| -
...|ignored for N ms
0|1
1|-
...|ignored for N ms
0|0

N is the number of milliseconds debounce time.
... represents any subsequent transitions (i.e. bounces)
For the sake of convenience, positive logic has been chosen but if the pin were pulled up, the table would just have all the 1s and 0s reversed.

## Supported Hardware

**smartpins** has been tested on the following hardware:

* ESP-01
* ESP-01S
* Sonoff Basic
* Sonoff S20
* Sonoff SV
* Wemos D1 mini
* Nodedmuc v0.9

There is no reason why it shouldn't run on any other ESP8266 boards

## Pin types

1. **RAW**

As the name suggests, there are no "cooked" events - the callback runs on every* transition of the pin. Callback parameter is current state: 1 or 0.

RAW| COOKED
---|---
1|-
0|-

2. **DEBOUNCED**

Ignores "bouncy" transitions for a period of N ms. Thus the pin has to stay in the changed state for at least N ms before a cooked event will fire.

Typical usage: Standard input pushbutton or switch, requiring near-instant action.

RAW| COOKED
---|---
1| -
...|ignored for N ms
1|1
0|-
...|ignored for N ms
0|0

3. **POLLED**

Callback fires on a timer, returning instantaneous state: for digital pins this will be 0|1, for A0 it will be the absolute current ADC reading. Transitions in between timer "ticks" will be ignored.

Typical usage: Slow-changing sensors, e.g. daylight sensors, temperature sensors. POLLED pins have an additional parameter specifying an analog read vs the more common digital read. The fact that minor occasional transitions in between ticks are ignored is useful for adding "hysteres" to the pin or "smoothing out" minor wobbles.

RAW| COOKED
---|---
1|-
timer fires|1
0|-
1|-
0|-
...|...
timer fires|0

4. **TIMED**

Based on a debounced pin (for N ms). Callback returns the current state AND the number of milliseconds that pin was held in the changed state

Typical usage: To differentiate between a "short" and a "long" event, e.g. a button press. For example, in the author's home automation devices, a "short" press switches the device on or off. A "medium" press reboots the device and "long" press causes a factory reset.

It is up to the user to define what "short" "medium" and "long" mean, by comparing the return value from the callback and taking the relevant action

RAW| COOKED|VALUE
---|---|---
1|1|1
e.g. 127ms elapses*|...|...
0|0|127
...e.g several minutes elapse|...|...
1|1|1
e.g. 1438ms elapse*|...|...
0|0|1438

* **NB** debouncing complication not shown

5. **REPORTING**

Based on a timed pin (for N ms). Callback returns the current state AND the number of milliseconds that pin was held in the changed state for every amount of "frequency" parameter.

Typical usage: To differentiate between a "short" and a "long" event, but to also get notified periodically while e.g. button is held down, allowing other actions to take place - perhaps to signify button state. In the author's HA system (see TIMED button above) when a button is held down for longer than a "short" period, an LED starts flashing. The longer the button is held, the faster the LED flashes, to signify its changing status to the user.

Example with repoorting frequency of 100ms (1 sec)

RAW| COOKED|VALUE
---|---|---
1|1|1
e.g. 1000ms elapses*|...|...
1|1|1000
...e.g 1000ms elapses|...|...
1|1|2000
e.g. 438ms elapse*|...|...
0|0|2438

* **NB** debouncing complication not shown

6. **LATCHING**

Based on a debounced pin. Turns a momentary button/switch into a latching button/switch

Typical usage: Providing long-term on / off state when only momentary hardware / input devices are available.

RAW| COOKED
---|---
1| -
...|ignored for N ms
0|1
any amount of time elapses...|...
1|-
...|ignored for N ms
0|0

7. **RETRIGGERED**

Based on a debounced pin. After first change, will call back only after [timeout] ms have elapsed. If subsequent "triggering" transitions occur before [timeout] has expired, then the internal timer is restarted. Thus the "untriggered" state can only ever occur <timeout> ms after the last (or only) inital trigger event. It also has an additional hysteresis timer (which can be zero) - this is the "deadzone" time between triggers, i.e. the pin will not react again after the previous "untrigger" until [hysteresis] ms have elapsed. 

Typical usage: PIR sensors. Although a typical PIR sensor will have these funtions built into the hardware, they are often physically inaccessible in IOT systems. A retriggering pin allows the PIR to function while being controlled / adjusted (possibly by e.g. MQTT) in software. If such an application is chosen, then the hardware should be set to NON-retriggering and timeout to the minimum value - at least less than the value chosen for the <timeout> parameter.
  
Notes:
A "truth table" is not additonally informative in this case. Note that unlike all the other pins types, a retriggering pin needs to know the difference between "on" and "off", "triggered" and "quiet". This is die to the fact that the device may indeed be in the "triggered" state at boot time (especially if that is performed manually). Since **smartpins** reacts to *changes* in state, if a PIR is already triggered at boot time, and **smartpins** is not aware, then all subsequent events will be interpreted "the wrong way round" and the timing sequence will be reversed. Thus an additional input parameter is required HIGH or LOW. A retriggered pin will ignore the first transition that is oposite in sense to this parameter. i.e. if the PIR is positive logic, provide a HIGH parameter...if it is already triggered HIGH at boot time, the 1st transiton will be to LOW and thus ignored.

Positive logic timeline: [debounce]=15ms [timeout]=30000ms (30sec) [hysteresis]=2500ms (2.5sec)

T=0|T=5|T=8|T=10|T=12|T=15|...|T=500|T=1500|T=31500|...|ignored until T=34000
---|---|---|----|----|----|---|-----|------|-------|---|---
RAW|  1|  0|   1|   0|   1|111|    0|     1|      -|...|-
COOKED|  -|  -|   -|   -|   1|---|    -|     -|      0|...|-
 B | O | U |  N |  C |  E |T start|     |T retrig   |T expire    |Dead|Alive
   

## Getting Started

# Prerequisites

**smartPins** relies upon the H4 timer/scheduler library, which must be installed first:

* https://github.com/philbowles/h4

Numerous tutorials exists explaing how to intall libraries into your Arduino IDE. Here are a couple of examples:

* https://www.baldengineer.com/installing-arduino-library-from-github.html
* http://skaarhoj.com/wiki/index.php/Steps_to_install_an_Arduino_Library_from_GitHub

## API reference

N.B. You must call the smartPins `loop()` function from within the main loop of your program as often as possible



(C) 2017 **Phil Bowles**
philbowles2012@gmail.com
