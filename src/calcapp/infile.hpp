#pragma once
#ifndef _INFILE_HPP
#define _INFILE_HPP
#include "config.h"

#include <string>
#include <istream>
#include <fstream>
#include <cstring>
#include <memory>

#include "calcapp/io.hpp"
#include "calcapp/exception.hpp"

namespace Calc {

class InFileText {
protected:
  std::unique_ptr<std::istream> m_f;
  std::string m_fileName;
  TFileType m_fileType;
  unsigned long m_lineNum;
  char m_line[LINE_BUF_SIZE];
  const bool m_seqAccess; //can be used to optimize sequential file reading
public:
  InFileText(const char * fileName, TFileType fileType = FT_Undefined, bool seqAccess = false);
  virtual ~InFileText() {}

  inline const char * line() const { return m_line; }
  inline unsigned long lineNum() const { return m_lineNum; }
  inline std::streamoff filePos() { return (*m_f).tellg(); }
  inline bool eof() const { return (*m_f).eof(); }
  inline bool canSeek() { return !m_seqAccess; }
  inline bool seek(std::streamoff offset, unsigned long lineNum) {
    if(canSeek())
    {
      (*m_f).seekg(offset, (*m_f).beg);
      m_lineNum = lineNum;
    };
    return canSeek();
  }

  bool readNextLineOrEOF();
  void readNextLine();

  int readNextLine_until(int nStr, ...);
  inline void readNextLine_until(const char * str) { readNextLine_until(1, str); }
  void readNextLine_untilExcept(const char * str, const char * except);

  int readNextLine_expectStr(int nStr, ...);
  
  // for filling in array of numbers 
  template<typename T> int readNextLine_scanNumArray(const int minCount, const int maxCount, T * const data, const int stride=1);

  // for lines with small number of individual parameters
  template<typename T> int readNextLine_scanNums(const int minCount, const int maxCount, /* T * dataN */ ...);
  int readNextLine_scan(int minCount, const char * format, ...);

  inline bool currentLineIs(const char * str) const { return strcmp(m_line, str) == 0; }

  void throwIOError(int code, const char * msg) const;
  void throwFormatError(int code, const char * msg) const;
  inline void throwDataError(const char * msg) const { throwFormatError(FERR_DATA_ERROR, msg); }

private:
  // no copying and copy assignment allowed
  InFileText(const InFileText&);
  InFileText(const InFileText&&);
  InFileText& operator= (const InFileText&);
};  

}

#endif /* _INFILE_HPP */
