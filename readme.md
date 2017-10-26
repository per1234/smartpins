![smartpins Logo](/assets/pins.png)
# SmartPins - an ESP8266 Arduino library to manage GPIO pins
## Introduction

**SmartPins** manages your input hardware/sensors for you. It removes the need to worry about ISRs, debouncing, background tasks, asychronous events and many other common issues.
Put simply, smartPins takes care of pretty much anything you might want to do with an input pin and calls you back when something interesting happens. Its main purpose is to allow safe and robust hardware events in the asynchronous programming environment that is the ESP8266. That's a bit of a technical mouthful, it's much easier to understand with a simple code snippet. Imagine you have a very "bouncy" switch on pin 4. The following code is all you need to act upon switch events (cleanly) while also co-operating with ESP8266 background tasks:

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
# Supported Hardware

**SmartPins** has been tested on the following hardware:

* ESP-01
* ESP-01S
* Sonoff Basic
* Sonoff S20
* Sonoff SV
* Wemos D1 mini
* Nodedmuc v0.9

There is no reason why it shouldn't run on any other ESP8266 boards

# In detail / additional functionality

**SmartPins** provides much more than just debouncing: it has numerous different input styles and - more importanly it puts all asynchronous input events into a single queue. This makes sure that all your code runs from the main loop thread, which reduces / prevents a number of common problems.

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

It also includes "hooks" which allow the user to do things like this, monitoring the state of the pins in (near) real-time. This graphic display example is taken from a device running the soon-to-be-released Esparto v2.0 (an all-in-one ESP8266 MQTT handler - *very basic "proof-of-concept" version available here, without web interface* https://github.com/philbowles/esparto )

![esparto](/assets/esparto.png)

While you can use **SmartPins** "out-of-the-box" as in the example above, it helps to understand two concepts:

1) It is an extension of the H4 timer/scheduler library (https://github.com/philbowles/h4) so it provides all the functions of that library in addition to the pin management. These include flexible and adaptable timing functions that can call class methods or `std::function`s (even "normal" functions too!) when the timer "fires".

2) It introduces the concept or "raw" and "cooked" pin states. The "raw" state is the actual physical state of the pin in as close as possible to real-time. All the managed pins are polled as fast as the processor (and your own main loop) allows - during testing on a variety of hardware running at CPU speed 80MHz this is typically about 40000 times per second, or about every 25us. So while it is not as "instantaneous" as an interrupt, it works for the majority of popular input devices. This is why the phrase "near real-time" is used here. On the plus side, it removes the need to worry about Interrupt Service Routines (ISRs) and the complexities / common errors that they bring. It is important to understand that all of this functionality adds an amount of time to the actual event, so users may notice some "lag" in very rapid systems. The added latency however is extremely small and is the price that one pays for robustness, ease-of-use and absence of WDT resets and/or other common problems. Nothing in life is free...

The "cooked" state has a different meaning for each different pin type.

## "Raw" vs "cooked"

Perhaps the easiest pin type to explain this is the "Latching" pin. A "latch" is something that stays in one state until it is commanded to change to another. Imagine then that you only have a simple and cheap "tactile" or "tact" momentary push button. The first issue is that they "bounce" - usually rather a lot. Imagine also you want one press of the button to turn an LED on and a second (later) press to turn it off.

Forget the debouncing for a second (or usually about 15ms - it's a nerd joke...) the "raw" state of the pin is on/off for each press. (it may also be off/on if it is "pulled-up" - **SmartPins** doesn't care which, it reacts to *changes* in the pin state). So we have to show it is "latched" after one on/off sequence and then *un*latched after a subsequent on/off sequence. Hence we have four raw inputs, but only two "meaningful" outputs. The meaningful outputs are the "cooked" states.

Back to the real world: the above is only true with an ideal switch that never bounces. So in reality we may get 10 or 15 on/off transitions for the press followed by 7 or 8 for the release, depending on just how "bouncy" the switch is. **SmartPins** irons all of that out and calls you once and only once, providing you with its own internal state "latched" (=1). Next time, it calls you back with "unlatched" (=0).

Referring back to the image of the LED monitor above, the top row of LEDs shows the "raw" state and the bottom row is "cooked". If you had a latching pin defined, the top "raw" LED would flicker on/off (usually at least once, due to the bouncing) as you press and release it, but the bottom "cooked" LED will stay on. Later, when you repeat this, the raw pin will again flicker on/off but now the cooked pin will go off.

