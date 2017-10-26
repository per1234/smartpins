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

	 vector<SmartPins::hwPin*>				SmartPins::hwPins;
//
//	getValue - returns current "state" of pin (value depends on pin type)
//
int ICACHE_FLASH_ATTR SmartPins::getValue(uint8_t _p){
	auto p=find_if(hwPins.begin(), hwPins.end(),[&](const hwPin* tp) { return _p==tp->p; });
	if(p!=hwPins.end()) return (*p)->getValue();
	else return -666;
}
//
//	loop - must be called frequently to feed underlying H4 scheduler/timer engine
//
void SmartPins::loop(){
	for(const auto& p: hwPins) p->run();
	H4::loop();
}
//
//  pulsePin
//
void ICACHE_FLASH_ATTR SmartPins::pulsePin(uint8_t pin,unsigned int ms){
	uint8_t state=digitalRead(pin);
	::digitalWrite(pin,!state);
	delay(ms);
	::digitalWrite(pin,state);
}
//
//	reconfigurePin - changes initial value(s) of given pin (v1 / v2 change meaning dependent on pin type)
//
bool ICACHE_FLASH_ATTR SmartPins::reconfigurePin(uint8_t _p,int v1, int v2){
	auto p=find_if(hwPins.begin(), hwPins.end(),[&](const hwPin* tp) { return _p==tp->p; });
	if(p!=hwPins.end()) {
		Serial.printf("reconfigurePin %d v1=%d v2=%d\n",_p,v1,v2);
	   (*p)->reconfigure(v1,v2);
	   return true;
	}
	else return false;
}
//
//	smart Pins:
//
//	Debounced	
//
void SmartPins::Debounced(uint8_t _p,uint8_t _mode,uint32_t _debounce,SMARTPIN_STATE _callback){
	hwPins.push_back(new spDebounce(_p,_mode,_debounce,_callback,this));	
}
//
//	Encoder
//
void SmartPins::Encoder(uint8_t _pA,uint8_t _pB,uint8_t _mode,SMARTPIN_STATE _callback){
	hwPins.push_back(new spEncoder(_pA,_pB,_mode,_callback,this));	
}
//
//	EncoderAuto
//
SMARTPIN_ENC_AUTO SmartPins::EncoderAuto(uint8_t _pA,uint8_t _pB,uint8_t _mode,SMARTPIN_STATE _callback,int _Vmin,int _Vmax,int _Vinc,int _Vset){
	SMARTPIN_ENC_AUTO rv=new spEncoderAuto(_pA,_pB,_mode,_callback,this,_Vmin,_Vmax,_Vinc,_Vset);
	hwPins.push_back(rv);
	return rv;
}
//
//	Latching
//
void SmartPins::Latching(uint8_t _p,uint8_t _mode,uint32_t _debounce,SMARTPIN_STATE _callback){
	hwPins.push_back(new spLatching(_p,_mode,_debounce,_callback,this));	
}
//
//	Polled
//
void SmartPins::Polled(uint8_t _p,uint8_t _mode,uint32_t freq, SMARTPIN_STATE _callback,bool adc){
	hwPins.push_back(new spPolled(_p,_mode,freq,_callback,this,adc));
}
//
//	Raw
//
void SmartPins::Raw(uint8_t _p,uint8_t _mode,SMARTPIN_STATE _callback){
	hwPins.push_back(new spRaw(_p,_mode,_callback,this));
}
//
//	Reporting
//
void SmartPins::Reporting(uint8_t _p,uint8_t _mode,uint32_t _debounce,uint32_t _freq,SMARTPIN_STATE_VALUE _callback){
	hwPins.push_back(new spReporting(_p,_mode,_debounce,_freq,_callback,this));	
}
//
//	Retriggering
//
void SmartPins::Retriggering(uint8_t _p,uint8_t _mode,uint32_t _timeout,SMARTPIN_STATE _callback,uint32_t _active,uint32_t _hyst){
	hwPins.push_back(new spRetriggering(_p,_mode,_timeout,_callback,_active,_hyst,this));	
}
//
//	Timed
//
void SmartPins::Timed(uint8_t _p,uint8_t _mode,uint32_t _debounce,SMARTPIN_STATE_VALUE _callback){
	hwPins.push_back(new spTimed(_p,_mode,_debounce,_callback,this));	
}