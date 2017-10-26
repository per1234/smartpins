/*
 MIT License

Copyright (c) 2017 Phil Bowles

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <SmartPins.h>
/*
 *   You will need a pushbutton for this demo, preferably one that "bounces" a lot
 *   
 *   change the PUSHBUTTON value below to match the chosen pin
 *   if you use an external PULLUP (10k recommended) use mode INPUT
 *   if not using external pullup                    use mode INPUT_PULLUP
 *   
 *   message will be printed on every changed of Latched state 
 *      
 */
SmartPins sp;
#define PUSHBUTTON  0
#define DEBOUNCE_TIME 15

void pinChange(int hilo){
  Serial.printf("T=%d Latching: state=%s\n",millis(),hilo ? "DC":"AC");
}

void setup(){
    Serial.begin(74880);
    Serial.printf("SmartPins Latching Example, pin=%d debounce=%dms\n",PUSHBUTTON,DEBOUNCE_TIME); 
    sp.Latching(PUSHBUTTON,INPUT_PULLUP,DEBOUNCE_TIME,pinChange);       // no external pullup resistor
    // sometime between 30s and 1 minute change debounce time to 1ms
    // *there is an inbuilt "latency" in running the debounce code...if this is longer than than the actual bouncing,
    // even a value of 0 may remove the bounce. try for as low a value as possible...
    sp.onceRandom(30000,60000,[](){
      sp.reconfigurePin(PUSHBUTTON,1);
      });
}

void loop(){
  sp.loop();
}
