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
#ifndef SPS_H
#define SPS_H

#define CSTR(x) x.c_str()

#include <functional>
#include <FunctionalInterrupt.h>
#include <H4.h>
#include "stats.h"
#include "changelog.h"

using namespace std;
using namespace std::placeholders;

using VFN = function<void(void)>;
using SP_STATE=function<void(int)>;
using SP_STATE_VALUE=function<void(int,int)>;

#define SP_MAX_RATE	500

enum{
	SP_TYPE_CANTUSE,
	SP_TYPE_BOOT,
	SP_TYPE_RX,
	SP_TYPE_TX,
	SP_TYPE_GPIO,
	SP_TYPE_WAKE,
	SP_TYPE_ADC,
	SP_TYPE_LD,
	SP_TYPE_BN,
	SP_TYPE_RY,
	SP_TYPE_NONAME
};

enum{
	SP_STYLE_UNUSED,
	SP_STYLE_OUTPUT,
	SP_STYLE_RAW,
	SP_STYLE_DEBOUNCED,
	SP_STYLE_FILTERED,
	SP_STYLE_LATCHING,
	SP_STYLE_RETRIGGERING,
	SP_STYLE_ENCODER,
	SP_STYLE_ENCODER_AUTO,
	SP_STYLE_REPORTING,
	SP_STYLE_TIMED,
	SP_STYLE_POLLED,
	SP_STYLE_INTERRUPT,
	SP_STYLE_3STAGE
};

struct pinInt{
	uint8_t			pin;
	uint8_t			state;
	uint8_t			imode;
	uint32_t		at;
	SP_STATE_VALUE	f;
};

#define SP_RAW		0
#define SP_COOKED 	1

#define SP_N_PINS	17
#define SP_MAX_PIN	SP_N_PINS + 1

class SmartPins: public H4 {
		static SP_STATE_VALUE	rawHookFn,cookedHookFn,throttleHookFn;
		
		static	pinInt async;
		
	protected:
		
		class hwPin;
		
		struct SP_PIN {
			uint8_t 						D;
			uint8_t 						type;
			hwPin*							h;
		};
		static SP_PIN spPins[];
//
//	hwPin - virtual base class
//
		class hwPin {					
				bool				throttled=false;
			protected:
				choke*				ps=nullptr;
				int					style;
				bool				active=HIGH;
				uint32_t        	last=0;

				virtual void 		raw(int v){}
				virtual void 		prepared(int v){}

			public:
				virtual void 		cooked(int v){}
				
				int8_t         		p; // lazy: allow accesss to encoder pinB stuff
				uint32_t        	state;
									// root
									void root(uint8_t _p,uint8_t mode,int _style){
										 p=_p;
										 style=_style;
										 ::pinMode(_p,mode);
									}
									// input
									hwPin(uint8_t _p,uint8_t mode,int _style){
										root( _p, mode, _style);
										state=digitalRead(p);
										active=!state;
									}
									// output
									hwPin(uint8_t _p,bool _active,uint8_t initial){
										root( _p, OUTPUT, SP_STYLE_OUTPUT);
										active=_active;
										state=initial;
										::digitalWrite(_p,initial); // avoid first time "not-readies...?" i'd prefer purity
									}
									
						bool		isThrottled(){ return ps ? true:false;	}
						
						int 		getActive(){ return active; }

				virtual int 		getStyle(){ return style; }
				
				virtual int 		getValue(){ return state; }
							 
				virtual void 		reconfigure(int v1,int v2){};

				virtual void 		run(){
					
						uint8_t	 	instant=digitalRead(p);						
							if(instant!=state){								
								if(ps && !ps->tick()){
//									if(!throttled) DIAG("CHOKE %s inst=%d\n",CSTR(ps->getName()),ps->getInstant());
//									throttled=true;
//									throttleHookFn(p,1);
									}
								else {
//									if(throttled) DIAG("UNCHOKE %s inst=%d\n",CSTR(ps->getName()),ps->getInstant());
//									throttled=false;
//									throttleHookFn(p,0);
									state=instant;
									if(rawHookFn) rawHookFn(p,instant);
									raw(instant);
									last=micros();
								}
							}
						} 
						
						void		setStyle(int s){ style=s; } // needed for encoders
						
						void		setThrottle(uint32_t lim){
							if(!ps) ps=new choke(CSTR(string("pin"+stringFromInt(p))),lim,100);
							else ps->setRate(lim);
							}
				
			};
						
