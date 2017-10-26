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

/*
	ESP8266 Arduino library providing various different HW pin handlers, e.g. debounced, latching, rotary encoder
	each pin is treated as having "raw" and "cooked" events:
	
	"raw" is what the pin is actually doing in realtime, "cooked" is what the pin sends to the user depending
	on the semantics of the pin functionality.
	Example:
					  Raw Event       Cooked Event
		Latching pin  	0->1			-
						1->0			0->1    (i.e. latched=true)
						0->1			-
						1->0			1->0	(i.e. latched=false)
		
		Debounced pin	0->1			-
						1->0
						0->1
						...   up to debounce
								time
						1->0
						0->1			0->1
						
	provides external hooks to receive raw/cooked events for post-processing
	
	Requires H4 timer / scheduler library (also by this author)
	
*/
#ifndef SMARTPINS_H
#define SMARTPINS_H

#include "changelog.h"

#include <Arduino.h>
#include <functional>

#include <H4.h>

using namespace std;
using namespace std::placeholders;

typedef function<void(int)>           SMARTPIN_STATE;
typedef function<void(int,int)>       SMARTPIN_STATE_VALUE;

enum{
	SMARTPIN_STYLE_UNUSED,
	SMARTPIN_STYLE_OUTPUT,
	SMARTPIN_STYLE_RAW,
	SMARTPIN_STYLE_DEBOUNCED,
	SMARTPIN_STYLE_LATCHING,
	SMARTPIN_STYLE_RETRIGGERING,
	SMARTPIN_STYLE_ENCODER,
	SMARTPIN_STYLE_ENCODER_AUTO,
	SMARTPIN_STYLE_REPORTING,
	SMARTPIN_STYLE_TIMED,
	SMARTPIN_STYLE_POLLED
};
class hwPin;

class SmartPins: public H4 {
		SMARTPIN_STATE_VALUE	rawHookFn,cookedHookFn;		
	protected:
//
//	hwPin - virtual base class
//
		class hwPin {
			protected:
				uint32_t        last;

				virtual void raw(int v)=0;
				virtual void prepared(int v)=0;
				virtual void cooked(int v){}
			public:
				SmartPins*	sp;
				uint32_t        state;
				uint8_t         p;
				int				style=SMARTPIN_STYLE_UNUSED;
				hwPin(uint8_t _p,uint8_t mode,SmartPins* _sp){ 
					 p=_p;
					 sp=_sp;
					 pinMode(p,mode);
					 state=digitalRead(p);
				 }
				 void simCooked(int v){ prepared(v); }
				 void simRaw(uint32_t instant){
					if(instant!=state){
						 state=instant;
						 if(sp->rawHookFn) sp->rawHookFn(p,instant);
						 raw(instant);
						 last=micros();
					 }
				 }
				 virtual void run(){ simRaw(digitalRead(p)); }
				 virtual int getValue(){ return state; }
				 virtual void reconfigure(int v1,int v2)=0;
			};
			
			static vector<hwPin*>				hwPins;
			
	public:
//
//	smartPin - base class templated by callback type ( SMARTPIN_STATE || SMARTPIN_STATE_VALUE)
//		
			template<typename T>
			class smartPin: public hwPin {
				protected:
					T              callback;
				public:
					smartPin(uint8_t _p,uint8_t _mode,T _callback,SmartPins* _sp): hwPin(_p,_mode,_sp) { callback=move(_callback); }
					virtual void prepared(int v){
						if(sp->cookedHookFn) sp->cookedHookFn(p,v);
						cooked(v);
					}

			};
//
//	spRaw - feeds all transitions to caller
//		
			class spRaw: public smartPin<SMARTPIN_STATE>{
					void raw(int v){ callback(v); }
					void reconfigure(int v1,int v2){}
				public:
					spRaw(uint8_t _p,uint8_t mode,SMARTPIN_STATE _callback,SmartPins* _sp): smartPin(_p,mode,_callback,_sp) {
						style=SMARTPIN_STYLE_RAW;
					}    
			};
//
//	spPolled - reports state every N milliseconds
//
			class spPolled: public smartPin<SMARTPIN_STATE>{
					bool	adc;
					void raw(int v){ }
					void cooked(int v){	callback(v); }
					uint32_t 	lastpoll;
					H4_TIMER	timer;
					void setTimer(uint32_t freq){
						timer=sp->every(freq,bind([](spPolled* me){
									uint32_t instant;
									instant=me->adc ? analogRead(me->p):digitalRead(me->p);
									if(me->lastpoll!=instant){
										me->lastpoll=instant;
										me->prepared(instant);
									}
								},this));  		
					}
				public:
					int getValue(){ return static_cast<int>(lastpoll); }
					void reconfigure(int v1, int v2){
						sp->never(timer);
						setTimer(v1);
					}
					spPolled(uint8_t _p,uint8_t mode,uint32_t freq,SMARTPIN_STATE _callback,SmartPins* _sp,bool _adc): smartPin(_p,mode,_callback,_sp) {
						style=SMARTPIN_STYLE_POLLED;
						adc=_adc;
						lastpoll=adc ? analogRead(p):digitalRead(p);
						setTimer(freq);
					}
			};
//
//	_deSpike - base class for all subsequent dependees on a debounced (by N millisecnods) input	
//
			template<typename T>
			class _deSpike: public smartPin<T>{
					bool        bouncing=false;
					int         savedState;

