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
 *   message will be printed on every debounced state change. You should see only ONE per up or down press.
 *   if you see more, increase the mS value
 *   also observe what happens if you put a very large value (e.g. 1000 (=1 sec))
 *   experiment with the value to get the smallest mS that gives no bounces
 *   
 *   try some different buttons to compare them
 *   
 */
SmartPins sp;
#define PUSHBUTTON  0
#define DEBOUNCE_TIME 15

void pinChange(int hilo){
  Serial.printf("T=%d Debounced: %s\n",millis(),hilo ? "HI":"LO");
}

void setup(){
    Serial.begin(74880);
    Serial.printf("SmartPins Debounced Example, pin=%d debounce=%dms\n",PUSHBUTTON,DEBOUNCE_TIME); 
    sp.Debounced(PUSHBUTTON,INPUT_PULLUP,DEBOUNCE_TIME,pinChange);       // no external pullup resistor
    // after 1 minute change debounce time to 1ms and now bouncing may* re-appear
    // *there is an inbuilt "latency" in running the debounce code...if the actual bounce is shorter than the scheduling time,
    // even a value of 0 may remove the bounce. Try for as low a value as possible...you can adjust it "on the fly"
    sp.once(60000,[](){
      Serial.printf("Changing debounce time to 1ms\n");
      sp.reconfigurePin(PUSHBUTTON,1);
      Serial.printf("You may well see bouncing\n");
      });
}

void loop(){
  sp.loop();
}