	public:
//
//
//	smartPin - base class templated by callback type ( SP_STATE || SP_STATE_VALUE)
//		
			template<typename T>
			class smartPin: public hwPin {
				protected:
					T              callback;
				public:
					smartPin(uint8_t _p,uint8_t _mode,T _callback,int _style): hwPin(_p,_mode,_style) {
						callback=move(_callback);
						}
					virtual void prepared(int v){
						if(cookedHookFn) cookedHookFn(p,v);
						cooked(v);
					}

			};
//
//		spOutput bit of a special case...
//
			class spOutput: public hwPin {
					SP_STATE	onChange;
				
					int 		getValue(){ return state; }
					void 		run(){}				
				public:
								spOutput(uint8_t p,bool active,uint8_t initial,SP_STATE callback=nullptr): onChange(callback), hwPin( p, active, initial){}
					void		cooked(int s){
						state=s;
						if(onChange) onChange(s);
						}
		};
//
//	spRaw - feeds all transitions to caller
//		
			class spRaw: public smartPin<SP_STATE>{
					void raw(int v){ callback(v); }
					void reconfigure(int v1,int v2){}
				public:
					spRaw(uint8_t _p,uint8_t mode,SP_STATE _callback): smartPin(_p,mode,_callback,SP_STYLE_RAW){}    
			};
			
			class spFiltered: public smartPin<SP_STATE> {
					bool filter;
					void raw(int v){
						if(v==filter) {
							static uint32_t prev;						
							callback(v);
							prev=last;
						}
					}
					void reconfigure(int v1,int v2){}
				public:
					spFiltered(uint8_t _p,uint8_t _mode,bool _filter,SP_STATE _callback): filter(_filter), smartPin(_p,_mode,_callback,SP_STYLE_FILTERED){}    
			};
//
//	spInterrupt
//		Here's tricky: we have to serialise the int event as fast as possible and "stack em up" in the queue
//		"raw" functional interrupt comes to us with no params, so we must capture state / time
//		to pass to callback then set up background job let callback do whatever...
//		so that we can get out a.s.a.p.
//
//		Hence spiSetup (function<void(void)> as required by FunctionalInterrupt) had a complex syntax:
//			its a bound function within a bound function - both requiring the current context (and outer context cos we're a nested class)
//			to be passed in. Toy with it at your peril...
//
//		ALSO:
//			Some devices will signal too fast for us to keep up (we are adding latency here...) so we "cheat"
//			We can only do this when interrupt mode is CHANGE. Both RISING and FALLING modes will produce stream
//			of identical states, so it is impossible to spot a missing one.
//			CHANGE mode should produce 1010101... or 0101010...depending on whether it starts HI or LO
//			so if we see: 1010010...
// 		    we know that here-^ we have missed a 1 so we cheat by synthesising (or "making up")
//	           an interrupt 1 timed at half way between the two tell-tale 0s
//
			class spInterrupt: public smartPin<SP_STATE_VALUE>{
					
					static void ICACHE_RAM_ATTR spiSetup(uint8_t _p,SP_STATE_VALUE f,uint8_t imode){
						async.state=digitalRead(_p);
						async.at=micros();
						async.pin=_p;
						async.imode=imode;
						async.f=f;
					}
						
					void spiHandler(uint8_t p,uint8_t s,uint32_t m,SP_STATE_VALUE callback,uint8_t imode){
//						DIAG("spiHandler pin %d state=%d u=%d e=%d\n",p,s,m,e);
						static	uint8_t 	prev=!active; // state..
						static	uint32_t	then=0;					// ..and time of previous interrupt
								uint32_t	now=m;
								uint32_t	delta=now-then;	// uSec since previous	
								
						if(imode==CHANGE && s==prev){
							callback(!s,delta/2); // "fake" missing int TODO parameterise fake function! e.g. rand(then,now)
							if(rawHookFn) rawHookFn(p,!s);
						}
						prev=s;
						then=now; // use earlier value to minimise "drift"
						if(rawHookFn) rawHookFn(p,s);
						callback(s,delta);	
					}
				public:
					spInterrupt(uint8_t _p,uint8_t _mode,int imode,SP_STATE_VALUE _callback,bool _active=HIGH): smartPin(_p,_mode,_callback,SP_STYLE_INTERRUPT) {
						active=_active;
						attachInterrupt(_p,bind(spiSetup,_p,_callback,imode),imode);
					}
					