A "state table" for a latching pin would look something like this:

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

# Pin types

## 1. **RAW**

As the name suggests, there are no "cooked" events - the callback runs on every* transition of the pin. Callback parameter is current state: 1 or 0. [* providing they occur within thw 25us "window". **SmartPins** will not funtion correctly with devices which require a fatser reaction time than this]

RAW| COOKED
---|---
1|-
0|-

### Example [Raw](../master/examples/00_Raw/00_Raw.ino)

## 2. **DEBOUNCED**

Ignores "bouncy" transitions for a period of N ms. Thus the pin has to stay in the changed state for at least N ms before a cooked event will fire.

Typical usage: Standard debounced input pushbutton or switch, requiring near-instant action.

RAW| COOKED
---|---
1| -
...|ignored for N ms
1|1
0|-
...|ignored for N ms
0|0

### Example [Debounced](../master/examples/01_Debounced/01_Debounced.ino)

## 3. **POLLED**

Callback fires on a timer, returning instantaneous state: for digital pins this will be 0|1, for A0 it will be the absolute current ADC reading. Transitions in between timer "ticks" will be ignored.

Typical usage: Slow-changing sensors, e.g. daylight sensors, temperature sensors. POLLED pins have an additional parameter specifying an analog read vs the more common digital read. The fact that minor occasional transitions in between ticks are ignored is useful for adding "hysteresis" to the pin or "smoothing out" minor wobbles.

RAW| COOKED
---|---
1|-
timer fires|1
0|-
1|-
0|-
...|...
timer fires|0

### Example [Polled](../master/examples/02_Polled/02_Polled.ino)

## 4. **TIMED**

Based on a debounced pin (for N ms). Callback returns the current state AND the number of milliseconds that pin was held in the changed state.

Typical usage: To differentiate between a "short" and a "long" event, e.g. a button press. For example, in the author's home automation devices, a "short" press switches the device on or off. A "medium" press reboots the device and "long" press causes a factory reset.

It is up to the user to define what "short" "medium" and "long" mean, by comparing the return value from the callback and taking the relevant action.

RAW| COOKED|VALUE
---|---|---
1|1|1
e.g. 127ms elapses*|...|...
0|0|127
...e.g several minutes elapse|...|...
1|1|1
e.g. 1438ms elapse*|...|...
0|0|1438

*debouncing complication not shown

### Example * [Timed](../master/examples/03_Timed/03_Timed.ino)

## 5. **REPORTING**

Based on a timed pin (for N ms). Callback returns the current state AND the number of milliseconds that pin was held in the changed state for every amount of "frequency" parameter.

Typical usage: To differentiate between a "short" and a "long" event, but to also get notified periodically while e.g. button is held down, allowing other actions to take place - perhaps to signify button state. In the author's HA system (see TIMED button above) when a button is held down for longer than a "short" period, an LED starts flashing. The longer the button is held, the faster the LED flashes, to signify its changing status to the user.

Example with reporting frequency of 1000ms (1 sec)

RAW| COOKED|VALUE
---|---|---
1|1|1
e.g. 1000ms elapses*|...|...
1|1|1000
...e.g 1000ms elapses|...|...
1|1|2000
e.g. 438ms elapse*|...|...
0|0|2438

*debouncing complication not shown

### Example [Reporting](../master/examples/04_Reporting/04_Reporting.ino)

## 6. **LATCHING**

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

### Example [Latching](../master/examples/05_Latching/05_Latching.ino)

## 7. **RETRIGGERING**

Based on a debounced pin. After first change, will call back only after [timeout] ms have elapsed. If subsequent "triggering" transitions occur before [timeout] has expired, then the internal timer is restarted. Thus the "untriggered" state can only ever occur [timeout] ms after the last (or only) inital trigger event. It also has an additional hysteresis timer (which can be zero) - this is the "deadzone" time between triggers, i.e. the pin will not react again after the previous "untrigger" until [hysteresis] ms have elapsed. 

Typical usage: PIR sensors. Although a typical PIR sensor will have these funtions built into the hardware, they are often physically inaccessible in IOT systems. A retriggering pin allows the PIR to function while being controlled / adjusted (possibly by e.g. MQTT) in software. If such an application is chosen, then the hardware should be set to NON-retriggering and the *hardware* timeout set to its minimum value - it should be less than the value chosen for the [timeout] parameter.
  
