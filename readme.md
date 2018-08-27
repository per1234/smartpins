![smartpins Logo](/assets/pins.png)

# Introduction

## SmartPins and H4 libraries are essential components of Esparto v2.0 They are designed to be used in the Arduino IDE 1.8.6 with the esp8266 core 2.4.2

* https://www.arduino.cc/en/Main/Software
* https://github.com/esp8266/arduino

They each need to be downloaded and installed prior to Esparto v2.0. They will be supported only as part of the Esparto project, so any isses / requests / etc should be made only on the Esparto pages.

* https://github.com/philbowles/h4
* https://github.com/philbowles/smartpins
* https://github.com/philbowles/esparto

All installation / documentation etc is on the Esparto site. Further information on my blog:
https://8266iot.blogspot.com/

Here's a fun snippet of what it can do:

[![ESP8266 Music Flasher](https://i.ytimg.com/vi/X0bfMkeNdRQ/hqdefault.jpg?sqp=-oaymwEZCPYBEIoBSFXyq4qpAwsIARUAAIhCGAFwAQ==&rs=AOn4CLBb7E8RB5tFxhDwsj1DdJNXb2BkRw)](https://youtu.be/X0bfMkeNdRQ)

The total code for the above is :

```cpp
#include <ESPArto.h>
ESPArto Esparto;
void setupHardware(){
    Esparto.pinMode(LED_BUILTIN,OUTPUT,LOW,HIGH);
    Esparto.Raw(D6,INPUT_PULLUP,[](int v){ if(v) smartPins.pulseLED(1); });
}
```

# SUMMARY OF ESPARTO MAIN FEATURES

## Ease of use
*   WiFi + MQTT control built-in and ready "out of the box"
*	Extremely simple programming interface with Arduino IDE.
*   Numerous (32) working code examples, making it an ideal self-teaching tool
*	Flexibility: create apps from simple "blinky" to fully-featured, complex, resilient IOT / home automation firmware
*	Tested on a wide variety of hardware: ESP-01, SONOFF, Wemos D1, NodeMCU etc
## Rapid development
*	Most common errors and “gotchas” avoided
*	Many flexible input-pin options pre-configured e.g. fully debounced rotary encoder support with a single line of code
*	Create MQTT controlled firmware in only 15 lines of code
*	User code hugely simplified, consisting mainly of short callback functions
*	Several flexible asynchronous LED flashing functions including slow PWM, arbitrary pattern e.g. "... --- ..." for SOS, 
*	Modular: Esparto “Lite” / Esparto WiFi / Esparto MQTT: use only what you need / your experience matches
## “Industrial strength”
*	Voice-driven compatibility with Amazon Alexa (via Belkin Wemo emulation)
*	Copes resiliently with WiFi outage or total network loss, reconnecting automatically without requiring reboot
*	Hardware features continue to function at all times irrespective of connection status
*	OTA updates including local server per ESPHTTPUdate protocol
*	Serialises all asynchronous events into main-loop task queue, avoiding WDT resets and obviating volatile/ISR etc
*	Web UI showing real-time GPIO status and providing MQTT simulator
*	MQTT simulator with numerous command / control functions
*	Highly configurable through Web UI
*	Captive portal AP mode for initial configuration 
*	Instant dynamic reconnect on SSID / password change, with no reboot

# Main API functionality

## Timers (operate on "bare" function, arbitrary class methos or std::function)
*   Repetetive
*   Random Repetetive
*   n Times
*   n Times Random
*   Single-shot
*   Single-shot Random
*   Random Times
*   Random Times Random (mayhem)
*   Conditional Firing (when X happens, do Y. X can be any valid c++ expression / function returning a bool)
*   Repetetive Conditional (best to reset condtion as first thing...)
*   Instantaneous - submit function to main task queue NOW

Repetetive timer are cancellable before natural expiry
Expring timers have a chain paramter to allow extended. complex, ordered events with no loops or delays

## Pin Types

*   Debounced
*   Encoder
*   EncoderAuto
*   Filtered
*   Interrupt
*   Latching
*   Output
*   Polled
*   Raw
*   Reporting
*   Retriggering
*   ThreeStage
*   Timed

## Flasher Functions

*   Single-shot timed pulse   
*   Symmetric on/off a.k.a "Blinky"
*   Slow PWM - asymmetric by pulse width / duty cycle
*   Arbitrary Pattern e.g. "... --- ..." = S-O-S

## Other Functions:

Esparto contains sufficient functionality to create replacement firmware for e.g. SONOFF in 20-odd lines of code

(C) 2018 Phil Bowles <esparto8266@gmail.com>
http://8266iot.blogspot.com
