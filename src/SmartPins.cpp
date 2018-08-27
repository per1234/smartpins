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

pinInt 						SmartPins::async={0,0,0,0,nullptr};
SP_STATE_VALUE				SmartPins::rawHookFn=[](uint32_t,uint32_t){};
SP_STATE_VALUE				SmartPins::cookedHookFn=[](uint32_t,uint32_t){};
SP_STATE_VALUE				SmartPins::throttleHookFn=[](uint32_t,uint32_t){};
	
class SmartPins::flasher;	
vector<SmartPins::flasher*>			SmartPins::fList;	

#if defined(ARDUINO_ESP8266_WEMOS_D1MINI)
SmartPins::SP_PIN SmartPins::spPins[]={
	{3,SP_TYPE_BOOT,nullptr}, // 0
	{SP_MAX_PIN,SP_TYPE_TX,nullptr}, // 1
	{4,SP_TYPE_BOOT,nullptr}, // 2
	{SP_MAX_PIN,SP_TYPE_RX,nullptr}, // 3
	{2,SP_TYPE_GPIO,nullptr}, // 4
	{1,SP_TYPE_GPIO,nullptr}, // 5
	{SP_MAX_PIN,SP_TYPE_CANTUSE,nullptr}, // 6
	{SP_MAX_PIN,SP_TYPE_CANTUSE,nullptr}, // 7
	{SP_MAX_PIN,SP_TYPE_CANTUSE,nullptr}, // 8
	{SP_MAX_PIN,SP_TYPE_CANTUSE,nullptr}, // 9
	{SP_MAX_PIN,SP_TYPE_CANTUSE,nullptr}, // 10
	{SP_MAX_PIN,SP_TYPE_CANTUSE,nullptr}, // 11
	{6,SP_TYPE_GPIO,nullptr}, // 12
	{7,SP_TYPE_GPIO,nullptr}, // 13
	{5,SP_TYPE_GPIO,nullptr}, // 14
	{8,SP_TYPE_BOOT,nullptr},  // 15
	{0,SP_TYPE_WAKE,nullptr}, // 16
	{SP_MAX_PIN,SP_TYPE_ADC,nullptr}  // 17
};
#elif defined(ARDUINO_ESP8266_WEMOS_D1MINILITE)
SmartPins::SP_PIN SmartPins::spPins[]= {
	{3,SP_TYPE_BOOT,nullptr}, // 0
	{0,SP_TYPE_TX,nullptr}, // 1
	{4,SP_TYPE_BOOT,nullptr}, // 2
	{0,SP_TYPE_RX,nullptr}, // 3
	{2,SP_TYPE_GPIO,nullptr}, // 4
	{1,SP_TYPE_GPIO,nullptr}, // 5
	{0,SP_TYPE_CANTUSE,nullptr}, // 6
	{0,SP_TYPE_CANTUSE,nullptr}, // 7
	{0,SP_TYPE_CANTUSE,nullptr}, // 8
	{0,SP_TYPE_GPIO,nullptr}, // 9
	{0,SP_TYPE_GPIO,nullptr}, // 10
	{0,SP_TYPE_CANTUSE,nullptr}, // 11
	{6,SP_TYPE_GPIO,nullptr}, // 12
	{7,SP_TYPE_GPIO,nullptr}, // 13
	{5,SP_TYPE_GPIO,nullptr}, // 14
	{8,SP_TYPE_BOOT,nullptr},  // 15
	{0,SP_TYPE_WAKE,nullptr}, // 16
	{0,SP_TYPE_ADC,nullptr}  // 17
};
#elif defined(ARDUINO_ESP8266_NODEMCU)
SmartPins::SP_PIN SmartPins::spPins[]= {
	{3,SP_TYPE_BOOT,nullptr}, // 0
	{10,SP_TYPE_TX,nullptr}, // 1
	{4,SP_TYPE_BOOT,nullptr}, // 2
	{9,SP_TYPE_RX,nullptr}, // 3
	{2,SP_TYPE_GPIO,nullptr}, // 4
	{1,SP_TYPE_GPIO,nullptr}, // 5
	{0,SP_TYPE_CANTUSE,nullptr}, // 6
	{0,SP_TYPE_CANTUSE,nullptr}, // 7
	{0,SP_TYPE_CANTUSE,nullptr}, // 8
	{0,SP_TYPE_CANTUSE,nullptr}, // 9
	{0,SP_TYPE_CANTUSE,nullptr}, // 10
	{0,SP_TYPE_CANTUSE,nullptr}, // 11
	{6,SP_TYPE_GPIO,nullptr}, // 12
	{7,SP_TYPE_GPIO,nullptr}, // 13
	{5,SP_TYPE_GPIO,nullptr}, // 14
	{8,SP_TYPE_GPIO,nullptr},  // 15
	{0,SP_TYPE_WAKE,nullptr},  // 16
	{0,SP_TYPE_ADC,nullptr}  // 17
};
#elif defined(ARDUINO_SONOFF_BASIC)
SmartPins::SP_PIN SmartPins::spPins[]= {
	{3,SP_TYPE_BN,nullptr}, // 0
	{0,SP_TYPE_TX,nullptr}, // 1
	{0,SP_TYPE_CANTUSE,nullptr}, // 2
	{0,SP_TYPE_RX,nullptr}, // 3
	{0,SP_TYPE_CANTUSE,nullptr}, // 4
	{0,SP_TYPE_CANTUSE,nullptr}, // 5
	{0,SP_TYPE_CANTUSE,nullptr}, // 6
	{0,SP_TYPE_CANTUSE,nullptr}, // 7
	{0,SP_TYPE_CANTUSE,nullptr}, // 8
	{0,SP_TYPE_CANTUSE,nullptr}, // 9
	{0,SP_TYPE_CANTUSE,nullptr}, // 10
	{0,SP_TYPE_CANTUSE,nullptr}, // 11
	{12,SP_TYPE_RY,nullptr}, // 12
	{13,SP_TYPE_LD,nullptr}, // 13
	{5,SP_TYPE_GPIO,nullptr}, // 14
	{0,SP_TYPE_CANTUSE,nullptr},  // 15
	{0,SP_TYPE_CANTUSE,nullptr}, // 16
	{0,SP_TYPE_CANTUSE,nullptr}  // 17

};
#elif defined(ARDUINO_SONOFF_SV)
SmartPins::SP_PIN SmartPins::spPins[]= {
	{3,SP_TYPE_BN,nullptr}, // 0
	{0,SP_TYPE_TX,nullptr}, // 1
	{0,SP_TYPE_CANTUSE,nullptr}, // 2
	{0,SP_TYPE_RX,nullptr}, // 3
	{2,SP_TYPE_GPIO,nullptr}, // 4
	{1,SP_TYPE_GPIO,nullptr}, // 5
	{0,SP_TYPE_CANTUSE,nullptr}, // 6
	{0,SP_TYPE_CANTUSE,nullptr}, // 7
	{0,SP_TYPE_CANTUSE,nullptr}, // 8
	{0,SP_TYPE_CANTUSE,nullptr}, // 9
	{0,SP_TYPE_CANTUSE,nullptr}, // 10
	{0,SP_TYPE_CANTUSE,nullptr}, // 11
	{12,SP_TYPE_RY,nullptr}, // 12
	{13,SP_TYPE_LD,nullptr}, // 13
	{5,SP_TYPE_GPIO,nullptr}, // 14
	{0,SP_TYPE_CANTUSE,nullptr},  // 15
	{0,SP_TYPE_CANTUSE,nullptr},  // 16
	{0,SP_TYPE_CANTUSE,nullptr}  // 17
};
#elif (defined(ARDUINO_ESP8266_ESP01) || defined(ARDUINO_ESP8266_ESP01S))
SmartPins::SP_PIN SmartPins::spPins[]= {
	{3,SP_TYPE_BOOT,nullptr}, // 0
	{1,SP_TYPE_TX,nullptr}, // 1
	{2,SP_TYPE_GPIO,nullptr}, // 2
	{0,SP_TYPE_RX,nullptr}, // 3
	{0,SP_TYPE_CANTUSE,nullptr}, // 4
	{0,SP_TYPE_CANTUSE,nullptr}, // 5
	{0,SP_TYPE_CANTUSE,nullptr}, // 6
	{0,SP_TYPE_CANTUSE,nullptr}, // 7
	{0,SP_TYPE_CANTUSE,nullptr}, // 8
	{0,SP_TYPE_CANTUSE,nullptr}, // 9
	{0,SP_TYPE_CANTUSE,nullptr}, // 10
	{0,SP_TYPE_CANTUSE,nullptr}, // 11
	{0,SP_TYPE_CANTUSE,nullptr}, // 12
	{0,SP_TYPE_CANTUSE,nullptr}, // 13
	{0,SP_TYPE_CANTUSE,nullptr}, // 14
	{0,SP_TYPE_CANTUSE,nullptr},  // 15
	{0,SP_TYPE_CANTUSE,nullptr},  // 16
	{0,SP_TYPE_CANTUSE,nullptr}  // 17	
};
#endif