					void run(){						    
						noInterrupts();
						pinInt copy=async;
						async.f=nullptr;
						interrupts();						
						if(copy.f) spiHandler(copy.pin,copy.state,copy.at,copy.f,copy.imode);
					} 	
			};

//
//	spPolled - reports state every N milliseconds
//
			class spPolled: public smartPin<SP_STATE>{
					bool	adc;
					void raw(int v){ }
					void cooked(int v){	callback(v); }
					uint32_t 	lastpoll;
					H4_TIMER	timer;
					void setTimer(uint32_t freq){
						timer=every(freq,bind([this](){
									uint32_t instant;
									instant=adc ? analogRead(p):digitalRead(p);
									if(lastpoll!=instant){
										lastpoll=instant;
										prepared(instant);
									}
								}));  		
					}
				public:
					int getValue(){ return static_cast<int>(lastpoll); }
					void reconfigure(int v1, int v2){
						cancel(timer);
						setTimer(v1);
					}
					spPolled(uint8_t _p,uint8_t mode,uint32_t freq,SP_STATE _callback,bool _adc): smartPin(_p,mode,_callback,SP_STYLE_POLLED) {
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
							once(debounce,bind([this](){
									bouncing=false;
									if(smartPin<T>::state==savedState) { prepared(smartPin<T>::state);	}
							}));
						}
					}
					
				protected:
					uint32_t    debounce;
					virtual void prepared(int v)=0;
					void reconfigure(int v1, int v2=0){	debounce=v1; }

				public:
					_deSpike(uint8_t _p,uint8_t mode,uint32_t _debounce,T _callback, uint8_t _style): smartPin<T>(_p,mode,_callback,_style) {debounce=_debounce;}
			};
//
//	spDebounce - straightforward debounced input pin
//
			class spDebounce: public _deSpike<SP_STATE> {
					void cooked(int v){ callback(v); }	
					void prepared(int v){ smartPin<SP_STATE>::prepared(v); }
				public:
					spDebounce(uint8_t _p,uint8_t mode,uint32_t _debounce,SP_STATE _callback): _deSpike<SP_STATE>(_p,mode,_debounce,_callback,SP_STYLE_DEBOUNCED){}    
			};
//
//	spTimed - reports time (in ms) that pin was held for
//
			class spTimed: public _deSpike<SP_STATE_VALUE>{
						uint32_t     startTime=0;
						void cooked(int v){ callback(state,v); }
						void prepared(int v){
							if(startTime){
								smartPin<SP_STATE_VALUE>::prepared((last-startTime)/1000);
								startTime=0;
								}
							else{
								smartPin<SP_STATE_VALUE>::prepared(state);
								startTime=last;
							}
						}
				public:
					spTimed(uint8_t _p,uint8_t mode,uint32_t _debounce,SP_STATE_VALUE _callback): _deSpike<SP_STATE_VALUE>(_p,mode,_debounce,_callback,SP_STYLE_TIMED) {}    
			};
//
//	spReporting - as for timed, but also reports intervening whole periods of f
//				e.g. if f=1000 and pin is held for 3.25sconds, will report 1000, 2000, 3000 and 3250
//
			class spReporting: public _deSpike<SP_STATE_VALUE>{
						uint32_t     startTime=0;
						uint32_t     freq;
						uint32_t     fincr=0;
						H4_TIMER     timer;
						void cooked(int v){ callback(state,v); }
						void prepared(int hilo){
							if(startTime){
								cancel(timer);
								smartPin<SP_STATE_VALUE>::prepared((last-startTime)/1000);
								startTime=0;
								fincr=0;
							}
							else{
								smartPin<SP_STATE_VALUE>::prepared(state);
								startTime=last;
								timer=every(freq,bind([this](){
									fincr+=freq;
									callback(state,fincr);
									}));      
							}
						}
				public:
					void reconfigure(int v1, int v2){
						_deSpike<SP_STATE_VALUE>::reconfigure(v1);
						freq=v2;
					} 
					spReporting(uint8_t _p,uint8_t mode,uint32_t _debounce,uint32_t f,SP_STATE_VALUE _callback): freq(f),_deSpike(_p,mode,_debounce,_callback,SP_STYLE_REPORTING) {}    
			};
