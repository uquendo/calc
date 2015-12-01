#include "calcapp/io.hpp"

#include <cstdio>
#include <cstring>
#include <cctype>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

#if defined(MSC_VER)
#define HAVE_UNLINK
#elif defined(__linux__) || defined(__APPLE__) || defined(_POSIX_C_SOURCE) || HAVE_POSIX_UNISTD_H
#include "unistd.h"
#define HAVE_UNLINK
#endif

using std::string;

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

bool readLine(std::istream& in, char * buf, int bufSize) 
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

bool readLine(std::istream& in, string& str)
{
  char buf[LINE_BUF_SIZE];
  bool r = readLine(in, buf, LINE_BUF_SIZE);
  str.assign(buf);
  return r;
}

bool tryOpenFile(const char * name, const char * mode) {
  FILE *pF;
  if ( ( pF = fopen(name, mode) ) ) {
    fclose(pF);
    return true;
  }
    return false;
}

bool isOkToReadFile(const string& fileName)
{
  // check filename
  if ( fileName.length() == 0 )
    return false;

  // check existense and access rights:
  return tryOpenFile(fileName.c_str(), "r");
}

bool isOkToWriteFile(const string& fileName)
{
  // check filename
  if ( fileName.length() == 0 )
    return false;

  // check existense and access rights:
  const char * nm = fileName.c_str();
  if ( tryOpenFile(nm, "r") )
    return tryOpenFile(nm, "a");

#ifdef HAVE_UNLINK
  // check that we can open file for writing
  if ( tryOpenFile(nm, "w") ) {
    unlink(nm);
    return true;
  }
#endif

  return false;
}

//get filename extension converted to lower case
string getFileExt(const string& fileName)
{
  if ( fileName.length() < 2 ) 
    return "";

  //finding last dot
  string::size_type pp = fileName.find_last_of('.');
  if ( pp == string::npos )
    return "";

  //getting extension string
  string ext = fileName.substr(pp);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  return ext;
}

TFileType guessFileTypeByExt(const string& fileName)
{
    string ext = getFileExt(fileName);

    for ( int i = 0; i < FT_Count; ++i ) {
      if ( ext == TFileExt[i] )
        return (TFileType) i;
    }

    return FT_Undefined;
}


string fileGrep(const std::string& fileName, const std::regex& rx, const bool firstOnly, const unsigned submatchNumber) {
    string out;

    std::ifstream f(fileName.c_str());
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