					void raw(int hilo){
						if(!bouncing){
							savedState=smartPin<T>::state;
							bouncing=true;
							smartPin<T>::sp->once(debounce,bind([](_deSpike<T>* me){
									me->bouncing=false;
									if(me->state==me->savedState) me->prepared(me->state);
							},this));
						}
					}
					
				protected:
					uint32_t    debounce;
					virtual void prepared(int v)=0;
					void reconfigure(int v1, int v2=0){	debounce=v1; }

				public:
					_deSpike(uint8_t _p,uint8_t mode,uint32_t _debounce,T _callback,SmartPins* _sp): smartPin<T>(_p,mode,_callback,_sp) {debounce=_debounce;}
			};
//
//	spDebounce - straightforward debounced input pin
//
			class spDebounce: public _deSpike<SMARTPIN_STATE> {
					void cooked(int v){ callback(v); }	
					void prepared(int v){ smartPin<SMARTPIN_STATE>::prepared(v); }
				public:
					spDebounce(uint8_t _p,uint8_t mode,uint32_t _debounce,SMARTPIN_STATE _callback,SmartPins* _sp): _deSpike<SMARTPIN_STATE>(_p,mode,_debounce,_callback,_sp){
						style=SMARTPIN_STYLE_DEBOUNCED;
					}    
			};
//
//	spTimed - reports time (in ms) that pin was held for
//
			class spTimed: public _deSpike<SMARTPIN_STATE_VALUE>{
						uint32_t     startTime=0;
						void cooked(int v){ callback(state,v); }
						void prepared(int v){
							if(startTime){
								smartPin<SMARTPIN_STATE_VALUE>::prepared((last-startTime)/1000);
								startTime=0;
								}
							else{
								smartPin<SMARTPIN_STATE_VALUE>::prepared(state);
								startTime=last;
							}
						}
				public:
					spTimed(uint8_t _p,uint8_t mode,uint32_t _debounce,SMARTPIN_STATE_VALUE _callback,SmartPins* _sp): _deSpike<SMARTPIN_STATE_VALUE>(_p,mode,_debounce,_callback,_sp) {
						style=SMARTPIN_STYLE_TIMED;
					}    
			};
//
//	spReporting - as for timed, but also reports intervening whole periods of f
//				e.g. if f=1000 and pin is held for 3.25sconds, will report 1000, 2000, 3000 and 3250
//
			class spReporting: public _deSpike<SMARTPIN_STATE_VALUE>{
						uint32_t     startTime=0;
						uint32_t     freq;
						uint32_t     fincr=0;
						H4_TIMER     timer;
						void cooked(int v){ callback(state,v); }
						void prepared(int hilo){
							if(startTime){
								sp->never(timer);
								smartPin<SMARTPIN_STATE_VALUE>::prepared((last-startTime)/1000);
								startTime=0;
								fincr=0;
							}
							else{
								smartPin<SMARTPIN_STATE_VALUE>::prepared(state);
								startTime=last;
								timer=sp->every(freq,bind([](spReporting* me){
									me->fincr+=me->freq;
									me->callback(me->state,me->fincr);
									},this));      
							}
						}
				public:
					void reconfigure(int v1, int v2){
						_deSpike<SMARTPIN_STATE_VALUE>::reconfigure(v1);
						freq=v2;
					} 
					spReporting(uint8_t _p,uint8_t mode,uint32_t _debounce,uint32_t f,SMARTPIN_STATE_VALUE _callback,SmartPins* _sp): _deSpike(_p,mode,_debounce,_callback,_sp) {
						style=SMARTPIN_STYLE_REPORTING;
						freq=f;
					}    
			};
//
//	spLatching - transforms momentary transition to permanent state
//				 e.g. one up/down (or down/up) sets latched=true, second up/down (etc..) set latched=false
//				 NB always starts unlatched, i.e. latched=false
//
			class spLatching: public _deSpike<SMARTPIN_STATE>{
					uint8_t     count=0;
					bool        latched;
					
