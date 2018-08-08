![smartpins Logo](/assets/pins.png)

## Introduction

I have chosen to withdraw support and documentation - apologies to anybody who may have downloaded it, but there is a good reason:

My H4 and Smartpins libraries have been significantly re-written to provide support for the imminent release of Esparto v2.0

To reflect this, they will be re-released as version 2.0.0 to coincide with the release of Esparto v2.0

The great leap in verion number signifies the major change in functionality - principally an inbuilt webserver - and there is little reason to now use H4 / Smartpins as "standalones" as all their functionality and (a whole lot more) is encapsulated in Esparto v2.0

In the meantime, enjoy this fun snippet of Esparto running a Wemos D1 as a music-sync'd LED flasher in just 6 lines of code. That's not a misprint, it's six lines of code, total in the Arduino IDE.

[![ESP8266 Music Flasher](https://i.ytimg.com/vi/X0bfMkeNdRQ/hqdefault.jpg?sqp=-oaymwEZCPYBEIoBSFXyq4qpAwsIARUAAIhCGAFwAQ==&rs=AOn4CLBb7E8RB5tFxhDwsj1DdJNXb2BkRw)](https://youtu.be/X0bfMkeNdRQ)

All done by:

```
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
*   Numerous working examples, making it an ideal self-teaching tool
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
*	Fallback to “last known good” SSID / password to protect against lockout if  entering an incorrect password
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
*   Latching
*   Polled
*   Raw
*   Reporting
*   Retriggering
*   Timed

## Flasher Functions

*   Single-shot timed pulse   
*   Symmetric on/off a.k.a "Blinky"
*   Slow PWM - asymmetric by pulse width / duty cycle
*   Arbitrary Pattern e.g. "... --- ..." = S-O-S
*   Morse Code by String: e.g. flashLED("Esparto") (just for those who served in the Militray in the 50s and 60s...)
*   Morse Code pure, for those of the above who like things tough and can still "fist" 140WPM
