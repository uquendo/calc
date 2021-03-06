#include "calcapp/outfile.hpp"
#include "calcapp/io.hpp"
#include "calcapp/exception.hpp"

#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdarg>

#ifdef __GLIBCXX__
#include <fcntl.h>
#include <ext/stdio_filebuf.h>
#endif

#ifdef _MSC_VER
// disable "'fopen': This function or variable may be unsafe. Consider using fopen_s instead."
#pragma warning (disable : 4996)
#endif

namespace Calc {

OutFileText::OutFileText(const char* fileName, TFileType fileType /* = FT_Undefined */, bool append /* = false */ )
  : OutFileText(std::string(fileName), fileType, append)
{}

OutFileText::OutFileText(const std::string& fileName, TFileType fileType /* = FT_Undefined */ , bool append /* = false */ )
  : m_f(nullptr) 
  , m_fileName(fileName)
  , m_fileType(fileType)
  , m_lineNum(0)
{
  //prepare filename and type
  if(m_fileType == FT_Undefined) {
    //try to guess type from file extension
    m_fileType = IOUtil::guessFileTypeByExt(fileName);
  } else {
    //append extension if file format is known
    m_fileName.append(TFileExt[fileType]);
  }
  if(!IOUtil::isOkToWriteFile(m_fileName))
    throw IOError(FERR_IO_GENERAL_WRITE, "Error while opening file for writing", m_fileType, m_fileName.c_str(), m_lineNum);

#ifdef __GLIBCXX__
  FILE* cfile = fopen(m_fileName.c_str(), append ? "a" : "w" );
  int posix_fd = fileno(cfile);
#if (_XOPEN_SOURCE >= 600 || _POSIX_C_SOURCE >= 200112L)
  //hint os kernel about writing 
  posix_fadvise(posix_fd, 0, 0, POSIX_FADV_NOREUSE);
#endif
  __gnu_cxx::stdio_filebuf<char>* filebuf = new __gnu_cxx::stdio_filebuf<char>(posix_fd, append ? std::ios::app : std::ios::out);
  if(filebuf == nullptr)
    throw IOError(FERR_IO_GENERAL_WRITE, "Error while opening file for writing", m_fileType, m_fileName.c_str(), m_lineNum);
  m_f.reset(new std::ostream(filebuf));
#else
  //TODO: on MSVC we can try to use winapi-ish CreateFile
  m_f.reset(new ofstream(m_fileName, append ? std::ios::app : std::ios::out));
#endif
  if(m_f == nullptr)
    throw IOError(FERR_IO_GENERAL_WRITE, "Error while opening file for writing", m_fileType, m_fileName.c_str(), m_lineNum);
}

void OutFileText::printf(const char * fmt, ...) 
{
  va_list va;
  va_start(va, fmt);

  char buf[LINE_BUF_SIZE];
  buf[0]=0;
  vsnprintf(buf, LINE_BUF_SIZE, fmt, va);
  va_end(va);

  print(buf);
}

void OutFileText::println(const char * str)
{
  *(m_f.get()) << str << '\n';
  if ( m_f->bad() )
    throw IOError(FERR_IO_GENERAL_WRITE, "Error while writing a file", m_fileType, m_fileName.c_str(), m_lineNum);
  m_lineNum++;
}

void OutFileText::print(const char * str)
{
  *(m_f.get()) << str;
  if ( m_f->bad() )
    throw IOError(FERR_IO_GENERAL_WRITE, "Error while writing a file", m_fileType, m_fileName.c_str(), m_lineNum);
}

void OutFileText::flush()
{
  m_f->flush();
}

}