				public:
					int getValue(){ return static_cast<int>(latched); }
					spLatching(uint8_t _p,uint8_t mode,uint32_t _debounce,SMARTPIN_STATE _callback,SmartPins* _sp): _deSpike<SMARTPIN_STATE>(_p,mode,_debounce,_callback,_sp) {
						style=SMARTPIN_STYLE_LATCHING;
						latched=state;
					}
					void cooked(int v) { callback(v); }
					void prepared(int v){
						if(++count > 1){
							latched=!latched;
							count=0;
							smartPin<SMARTPIN_STATE>::prepared(latched);
						}
					}
			};
//
//	spRetriggering - 1st transition starts timer. 2nd transition reported only after delay of N ms.
//					 Intervening transitions restart timer just like e.g. a PIR sensor
//
			class spRetriggering: public smartPin<SMARTPIN_STATE>{
								uint32_t    timeout;
								uint32_t	active;
								uint32_t	hysteresis;
					volatile 	uint32_t    timer=0;
					volatile	uint32_t	hTimer=0;
					
					virtual void cooked(int hilo){ callback(hilo);	}
			
					void raw(int hilo){
						if(!hTimer){
							if(hilo==active){
								if(timer) sp->never(timer);
								else prepared(hilo);
								timer=sp->once(timeout,bind([](spRetriggering* me){
												me->timer=0;
												if(me->state!=me->active) {
													me->prepared(me->state);
													me->hTimer=me->sp->once(me->hysteresis,bind([](spRetriggering* me){
														me->hTimer=0;
														},me));			  
																  
												}
										},this));
							}
							else if(!timer) prepared(hilo); // inactive after timer expiry
						}
					}
				public:
					int getValue(){ return static_cast<int>(timer!=0); }
					void reconfigure(int v1, int v2) {
						timeout=v1;
						hysteresis=v2;
						}
					spRetriggering(uint8_t _p,uint8_t mode,uint32_t _timeout,SMARTPIN_STATE _callback,uint32_t _active,uint32_t _hyst,SmartPins* _sp): smartPin<SMARTPIN_STATE>(_p,mode,_callback,_sp) {
						style=SMARTPIN_STYLE_RETRIGGERING;
						timeout=_timeout;
						hysteresis=_hyst;
						active=_active;
						if(state==_active) raw(_active); // already triggered at startup
					 }
			};
//
//	spEncoder - standard rotary encoder. reports +1 or -1 per click in clock/anticlock-wise direction
//				NB requires TWO pins (of course)
//
			class spEncoder: public spRaw{
					int			smartState=0;
				protected:
					int8_t      rot_states[16] =   {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
					uint8_t     AB = 0x03;
					
					void cooked(int v){
						if(sp->cookedHookFn) sp->cookedHookFn(pinB->p,v);
						callback(v);
						}
					
					void raw(int v){
						static int val=0;
						AB <<= 2;                  
						AB |= ((state<<1 | pinB->state) & 0x03); 
						val+=rot_states[AB & 0x0f];
						if(val==4 || val==-4){
							prepared(smartState=val/4);
							val=0;
						}       
					}
				public:
					spRaw*      pinB;
					int getValue(){ return smartState; }
					spEncoder(uint8_t _p,uint8_t _pB,uint8_t _mode,SMARTPIN_STATE _callback,SmartPins* _sp): spRaw(_p,_mode,_callback,_sp) {
						pinB=new spRaw(_pB,_mode,bind(&spEncoder::raw,this,_1),_sp);
						pinB->style=style=SMARTPIN_STYLE_ENCODER;
						}    
					void run(){
						spRaw::run();
						pinB->run();  
					}  
			};
//
// spEncoderAuto - "smart" encoder: will report absolute value (Vset) between Vmin Vmax
//				    Vset defaults to midway between Vmin and Vmax (i.e. (Vmin + Vmax) / 2)
//					Vmin / Vmax default to 0/100 (thus with Vset=50)
//
			class spEncoderAuto: public spEncoder{
				int	prev;
					
