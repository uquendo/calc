#include "calcapp/io.hpp"

#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

namespace Calc {

namespace IOUtil{

float __atof__(const char * str) { return strtof(str,nullptr); }
double __atod__(const char * str) { return atof(str); }
long double __atold__(const char * str) { return strtold(str,nullptr); }
#ifdef HAVE_QUADMATH
numeric::quad __atoq__(const char * str) { return numeric::quad(str); }
#endif
#ifdef HAVE_MPREAL
numeric::mpreal __atompfr__(const char * str) { return numeric::mpreal(str); }
#endif

bool readLine(istream& in, char * buf, int bufSize) 
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

bool readLine(istream& in, string& str)
{
  char buf[LINE_BUF_SIZE];
  bool r = readLine(in, buf, LINE_BUF_SIZE);
  str.assign(buf);
  return r;
}

bool tryOpenFile(const char * name, const char * mode) {
  FILE *pF;
  if ( (pF=fopen(name, mode)) ) {    
    fclose(pF);
    return true;
  }
    return false;
}

string fileGrep(const char * fileName, const std::regex& rx, const bool firstOnly, const unsigned submatchNumber) {
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

}

