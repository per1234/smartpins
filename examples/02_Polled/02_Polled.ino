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
 *   If you have an analog sensor, connect it to A0. If not, this may still work,
 *   but it will produce very low random values. If not, try attaching a wire
 *   to A0 and holding the end of it tightly to see values change
 *   
 *   REMEMBER: the pin does not necessarily callback every FREQUENCY ms
 *   It *CHECKS THE STATE* every FREQUENCY ms, and calls back only if it has changed, so if you 
 *   see no output, the pin hasn't changed!
 *     
 *   If you get no joy from the analog input, change A0 to a digital pin, attach a button
 *   then toggle it repeatedly. Don't forget to change:
 *    sp.Polled(SENSOR,INPUT,FREQUENCY,pinChange,true);
 *    to
 *    sp.Polled(SENSOR,INPUT,FREQUENCY,pinChange,false);
 *    or simply
 *    sp.Polled(SENSOR,INPUT,FREQUENCY,pinChange);
 *    if you choose to use a digital input
 *   
  */
SmartPins sp;
#define SENSOR    A0
#define FREQUENCY 5000     // every 5 seconds

void pinChange(int hilo){
  Serial.printf("T=%d Polled: %d\n",millis(),hilo);
}

void setup(){
    Serial.begin(74880);
    Serial.printf("SmartPins Polled Example, pin=%d frequency=%dms\n",SENSOR,FREQUENCY); 
    sp.Polled(SENSOR,INPUT,FREQUENCY,pinChange,true);      // true = analogRead, for digital pins, default false is used
    // after 30sec slow down frequency to every 10 seconds
    sp.once(30000,[](){
      Serial.printf("Let's slow things down...\n");
      sp.reconfigurePin(SENSOR,FREQUENCY * 2);
      });
}

void loop(){
  sp.loop();
}
