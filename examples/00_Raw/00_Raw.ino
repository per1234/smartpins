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
 *   message will be printed on every state change. You may see several for each up or down press.
 *   
 *   try some different buttons to compare them
 *   
 */
SmartPins sp;
#define PUSHBUTTON  0

void pinChange(int hilo){
  Serial.printf("T=%d Raw: %s\n",millis(),hilo ? "HI":"LO");
}

void setup(){
    Serial.begin(74880);
    Serial.printf("SmartPins Raw Example, pin=%d\n",PUSHBUTTON); 
    sp.Raw(PUSHBUTTON,INPUT_PULLUP,pinChange);       // no external pullup resistor
    sp.nTimesRandom(3,15000,30000,[](){
        Serial.printf("T=%d demo of fancy timer features that come for free!\n",millis());  
    });
}

void loop(){
  sp.loop();
}