Notes:
Unlike all the other pins types, a retriggering pin needs to know the difference between "on" and "off", "triggered" and "quiet". This is due to the fact that the device may indeed be in the "triggered" state at boot time (especially if that is performed manually). Since **SmartPins** reacts to *changes* in state, if a PIR is already triggered at boot time, and **SmartPins** is not aware, then all subsequent events will be interpreted "the wrong way round" and the timing sequence will be reversed. Thus an additional input parameter is required HIGH or LOW. A retriggering pin will ignore the first transition that is opposite in sense to this parameter. i.e. if the PIR is positive logic, provide a HIGH parameter...then if it is already triggered HIGH at boot time, the 1st transiton will be to LOW and thus ignored.

Positive logic timeline: [debounce]=15ms [timeout]=30000ms (30sec) [hysteresis]=2500ms (2.5sec)

T=0|T=5|T=8|T=10|T=12|T=15|...|T=500|T=1500|T=31500|H=2500|T=34000
---|---|---|----|----|----|---|-----|------|-------|---|---
RAW|  1|  0|   1|   0|   1|111|    0|     1|      -|ignored|-
COOKED|  -|  -|   -|   -|   1|---|    -|     -|      0|ignored|-
 B | O | U |  N |  C |  E |TRIG|     |RETRIG|EXPIRE|Dead|Active
 
 ### Example [Retriggering](../master/examples/06_Retriggering/06_Retriggering.ino)
 
 ## 8. **ENCODER**
 
Manages a standard rotary encoder. It will of course require *two* pins. For types that also include a push switch, a third (probably a debounced type) may also be added separately. Decodes dual input signals and calls back with +1 or -1 when a clockwise or anticlockwise "click" occurs. If this is the "wrong" sense for your application, simply reverese the two pin definitions.

### Example [Encoder](../master/examples/07_Encoder/07_Encoder.ino)

## 9. **ENCODERAUTO**

Based on an encoder, callback returns an absolute value representing its position. You provide four additional parameters Vmin, Vmax, Vset and Vinc. Vmin is the minimum value the pin(s) will return, Vmax  the maximum and Vset is where you require the "current" position to be. Obviously, at all times, Vset must be between Vmin and Vmax. Vinc is the amount by which the encoder increments (or decrements) with each click. Defaults are Vmin=0, Vmax=100, Vset=50, i.e. the mid-point and Vinc=1. One click anticlockwise will callback with 49, 1 click clockwise will callback with 51. The returned value will  never exceed Vmin or Vmax: if the encoder is turned "past" these points, it will simply not return a value.

Vmin, Vmax may be -ve. The encoder will function as long as the numerical condition Vmin < Vset < Vmax holds true. As an example, imagine an encoderauto pin-pair with Vmin=-273 Vmax=0 Vset=-100 and Vinc=10. One click anticlockwise will callback with -110 and one click clockwise will callback with -90. If the clockwise rotation is continued, callbacks will occur with -80, -70, -60 etc until zero is reached, when no further callbacks will occur until it is turned in the opposite direction.

### Example [EncoderAuto](../master/examples/08_EncoderAuto/08_EncoderAuto.ino)

# Getting Started

## Prerequisites

**SmartPins** relies upon the H4 timer/scheduler library, which must be installed first:

* https://github.com/philbowles/h4

Numerous tutorials exists explaing how to intall libraries into your Arduino IDE. Here are a couple of examples:

* https://www.baldengineer.com/installing-arduino-library-from-github.html
* http://skaarhoj.com/wiki/index.php/Steps_to_install_an_Arduino_Library_from_GitHub

# API reference

**N.B.** You must call the smartPins `loop()` function from within the main loop of your program as often as possible. This runs the underlying H4 scheduler. Without this call, nothing will happen! Also, if your own loop or other code causes long delays, the pin timings will suffer. **SmartPins** should ideally be deployed in applications that use only itself and the timer callbacks to run all code, avoiding any use of `delay()` calls. When used correctly, **SmartPins** removes the need to ever call `delay()`. Avoid it like the plague.

You do **NOT** need to call the Arduino pinMode() function before any of the *input* pin functions. **SmartPins** does this for you already.

## Inherited from H4:

H4_STD_FN is shorthand for `std::function<void(void)>`

