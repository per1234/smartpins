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
#include "utils.h"
//
// readSPIFFS
// 	String etc modes fail in async mode due to yield() in wrong context, hence the reinvented wheel here
//
String readSPIFFS(const char* fn){
	String rv;
	File f=SPIFFS.open(fn, "r");
	if(f) {
		int n=f.size();
		char* buff=(char *) malloc(n+1);
		f.readBytes(buff,n);
		buff[n]='\0';
		rv=buff;
		free(buff);
	}
	f.close();
	return rv;	
}
//
// split
//
void split(const string& s, char delim,vector<string>& v) {
	auto i = 0;
    auto pos = s.find(delim);
    while (pos != string::npos) {
      v.push_back(s.substr(i, pos-i));
      i = ++pos;
      pos = s.find(delim, pos);
      if (pos == string::npos) v.push_back(s.substr(i, s.length()));
    }
}
//
// join
//
string join(vector<string> vs,char delim) {
	string rv;
	for(auto const& v:vs) rv+=v+delim;
	while(rv.back()==delim)	rv.pop_back();
	return rv;
}
//
//	stringFromInt
//
string stringFromInt(int i,const char* fmt){
	char buf[16];
	sprintf(buf,fmt,i);
	return string(buf);
}
//
//	writeSPIFFS
//
void writeSPIFFS(const char* fn,const char* data){
	File b=SPIFFS.open(fn, "w");
	b.print(data);
	b.close();  
}