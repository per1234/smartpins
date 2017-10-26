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
 *   You will need an Encoder for this demo, preferably one that "bounces" a lot
 *   
 *   change A & B values below to match the chosen pins of your encoder
 *   if you use an external PULLUP (10k recommended) use INPUT
 *   if not using external pullup                    use INPUT_PULLUP
 *   
 *   current value
 *   
 */
SmartPins sp( [](int pin,int state){
                Serial.printf("Pin %d COOKED=%d\n",pin,state);        
              },
              [](int pin,int state){
                Serial.printf("Pin %d RAW=%d\n",pin,state);        
              });
              
#define A  5  // D1 on a Wemos D1 Mini
#define B  4  // D2 ...

int value=42;

void pinChange(int dir){
  Serial.printf("T=%d Encoder click: dir=%d\n",millis(),dir);
  value+=dir;
}

void setup(){
    Serial.begin(74880);
    Serial.printf("SmartPins Hooks Example, pinA=%d pinB=%d\n",A,B); 
    sp.Encoder(A,B,INPUT,pinChange);       // external pullup resistor
}

void loop(){
  sp.loop();
}
