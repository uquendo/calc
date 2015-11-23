#pragma once
#ifndef _OUTFILE_HPP
#define _OUTFILE_HPP

#include <calcapp/io.hpp>
#include <calcapp/exception.hpp>

#include <ostream>
#include <string>

namespace Calc {

class OutFileText {
protected:
  std::unique_ptr<std::ostream> m_f;
  std::string m_fileName;
  TFileType m_fileType;
  unsigned long m_curLine;
public:
  OutFileText(const char * fileName, TFileType fileType, bool append = false);
  virtual ~OutFileText() {}

  void printf(const char * fmt, ...);

  void println(const char * str);
  inline void println(const std::string& str) { println(str.c_str()); }
  inline void println() { println(""); }

  void print(const char * str);
  inline void print(const std::string& str) { print(str.c_str()); }

  template <class T> std::ostream& operator << (T t) {
    *m_f << t;
    return *m_f;
  }

  void flush();
private:
  // no copying and copy assignment allowed
  OutFileText(const OutFileText&);
  OutFileText(const OutFileText&&);
  OutFileText& operator= (const OutFileText&);
};

}

#endif /* _OUTFILE_HPP */
