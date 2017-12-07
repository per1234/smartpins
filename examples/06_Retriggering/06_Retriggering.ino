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
 *   You will need a pushbutton for this demo
 *   
 *   change the PUSHBUTTON value below to match the chosen pin
 *   if you use an external PULLUP (10k recommended) use mode INPUT
 *   if not using external pullup                    use mode INPUT_PULLUP
 *   
 */
SmartPins sp;
#define PUSHBUTTON  0
#define TIMEOUT 10000

void pinChange(int hilo){
  Serial.printf("T=%d Retriggering: state=%d\n",millis(),hilo);
}

void setup(){
    Serial.begin(74880);
    Serial.printf("SmartPins Retriggering Example, pin=%d timeout=%dms\n",PUSHBUTTON,TIMEOUT); 
    sp.Retriggering(PUSHBUTTON,INPUT_PULLUP,TIMEOUT,pinChange,LOW);       // no external pullup resistor

// sometime between 1:10s and 1:30 change timeout value to half 
    sp.onceRandom(70000,90000,[](){
      Serial.printf("T=%d set timeout to %d\n",millis(),TIMEOUT / 2);
      sp.reconfigurePin(PUSHBUTTON,TIMEOUT / 2);
      });
}

void loop(){
  sp.loop();
}
