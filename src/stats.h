/*
 MIT License

Copyright (c) 2018 Phil Bowles <esparto8266@gmail.com>

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
#ifndef SP_stats_H
#define SP_stats_H

#include<Ticker.h>
#include "utils.h"

//#define CSTR(x) x.c_str()

using SP_STATS_FN = function<uint32_t(void)>;

struct stat{
	uint32_t	initial;
	uint32_t	limit;
	uint32_t	count;
	uint32_t	instant;
	uint32_t	min;
	uint32_t	max;
};

class	statistic{
	protected:
		string		name;
		stat 		stats={};
	public:
		
		statistic() {}
		
		statistic(string id): name(id) {}
		
		statistic(string id,uint32_t first): statistic(id){
			stats.initial=first;
			stats.min=first;
			record(first);
		}

		statistic(string id,uint32_t first,uint32_t limit): statistic(id,first){				
			stats.limit=limit;
		}
						void            dump();
                        uint32_t        record(uint32_t v);
                        void	        record();
                        uint32_t		getInstant(){ return stats.instant; }		
						uint32_t		tick();												
};

class choke: public statistic, public Ticker {
	uint32_t	budget;
	uint32_t	sample;
	
	static 	void periodReset(uint32_t* me){	reinterpret_cast<choke*>(me)->stats.count=0; }
			
	public:
			void setRate(uint32_t v){
				stats.limit=v;
				budget=stats.limit*sample/1000;
			}

			uint32_t tick(){ return stats.count++ < budget;	}
			
			choke(string id,uint32_t limit,uint32_t _sample): sample(_sample), statistic(id){
				setRate(limit);
				_attach_ms(_sample,true,reinterpret_cast<Ticker::callback_with_arg_t>(choke::periodReset),reinterpret_cast<uint32_t>(this));		
			}	
};
#endif // SP_stats_H