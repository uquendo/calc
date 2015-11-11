#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>


#include "calcapp/io.hpp"

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
	char buf[LINE_BUF_SIZE];
	bool r = readLine(in, buf, LINE_BUF_SIZE);
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

string IOUtil::fileGrep(const char * fileName, const std::regex& rx, const bool firstOnly, const unsigned submatchNumber) {
    string out;

    ifstream f(fileName);
    while ( ! f.fail() && ! f.eof() ) {
        string line;
        getline(f, line);
        std::smatch m;
        if ( std::regex_match(line, m, rx) ) {
            if ( m.size() > submatchNumber ) {
                out += m[submatchNumber].str();
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