void __attribute__((weak)) configChange(int pin,int v1,int v2){};
//
//	getValue - returns current "state" of pin (value depends on pin type)
//
int SmartPins::getValue(uint8_t _p){ // hoist this
	if(hwPin* h=isSmartPin(_p))	return h->getValue();
	return -666;
}
//
//	loop - must be called frequently to feed underlying H4 scheduler/timer engine
//
void SmartPins::loop(){
	for(int i=0;i<SP_MAX_PIN;i++) {
		hwPin* p=spPins[i].h;
		if(p) p->run();
	}
	H4::loop();
}
//
//	reconfigurePin - changes initial value(s) of given pin (v1 / v2 change meaning dependent on pin type)
//
void SmartPins::reconfigurePin(uint8_t _p,int v1, int v2){
	if(hwPin* h=isSmartPin(_p)) {
        h->reconfigure(v1,v2);
		configChange(_p,v1,v2);
	}
}
//
//	throttlePin - slows down "noisy" pins lim is in MICROseconds i.e. 5000 = 5ms slowing pin to max 200/sec
//											
//
void SmartPins::throttlePin(uint8_t _p,uint32_t lim){ if(hwPin* h=isSmartPin(_p)) h->setThrottle(lim); }
//
//
//	smart Pins:
//
//	Debounced	
//
void SmartPins::Debounced(uint8_t _p,uint8_t _mode,uint32_t _debounce,SP_STATE _callback){
	if(isValidPin(_p))	spPins[_p].h=new spDebounce(_p,_mode,_debounce,_callback);	
}
//
//	Encoder callback
//
void SmartPins::Encoder(uint8_t _pA,uint8_t _pB,uint8_t _mode,SP_STATE _callback){
	if(isValidPin(_pA) && isValidPin(_pB)) spPins[_pA].h=new spEncoder(_pA,_pB,_mode,_callback);	
}
//
//	Encoder int* binding
//
void SmartPins::Encoder(uint8_t _pA,uint8_t _pB,uint8_t _mode,int* pV){
	if(isValidPin(_pA) && isValidPin(_pB)) spPins[_pA].h=new spEncoder(_pA,_pB,_mode,bind([](int* pV,int v){ *pV+=v; },pV,_1));
}
//
//	EncoderAuto callback
//
SP_ENC_AUTO SmartPins::EncoderAuto(uint8_t _pA,uint8_t _pB,uint8_t _mode,SP_STATE _callback,int _Vmin,int _Vmax,int _Vinc,int _Vset){
	if(isValidPin(_pA) && isValidPin(_pB)){
		SP_ENC_AUTO rv=new spEncoderAuto(_pA,_pB,_mode,_callback,_Vmin,_Vmax,_Vinc,_Vset);
		spPins[_pA].h=rv;		
		return rv;
	}
}
//
//	EncoderAutoi nt* binding
//
SP_ENC_AUTO SmartPins::EncoderAuto(uint8_t _pA,uint8_t _pB,uint8_t _mode,int* pV,int _Vmin,int _Vmax,int _Vinc,int _Vset){
	if(isValidPin(_pA) && isValidPin(_pB)){
		SP_ENC_AUTO rv=new spEncoderAuto(_pA,_pB,_mode,bind([](int* pV,int v){ *pV=v; },pV,_1),_Vmin,_Vmax,_Vinc,_Vset);
		spPins[_pA].h=rv;
		*pV=rv->getValue();	
		return rv;
	}
}
//
//	Filtered
//
void SmartPins::Filtered(uint8_t _p,uint8_t _mode,bool _filter,SP_STATE _callback){
	if(isValidPin(_p))	spPins[_p].h=new spFiltered(_p,_mode,_filter,_callback);
}
//
//	Interrupt
//
void SmartPins::Interrupt(uint8_t _p,uint8_t _mode,int imode,SP_STATE_VALUE _callback,bool active){
	if(isValidPin(_p))	spPins[_p].h=new spInterrupt(_p,_mode,imode,_callback,active);
}
//
//	Latching
//
void SmartPins::Latching(uint8_t _p,uint8_t _mode,uint32_t _debounce,SP_STATE _callback){
	if(isValidPin(_p))	spPins[_p].h=new spLatching(_p,_mode,_debounce,_callback);	
}
//
//	Output
//
void SmartPins::Output(uint8_t _p,bool active,uint8_t initial,SP_STATE _callback){
	if(_p < SP_MAX_PIN)	spPins[_p].h=new spOutput(_p,active,initial,_callback);
}
//
//	Polled
//
void SmartPins::Polled(uint8_t _p,uint8_t _mode,uint32_t freq, SP_STATE _callback,bool adc){
	if(isValidPin(_p))	spPins[_p].h=new spPolled(_p,_mode,freq,_callback,adc);
}
//
//	Raw
//
void SmartPins::Raw(uint8_t _p,uint8_t _mode,SP_STATE _callback){
	if(isValidPin(_p))	spPins[_p].h=new spRaw(_p,_mode,_callback);
}
//
//	Reporting
//
void SmartPins::Reporting(uint8_t _p,uint8_t _mode,uint32_t _debounce,uint32_t _freq,SP_STATE_VALUE _callback){
	if(isValidPin(_p))	spPins[_p].h=new spReporting(_p,_mode,_debounce,_freq,_callback);	
}
//
//	Retriggering
//
void SmartPins::Retriggering(uint8_t _p,uint8_t _mode,uint32_t _timeout,SP_STATE _callback,bool _active){
	if(isValidPin(_p))	spPins[_p].h=new spRetriggering(_p,_mode,_timeout,_callback,_active);	
}
//
//	ThreeStage
//
void SmartPins::ThreeStage(uint8_t _p,uint8_t mode,uint32_t _debounce,uint32_t f,SP_STATE _callback,SP_STATE _sf,uint32_t _m,SP_STATE _mf,uint32_t _l,SP_STATE _lf){
	if(isValidPin(_p))	spPins[_p].h=new spThreeStage( _p, mode, _debounce, f, _callback, _m, _l, _sf, _mf, _lf);		
}
//
//	Timed
//
void SmartPins::Timed(uint8_t _p,uint8_t _mode,uint32_t _debounce,SP_STATE_VALUE _callback){
	if(isValidPin(_p))	spPins[_p].h=new spTimed(_p,_mode,_debounce,_callback);	
}
//
//	flasher
//
using FLASHER_HANDLER = function<bool(SmartPins::flasher*)>;
//	private general-purpose cores	
bool SmartPins::_doFlasher(uint8_t pin,FLASHER_HANDLER fn){
	auto p=find_if(fList.begin(), fList.end(),[&](const flasher* tp) {	return pin==tp->_pin; });
	return p==fList.end() ? false:fn(*p);
}
void SmartPins::_flash(int period,int duty,uint8_t pin,VFN fn){
	if(isValidPin(pin) && duty < 100){
		if(_doFlasher(pin,bind([](flasher* p,int period,uint8_t duty,VFN fn){
			p->_PWM(period,duty,fn);
			return true;
			},_1,period,duty,fn)));
		else fList.push_back(new flasher(pin,period,duty,fn));
	}		
}
//
// flasher public functions
//
void SmartPins::flashLED(const char* pattern,int timebase,uint8_t pin){// flash arbitrary pattern
	if(isValidPin(pin)){
		if(_doFlasher(pin,bind([](flasher* p,const char* pattern,int timebase){ p->flashPattern(timebase,pattern); return true; },_1,pattern,timebase)));
		else fList.push_back(new flasher(pin,pattern,timebase));
	}
}