//
//	spThreeStage - short, medium long press - different callback for each
//			
			class spThreeStage: public spReporting{
					uint32_t	m,l;
					SP_STATE	fns[3];
					SP_STATE	progress;
					uint32_t	esc=0;
					
				public:
					void handler(int s,int t){
						uint32_t stage=t > l ? 2:(t > m ? 1:0);						
						if(s==active) {
							if(stage > esc) {
								progress(stage);
								esc=stage;
							}
						}
						else {
							esc=0;
							fns[stage](s);
						}
					}
					
					spThreeStage(uint8_t _p,uint8_t mode,uint32_t _debounce,uint32_t f,SP_STATE _progress,
								 uint32_t _m,uint32_t _l,
								 SP_STATE _sf,SP_STATE _mf,SP_STATE _lf):
									progress(_progress),
									m(_m),l(_l),				
									spReporting( _p, mode, _debounce, f,bind(&spThreeStage::handler,this,_1,_2)) {
				
						style=SP_STYLE_3STAGE;	
															
						fns[0]=_sf;
						fns[1]=_mf;
						fns[2]=_lf;														
					}    
	
		};
//
//	spLatching - transforms momentary transition to permanent state
//				 e.g. one up/down (or down/up) sets latched=true, second up/down (etc..) set latched=false
//				 NB always starts unlatched, i.e. latched=false
//
		class spLatching: public _deSpike<SP_STATE>{
					uint8_t     count=0;
					bool        latched;
					
				public:
					int getValue(){ return static_cast<int>(latched); }
					spLatching(uint8_t _p,uint8_t mode,uint32_t _debounce,SP_STATE _callback): _deSpike<SP_STATE>(_p,mode,_debounce,_callback,SP_STYLE_LATCHING) {
						latched=state;
					}
					void cooked(int v) { callback(v); }
					void prepared(int v){
						if(++count > 1){
							latched=!latched;
							count=0;
							smartPin<SP_STATE>::prepared(latched);
						}
					}
			};
