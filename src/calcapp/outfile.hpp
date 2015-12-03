#pragma once
#ifndef _OUTFILE_HPP
#define _OUTFILE_HPP

#include <calcapp/io.hpp>
#include <calcapp/exception.hpp>

#include <ios>
#include <iomanip>
#include <ostream>
#include <string>

namespace Calc {

class OutFileText {
protected:
  std::unique_ptr<std::ostream> m_f;
  std::string m_fileName;
  TFileType m_fileType;
  unsigned long m_lineNum;
public:
  OutFileText(const std::string& fileName, TFileType fileType = FT_Undefined, bool append = false);
  OutFileText(const char * fileName, TFileType fileType = FT_Undefined, bool append = false);
  virtual ~OutFileText() {}

  inline TFileType fileType() const { return m_fileType; }
  inline const std::string fileName() const { return m_fileName; }
  inline unsigned long lineNum() const { return m_lineNum; }
  inline bool eof() const { return m_f->eof(); }

  void printf(const char * fmt, ...);

  void println(const char * str);
  inline void println(const std::string& str) { println(str.c_str()); }
  inline void println() { println(""); }
  template<typename T> int println_printNumArray(const int count, T * const data, const int stride=1, const int digits=6, const char delim = ' ');

  void print(const char * str);
  inline void print(const std::string& str) { print(str.c_str()); }

  template <class T> std::ostream& operator << (T t) {
    *(m_f.get()) << t;
    return *(m_f.get());
  }

  void flush();
private:
  // no copying and copy assignment allowed
  OutFileText(const OutFileText&);
  OutFileText(const OutFileText&&);
  OutFileText& operator= (const OutFileText&);
};

template<typename T> int OutFileText::println_printNumArray(const int count, T * const data, const int stride, const int digits, const char delim)
{
  static const size_t PRINTF_FLOAT_OVERHEAD = 10;
  const size_t bufSize = digits + PRINTF_FLOAT_OVERHEAD;
  char * buf = new char[bufSize];
  buf[0] = 0;
  size_t wrote = 0;
//  *(m_f.get()) << std::ios::scientific << std::setprecision(digits);
  for(int i=0; i < count; i++)
  {
//    *(m_f.get()) << data[i*stride] << delim;
    if( IOUtil::printNumber(buf, bufSize, data[i*stride], digits) )
    {
      //TODO: setup assert about small buffer
      *(m_f.get()) << buf << delim;
      wrote++;
    }
  }
  println();
  return wrote;
}

}

#endif /* _OUTFILE_HPP */