				int value,Vmin,Vmax,Vinc;
					
				public:
					int getValue(){ return value; }
					void reconfigure(int _Vmin,int _Vmax,int _Vinc,int _Vset=0){
						Vmin=_Vmin;
						Vmax=_Vmax;
						Vinc=_Vinc;
						if(_Vset) setValue(_Vset);
						else center();
					}
					
					spEncoderAuto(uint8_t _pin,uint8_t _pinB,uint8_t _mode,SMARTPIN_STATE _callback,SmartPins* _sp,int _Vmin=0,int _Vmax=100,int _Vinc=1,int _Vset=0): spEncoder(_pin,_pinB,_mode,_callback,_sp) {
						pinB->style=style=SMARTPIN_STYLE_ENCODER_AUTO;
						reconfigure(_Vmin,_Vmax,_Vinc,_Vset=0);
					}
					void setMin(){
						setValue(Vmin);
					}  
					void setMax(){
						setValue(Vmax);
					}
					void setPercent(uint32_t pc){
						setValue((Vmax-Vmin)*pc/100);
					}
					void setValue(int v){
						value=constrain(v,Vmin,Vmax);
						if(value!=prev){
							prev=value;
							cooked(value);
						}
					}
					void center(){
						setValue((Vmin+Vmax)/2);
					}
					void prepared(int v){
						setValue(value+(Vinc*v));
					}
			};
			
				
		SmartPins(SMARTPIN_STATE_VALUE _cookedHook=nullptr, SMARTPIN_STATE_VALUE _rawHook=nullptr){
			setCookedHook(_cookedHook);
			setRawHook(_rawHook);
		}
		~SmartPins(){}
				
		int	 getValue(uint8_t _p);
		void loop();
		bool reconfigurePin(uint8_t _p,int v1, int v2=0);
		void pulsePin(uint8_t pin,unsigned int ms);
	
		void Debounced(uint8_t _p,uint8_t _mode,uint32_t _debounce,SMARTPIN_STATE _callback);	
		void Encoder(uint8_t _pA,uint8_t _pB,uint8_t mode,SMARTPIN_STATE _callback);	
		SmartPins::spEncoderAuto* EncoderAuto(uint8_t _pin,uint8_t _pinB,uint8_t _mode,SMARTPIN_STATE _callback,int _Vmin=0,int _Vmax=100,int _Vinc=1,int _Vset=0);	
		void Latching(uint8_t _p,uint8_t _mode,uint32_t _debounce,SMARTPIN_STATE _callback);	
		void Polled(uint8_t _p,uint8_t _mode,uint32_t freq,SMARTPIN_STATE _callback,bool adc=false);
		void Raw(uint8_t _p,uint8_t _mode,SMARTPIN_STATE _callback);
		void Reporting(uint8_t _p,uint8_t mode,uint32_t _debounce,uint32_t _freq,SMARTPIN_STATE_VALUE _callback);	
		void Retriggering(uint8_t _p,uint8_t _mode,uint32_t _timeout,SMARTPIN_STATE _callback,uint32_t active=HIGH,uint32_t hyst=3000);	
		void Timed(uint8_t _p,uint8_t mode,uint32_t _debounce,SMARTPIN_STATE_VALUE _callback);
	protected:
		void setCookedHook(SMARTPIN_STATE_VALUE fn){cookedHookFn=fn;}
		void setRawHook(SMARTPIN_STATE_VALUE fn){rawHookFn=fn;}	
};

typedef SmartPins::spEncoderAuto*	SMARTPIN_ENC_AUTO;

extern SmartPins smartPins;
#endif