//
//	spRetriggering - 1st transition starts timer. 2nd transition reported only after delay of N ms.
//					 Intervening transitions restart timer just like e.g. a PIR sensor
//
			class spRetriggering: public smartPin<SP_STATE>{
								uint32_t    timeout;
								H4_TIMER    timer=0;
					
					virtual void cooked(int hilo){ callback(hilo);	}
			
					void raw(int hilo){
						if(hilo==active){
							if(timer) {
								cancel(timer);
								timer=0;
							}
							prepared(hilo);
							timer=once(timeout,bind([this](){
									timer=0;
									if(state!=active) prepared(state);
									}));
						}
						else if(!timer) prepared(hilo); // inactive after timer expiry
					}
				public:
					int getValue(){ return static_cast<int>(timer!=0); }
					void reconfigure(int v1, int v2) {	timeout=v1;	}
					spRetriggering(uint8_t _p,uint8_t mode,uint32_t _timeout,SP_STATE _callback,bool _active): smartPin<SP_STATE>(_p,mode,_callback,SP_STYLE_RETRIGGERING) {
						active=_active;
						timeout=_timeout;
						if(state==_active){
//							DIAG("RT already triggered at startup pin %d active=%s state=%d\n",_p,_active ? "HI":"LO",state);
							raw(_active); // already triggered at startup
						}
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
						if(cookedHookFn) cookedHookFn(pinB->p,v);
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
					spEncoder(uint8_t _pA,uint8_t _pB,uint8_t _mode,SP_STATE _callback): spRaw(_pA,_mode,_callback) {						
						spPins[_pB].h=pinB=new spRaw(_pB,_mode,bind(&spEncoder::raw,this,_1));
						pinB->setStyle(style=SP_STYLE_ENCODER);
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
					
					void _reconfigure(int _Vmin,int _Vmax,int _Vinc,int _Vset=0){
						Vmin=_Vmin;
						Vmax=_Vmax;
						Vinc=_Vinc;	
					}
					
					int	 _middle(){ return (Vmin+Vmax)/2; }
					
					void prepared(int v){ setValue(value+(Vinc*v));	}
					
				public:
					int getValue(){ return value; }
					
					void reconfigure(int _Vmin,int _Vmax,int _Vinc,int _Vset=0){
						_reconfigure( _Vmin, _Vmax, _Vinc, _Vset);
						if(_Vset) setValue(_Vset);
						else center();
					}
					
					spEncoderAuto(uint8_t _pA,uint8_t _pB,uint8_t _mode,SP_STATE _callback,int _Vmin=0,int _Vmax=100,int _Vinc=1,int _Vset=0): spEncoder(_pA,_pB,_mode,_callback) {
						pinB->setStyle(style=SP_STYLE_ENCODER_AUTO);
						_reconfigure(_Vmin,_Vmax,_Vinc,_Vset);
						value=_Vset ? constrain(_Vset,Vmin,Vmax):_middle();
					}
				
					void setValue(int v){
						value=constrain(v,Vmin,Vmax);
						if(value!=prev){
							prev=value;
							cooked(value);
						}
					}
					
					void setMin(){ setValue(Vmin);	}
					
					void setMax(){ setValue(Vmax);	}
					
					void setPercent(uint32_t pc){ setValue((Vmax-Vmin)*pc/100); }
					
					void center(){ setValue(_middle()); }

			};
			
		class flasher{
			bool 		_active;
			
			H4_TIMER	_timer=NULL;
			H4_TIMER	_off=NULL;
			
			string		_pattern;
			uint8_t		_len;
			uint8_t		i=0;
			int			_timebase=0;
			
			void _toggle(){	digitalWrite(_pin,!digitalRead(_pin)); }
			
			void _pulse(int width,VFN fn=[]{}){
				_toggle();
				_off=once(width,[this](){ _toggle(); },fn);				
			}
			
			void _cycle(){
				uint8_t	_dc;	
				i=i < _len ? ++i:0;
				switch(_pattern[i]){
					case '.':
						_dc=5;
						break;
					case '-':
						_dc=60;
						break;
					case ' ':
						_dc=0;		// pause
						break;
					default:
						_dc=50;	
				}
				if(_dc){
					int width=_timebase/(100/_dc);
					_pulse(width);
				}
			}
			
		public:
			uint8_t 	_pin; // really?
						
			void _PWM(int period,int duty,VFN fn=[]{}){
				// special case - when duty=0, one single pulse occurs see pulseLED
				if(duty){
					_stop();
					int width=period/(100/duty);
					_pulse(width); // start immediately
					_timer=every(period,bind([this](int width){ _pulse(width); },width)); //run the timebase
				}
				else _pulse(period,fn); // run once and execute callback so pulseLED can kill object
			}
			
			flasher(uint8_t pin,int period,uint8_t duty,VFN fn=[]{}): _pin(pin){
				_active=getPinActive(pin);
				_PWM(period,duty,fn);
				}
			
			void flashPattern(int timebase,const char* pattern){
				_stop();
				_timebase=timebase;
				// stretch the dashes...
				String pat=pattern;
				pat.replace("-","- ");
				_pattern=CSTR(pat);
				_len=_pattern.size()-1;
				i=0;
				_timer=every(_timebase,[this](){ _cycle(); }); //run the timebase				
			}
			
			flasher(uint8_t pin,const char* pattern,int timebase): _pin(pin){
				_active=getPinActive(pin);
				flashPattern(timebase,pattern);
				}
			
			virtual ~flasher(){}
						
			bool _isFlashing(){	return _timer; }
			
			void _stop(){
				cancel(_timer);
				_timer=NULL;				// for isFlashing. no need to null _off as not used elsewhere
				cancel(_off);
				digitalWrite(_pin,!_active);
			}
		};

	using FLASHER_HANDLER = function<bool(flasher*)>;	
	private:
		static vector<flasher*>	fList;
		
		static bool _doFlasher(uint8_t pin,FLASHER_HANDLER fn);
		static void _flash(int period,int duty,uint8_t pin=LED_BUILTIN,VFN fn=[]{});	
//
//		flasher functions
//
	public:	
		SmartPins(
				  uint32_t nSlots=H4_Q_CAPACITY,
				  uint32_t hWarn=H4_H_WARN_PCENT,
				  SP_STATE_VALUE _cookedHook=nullptr,
				  SP_STATE_VALUE _rawHook=nullptr,		  
				  SP_STATE_VALUE _chokeHook=nullptr				  
				  ): H4(nSlots,hWarn){
			
			cookedHookFn=_cookedHook;
			rawHookFn=_rawHook;
			throttleHookFn=_chokeHook;		
		}
		
		~SmartPins(){}
//
//		these have to be public, but DON'T EVER CALL THEM!
//
		static void loop();
//
//		Call these if you must, but its usually an indication you are doing something tricky, so make sure
//		you know what you are doing!
//
		static uint8_t	 	getDno(uint8_t i){ return spPins[i].D; }
		static bool			getPinActive(uint8_t i){
			if(hwPin* h=isSmartPin(i)) return h->getActive();
			return false;
		}
		
		static int			getStyle(uint8_t i){
			if(hwPin* h=isSmartPin(i)) return h->getStyle();	
			return 0;
		}
		
		static uint8_t	 	getType(uint8_t i){ return spPins[i].type; }		
		
		static hwPin* 		isOutputPin(uint8_t i){
			hwPin* h=isSmartPin(i);
			if(h && h->getStyle()==SP_STYLE_OUTPUT) return h;
			return nullptr;
			}				
		
		static hwPin*		isSmartPin(uint8_t i){ return isUsablePin(i) ? spPins[i].h:nullptr; }
		
		static bool			isThrottledPin(uint8_t i){
			if(hwPin* h=isSmartPin(i)) return h->isThrottled();
			return false;
			}
			
		static bool 		isUsablePin(uint8_t i){ return isValidPin(i) &&  spPins[i].type!=SP_TYPE_CANTUSE; }
		
		static bool 		isValidPin(uint8_t i){ return i < SP_MAX_PIN ? :false; }
//
//		THESE are the ones you should be calling:
//		
		static 	void 		digitalWrite(uint8_t pin,uint8_t value);
		static 	int	 		getValue(uint8_t _p);
		static 	void 		reconfigurePin(uint8_t _p,int v1, int v2=0);
		static	void 		throttlePin(uint8_t _p,uint32_t lim);
//
		static 	void 		flashLED(int period,int duty,uint8_t pin=LED_BUILTIN);	
		static 	void 		flashLED(int rate,uint8_t pin=LED_BUILTIN);	
		static 	void 		flashLED(const char * pattern,int timebase,uint8_t pin=LED_BUILTIN);
		static 	bool 		isFlashing(uint8_t pin=LED_BUILTIN);
		static 	void 		pulseLED(int period,uint8_t pin=LED_BUILTIN);
		static 	void 		stopLED(uint8_t pin=LED_BUILTIN);			
//		
//		the pins
//
		static void 		Debounced(uint8_t _p,uint8_t _mode,uint32_t _debounce,SP_STATE _callback);
		static void 		Encoder(uint8_t _pA,uint8_t _pB,uint8_t mode,SP_STATE _callback);	
		static void 		Encoder(uint8_t _pA,uint8_t _pB,uint8_t mode,int * pV);	
		static SmartPins::spEncoderAuto* EncoderAuto(uint8_t _pin,uint8_t _pinB,uint8_t _mode,SP_STATE _callback,int _Vmin=0,int _Vmax=100,int _Vinc=1,int _Vset=0);	
		static SmartPins::spEncoderAuto* EncoderAuto(uint8_t _pin,uint8_t _pinB,uint8_t _mode,int * pV,int _Vmin=0,int _Vmax=100,int _Vinc=1,int _Vset=0);	
		static void 		Filtered(uint8_t _p,uint8_t _mode,bool _filter,SP_STATE _callback);
		static void 		Interrupt(uint8_t _p,uint8_t _mode,int imode,SP_STATE_VALUE _callback,bool active=HIGH);	
		static void 		Latching(uint8_t _p,uint8_t _mode,uint32_t _debounce,SP_STATE _callback);
		static void 		Output(uint8_t _p,bool active=HIGH,uint8_t initial=LOW,SP_STATE _callback=nullptr);		
		static void 		Polled(uint8_t _p,uint8_t _mode,uint32_t freq,SP_STATE _callback,bool adc=false);
		static void 		Raw(uint8_t _p,uint8_t _mode,SP_STATE _callback);
		static void 		Reporting(uint8_t _p,uint8_t mode,uint32_t _debounce,uint32_t _freq,SP_STATE_VALUE _callback);	
		static void 		Retriggering(uint8_t _p,uint8_t _mode,uint32_t _timeout,SP_STATE _callback,bool active=HIGH);
		static void			ThreeStage(uint8_t _p,uint8_t mode,uint32_t _debounce,uint32_t f,SP_STATE _callback,SP_STATE _sf,uint32_t _m,SP_STATE _mf,uint32_t _l,SP_STATE _lf);
		static void 		Timed(uint8_t _p,uint8_t mode,uint32_t _debounce,SP_STATE_VALUE _callback);
};

typedef SmartPins::spEncoderAuto*	SP_ENC_AUTO;
#endif
