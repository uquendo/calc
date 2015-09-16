#include "calcapp/io.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <cstring>

using namespace std;

namespace Calc {

bool IOUtil::readLine(ifstream& in, char * buf, int bufSize) 
{
	buf[0]=0;
	in.getline(buf, bufSize, '\n');

	// Fast trim
	char * start = buf;
	char * end = start + strlen(start);
	while ( start < end && isspace(* (unsigned char *) start) ) ++start;
	while ( end > start && isspace(* (unsigned char *) (end-1)) ) --end;
	size_t len = end - start;
	memmove(buf, start, len);
	buf[len] = 0;

	return ! in.fail();
}

bool IOUtil::readLine(ifstream& in, string& str)
{
	static char buf[MAX_CH_ST];
	bool r = readLine(in, buf, MAX_CH_ST);
	str.assign(buf);
	return r;
}

bool IOUtil::tryOpenFile(const char * name, const char * mode) {
	FILE *pF;
	if ( (pF=fopen(name, mode)) ) {		
		fclose(pF);
		return true;
	}
    return false;
}

string IOUtil::fileGrep(const char * fileName, const boost::regex& rx, const bool firstOnly) {
    string out;

    ifstream f(fileName);
    while ( ! f.fail() && ! f.eof() ) {
        string line;
        getline(f, line);
        boost::smatch m;
        if ( boost::regex_match(line, m, rx) ) {
            if ( m.size() > 0 && m[1].matched ) {
                out += string(m[1].first, m[1].second);
            } else {
                out += line;
            }

            if ( firstOnly )
                break;
        }
    }

    return out;
}


}