void SmartPins::flashLED(int period,int duty,uint8_t pin){ _flash(period,duty,pin); }// flash "PWM"

void SmartPins::flashLED(int period,uint8_t pin){ flashLED(period*2,50,pin); }// simple symmetric SQ wave on/off

void SmartPins::pulseLED(int period,uint8_t pin){ _flash(period,0,pin); }

bool SmartPins::isFlashing(uint8_t pin){
	if(isValidPin(pin)) return _doFlasher(pin,[](flasher* p){ return p->_isFlashing(); });	
}

void SmartPins::stopLED(uint8_t pin){
	if(isValidPin(pin)){
		_doFlasher(pin,[](flasher* p)->bool{
			p->_stop();
			delete p;
			});
		fList.erase( remove_if(fList.begin(), fList.end(),[pin](flasher* p) {return pin==p->_pin;} ),fList.end());
	}			
}
//
//		fundamentals
//
void SmartPins::digitalWrite(uint8_t pin,uint8_t value){
	if(isOutputPin(pin)){
		::digitalWrite(pin,value);
		if(rawHookFn) rawHookFn(pin,value);
	}
}
/*
 * graveyard
 *vector<string> Morse={ /// FIX!!!!!!!!!!!!!!!!!!
				".-",
				"-...",
				"-.-.",
				"-..",
				".",
				"..-.",
				"--.",
				"....",
				"..",
				".---",
				"-.-",
				".-..",
				"--",
				"-.",
				"---",
				".--.",
				"--.-",
				".-.",
				"...",
				"-",
				"..-",
				"...-",
				".--",
				"-..-",
				"-.--",
				"--.."
			};
// what it DIAGs on the tin
void SmartPins::flashMorse(const char* msg,uint8_t pin){
	int len=strlen(msg);
	String pattern;
	for(int i=0;i<len;i++){
		char c=tolower(msg[i]);
		if(c==' ') pattern.concat(c);
		else {
			if(c > 0x60 && c < 0x7b) {
				pattern.concat(CSTR(Morse[c-0x61]));
				pattern.concat(' ');
			}
		}
	}
	flashLED(CSTR(pattern),200,pin);
}
void SmartPins::flashSOS(uint8_t pin){ flashMorse("SOS  ",pin); }

//		we are so low on ram that debug mode busts ram limits, so we are excluding the hogs...
#ifdef ESPARTO_DEBUG_PORT
const char* SmartPins::types[]={
	" ",
	"BOOT",
	"RX",
	"TX",
	"GPIO",
	"WAKE",
	"ADC",
	"LD",
	"BN",
	"RY",
	"NONAME"
	};
const char* SmartPins::styles[]={
	" ",
	"OUTPUT",
	"RAW",
	"DBOUNCE",
	"FILTER",
	"LATCH",
	"RETRIGR",
	"ENCODER",
	"ENCAUTO",
	"REPORT",
	"TIMED",
	"POLLED",
	"INTRUPT",
	"3-STAGE"
};
#endif
//		we are so low on ram that debug mode busts ram limits, so we are excluding the hogs...
#ifdef ESPARTO_DEBUG_PORT
		static const char* types[];
		static const char* styles[];
#endif
void SmartPins::dumpPins(){
	DIAG("GPIO\tArduino\tType\tStyle\tActive\tRaw\tCooked\n");
	for(int i=0;i<SP_MAX_PIN;i++){
		if(hwPin* h=isSmartPin(i)){
			DIAG("%2d\t%s\t%s\t%s\t%s\t%d\t%d\n",
					  i,
					  CSTR(getArduinoPin(i)),
					  types[spPins[i].type],
					  styles[h->getStyle()],
					  h->getActive() ? "HI":"LO",
					  digitalRead(i),
					  h->getValue()
			);	
		}
	}
}

//		void 		flashMorse(const char* msg,uint8_t pin=LED_BUILTIN);
//		void 		flashSOS(uint8_t pin=LED_BUILTIN);

		static	void 		dumpPins();


*/