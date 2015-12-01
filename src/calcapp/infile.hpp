#pragma once
#ifndef _INFILE_HPP
#define _INFILE_HPP
#include "config.h"

#include <string>
#include <istream>
#include <fstream>
#include <cstring>
#include <cstdarg>
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
  InFileText(const std::string& fileName, TFileType fileType = FT_Undefined, bool seqAccess = false);
  InFileText(const char * fileName, TFileType fileType = FT_Undefined, bool seqAccess = false);
  virtual ~InFileText() {}

  inline TFileType fileType() const { return m_fileType; }
  inline const std::string fileName() const { return m_fileName; }
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
  inline bool currentLineStartsWith(const char * str) const {
    size_t prefix_len=strlen(str);
    return ( strlen(m_line) < prefix_len ? false : strncmp(m_line, str, prefix_len) == 0 );
  }

  void throwIOError(int code, const char * msg) const;
  void throwFormatError(int code, const char * msg) const;
  inline void throwDataError(const char * msg) const { throwFormatError(FERR_DATA_ERROR, msg); }

private:
  // no copying and copy assignment allowed
  InFileText(const InFileText&);
  InFileText(const InFileText&&);
  InFileText& operator= (const InFileText&);
};

template <typename T> int InFileText::readNextLine_scanNumArray(const int minCount, const int maxCount, T * const dest, const int stride /* = 1 */)
{
//  static_assert(std::is_arithmetic<T>::value, "Arithmetical type expected"); //TODO: check boost::multiprecision type traits
  const std::string s = ( std::is_integral<T>::value ? " integer values, got " : " floating point values, got " );

  readNextLine();

  int n = (stride > 1 ? IOUtil::scanArray<T>(m_line, maxCount, dest, stride) : IOUtil::scanArray<T>(m_line, maxCount, dest) );
  if ( n < minCount )
    throwIOError(FERR_IO_FORMAT_ERROR, std::string("Expected at least ").append(std::to_string(minCount)).append(s).append(std::to_string(n)).c_str());

  return n;
}

template <typename T> int InFileText::readNextLine_scanNums(const int minCount, const int maxCount, /* (T *) */...) 
{
//  static_assert(std::is_arithmetic<T>::value, "Arithmetical type expected"); //TODO: check boost::multiprecision type traits
  const std::string s = ( std::is_integral<T>::value ? " integer values, got " : " floating point values, got " );

  readNextLine();

  std::unique_ptr<T[]> v(new T[maxCount]);
  int n = IOUtil::scanArray<T>(m_line, maxCount, &v[0]);

  va_list va;
  va_start(va, maxCount);
  for ( int i = 0; i < n; ++i )
    *va_arg(va, T *) = v[i];
  va_end(va);

  if ( n < minCount )
    throwIOError(FERR_IO_FORMAT_ERROR, std::string("Expected at least ").append(std::to_string(minCount)).append(s).append(std::to_string(n)).c_str());

  return n;
}


}

#endif /* _INFILE_HPP */