```c++
H4_TIMER	every(uint32_t msec,H4_STD_FN fn);
H4_TIMER	everyRandom(uint32_t Rmin,uint32_t Rmax,H4_STD_FN fn);
void 		never();
void 		never(H4_TIMER uid);
H4_TIMER 	nTimes(uint32_t n,uint32_t msec,H4_STD_FN fn,H4_STD_FN chain=nullptr);
H4_TIMER 	nTimesRandom(uint32_t n,uint32_t msec,uint32_t Rmax,H4_STD_FN fn,H4_STD_FN chain=nullptr);
H4_TIMER 	once(uint32_t msec,H4_STD_FN fn,H4_STD_FN chain=nullptr);
H4_TIMER 	onceRandom(uint32_t Rmin,uint32_t Rmax,H4_STD_FN fn,H4_STD_FN chain=nullptr);
void	 	runNow(H4_STD_FN fn);
```
All of the above may be called as methods of a **SmartPins** object, i.e. you do not have to include a separate H4 object or include the H4 library - see example sketches.

**N.B.** See https://github.com/philbowles/h4 for full description of timer / scheduler calls

## Directly from SmartPins
```c++

SMARTPIN_STATE is defined as function<void(int)>
SMARTPIN_STATE_VALUE os defined as function<void(int,int)>

Constructor:

SmartPins(SMARTPIN_STATE_VALUE _cookedHook=nullptr, SMARTPIN_STATE_VALUE _rawHook=nullptr);
    
General Purpose:

int	  getValue(uint8_t _p);
void 	loop();
bool 	reconfigurePin(uint8_t _p,uint32_t v1, uint32_t v2=0);
void 	pulsePin(uint8_t pin,unsigned int ms);
    
Pin Methods:

void Debounced(uint8_t _p,uint8_t _mode,uint32_t _debounce,SMARTPIN_STATE _callback);	
void Encoder(uint8_t _pA,uint8_t _pB,uint8_t mode,SMARTPIN_STATE _callback);	
SmartPins::spEncoderAuto* EncoderAuto(uint8_t _pin,uint8_t _pinB,uint8_t _mode,SMARTPIN_STATE _callback,int _Vmin=0,int _Vmax=100,int _Vinc=1,int _Vset=0);	
void Latching(uint8_t _p,uint8_t _mode,uint32_t _debounce,SMARTPIN_STATE _callback);	
void Polled(uint8_t _p,uint8_t _mode,uint32_t freq,SMARTPIN_STATE _callback,bool adc=false);
void Raw(uint8_t _p,uint8_t _mode,SMARTPIN_STATE _callback);
void Reporting(uint8_t _p,uint8_t mode,uint32_t _debounce,uint32_t _freq,SMARTPIN_STATE_VALUE _callback);	
void Retriggering(uint8_t _p,uint8_t _mode,uint32_t _timeout,SMARTPIN_STATE _callback,uint32_t active=HIGH,uint32_t hyst=3000);	
void Timed(uint8_t _p,uint8_t mode,uint32_t _debounce,SMARTPIN_STATE_VALUE _callback);

EncoderAuto-only methods:

void center();
int getValue();
void reconfigure(int _Vmin,int _Vmax,int _Vinc,int _Vset=0);
void setMin();
void setMax();
void setPercent(uint32_t pc);
void setValue(int v);

```

**Constructor**

`SmartPins(SMARTPIN_STATE_VALUE cookedHook=nullptr, SMARTPIN_STATE_VALUE rawHook=nullptr);`

Both parameters are optional. If either function is provided it will be called on each and every pin transition. The user must take great care to minimise the time spent in these functions or they could add greatly to the "lag" between the true event occuring and the user callback being executed.

In either case, the function must be void(int,int) the first parameter being the pin number and the second being its value. For rawHook this will only ever be 0 or 1. For the cooked hook this will be whatever type of value the pin type returns, e.g. +1/-1 for an encoder pin-pair, a millisecond value for timed pin, 0/1 for a latching pin etc.

**getValue** `int	 getValue(uint8_t p)`

Returns the current value of pin p. See **Constructor** for the range of values returned

**loop** `void loop()`

Runs the underlying scheduler. Must be called from the main loop as often as possible.

**reconfigurePin** `void reconfigurePin(uint8_t p,uint32_t v1, uint32_t v2=0)`

Changes the values associated with an already configured pin p. v1 and v2 have different meanings for each pin type as follows:

Pin Type|v1|v2
---|---|---
raw|-|-
debounced|debounce time|-
polled|frequency|-
timed|debounce time|
reporting|debounce time|frequency
latching|debounce time|
retriggering|timeout|hysteresis
encoder|-|-
encoderauto*|see|later

*encoderauto will be automatically "centered", i.e. Vset = (Vmin+Vmax) / 2 after this call

**pulsePin** `void pulsePin(uint8_t p,unsigned int ms)`

Sends a single ms millisecond pulse to pin p, which must have been configured previously using standard Arduino `pinMode(pin,OUTPUT);`
**WARNING** the pulse must be kept extremely short. This function calls `delay(ms)` and thus long pulses will a) interfere with timing and b) may cause WDT problems / crashes if used indiscriminately.

The parameters to the pin methods should be obvious when read in conjuntion with the pin descriptions above. Common / frequent values are:
* p is the pin number
* mode is the INPUT or INPUT_PULLUP  value as for standard Arduino `pinmode()` function
* debounce is the debounce time in milliseconds
* callback is the name of the callback function
Others will be detailed where they are not obvious

**Debounced** `void Debounced(uint8_t p,uint8_t mode,uint32_t debounce,SMARTPIN_STATE callback);`

**Encoder** `void Encoder(uint8_t pA,uint8_t pB,uint8_t mode,SMARTPIN_STATE callback);`
pA and pB are the *two* hardware pins attached to the encoder. They must *both* be wired electrically the same, since the input mode applies to both pins.

**EncoderAuto** `SmartPins::spEncoderAuto* EncoderAuto(uint8_t pA,uint8_t _pB,uint8_t mode,SMARTPIN_STATE callback,int Vmin=0,int Vmax=100,int Vinc=1,int Vset=0);`

Returns a pointer to an SmartPins::EncoderAuto object, which can be used to call additional methods which only apply to this pin type:

`void center();`  Set encoder to its "center" value - same as  setPercent(50) i.e. (Vmin+Vmax) /2.

`int getValue();`  RFeturns current value.

`void reconfigure(int Vmin,int Vmax,int Vinc,int Vset=0);` "Does what it says on the tin..."

`void setMin();`  Set encoder to its minimum value.

`void setMax();`  Set encoder to its maximum value.

`void setPercent(uint32_t pc);` Set encoder to a position pc% between Vmin and Vmax.

`void setValue(int v);` Set current value.

**Latching** `void Latching(uint8_t p,uint8_t mode,uint32_t debounce,SMARTPIN_STATE callback);`

**Polled** `void Polled(uint8_t p,uint8_t mode,uint32_t freq,SMARTPIN_STATE callback,bool adc=false);`
freq is the frequency in milliseconds with which the pin value will be checked. adc=true causes the pin to be treated as an ADC pin and thus this should only be set when p=A0 (or 17 on many systems). Doing otherwise resukts in undefined behaviour

**Raw** `void Raw(uint8_t p,uint8_t mode,SMARTPIN_STATE callback);`

**Reporting** `void Reporting(uint8_t p,uint8_t mode,uint32_t debounce,uint32_t freq,SMARTPIN_STATE_VALUE callback);`
freq is the frequency with which the callback will be called while the pin remains in the changed state

**Retriggering** `void Retriggering(uint8_t p,uint8_t mode,uint32_t timeout,SMARTPIN_STATE callback,uint32_t active=HIGH,uint32_t hyst=3000);`
active is the state when triggered, hyst is the hysteresis timeout in milliseconds

**Timed** `void Timed(uint8_t p,uint8_t mode,uint32_t debounce,SMARTPIN_STATE_VALUE callback);`

# Code Examples:

It is recommended that you work through these examples in order, as they get more complex. They also include examples of the timer / scheduler functions. The EncoderAuto example contains a hidden "Easter Egg" - if you understand the code fully, you will discover a "secret" method of triggering a congratulation message. Have fun!

* [Raw](../master/examples/00_Raw/00_Raw.ino)
* [Debounced](../master/examples/01_Debounced/01_Debounced.ino)
* [Polled](../master/examples/02_Polled/02_Polled.ino)
* [Timed](../master/examples/03_Timed/03_Timed.ino)
* [Reporting](../master/examples/04_Reporting/04_Reporting.ino)
* [Latching](../master/examples/05_Latching/05_Latching.ino)
* [Retriggering](../master/examples/06_Retriggering/06_Retriggering.ino)
* [Encoder](../master/examples/07_Encoder/07_Encoder.ino)
* [EncoderAuto](../master/examples/08_EncoderAuto/08_EncoderAuto.ino)
* [Hooks](../master/examples/09_Hooks/09_Hooks.ino)


(C) 2017 **Phil Bowles**
philbowles2012@gmail.com
