#include "calcapp/outfile.hpp"
#include "calcapp/io.hpp"

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

OutFileText::OutFileText(const char * fileName, TFileType fileType, bool append) 
  : m_f(nullptr) 
  , m_fileName(fileName)
  , m_fileType(fileType)
  , m_curLine(0)
{
#ifdef __GLIBCXX__
  FILE* cfile = fopen(fileName, append ? "a" : "w" );
  int posix_fd = fileno(cfile);
#if (_XOPEN_SOURCE >= 600 || _POSIX_C_SOURCE >= 200112L)
  //hint os kernel about writing 
  posix_fadvise(posix_fd, 0, 0, POSIX_FADV_NOREUSE);
#endif
  __gnu_cxx::stdio_filebuf<char> filebuf(posix_fd, std::ios::in);
  m_f.reset(new std::ostream(&filebuf));
#else
  //TODO: on MSVC we can try to use winapi-ish CreateFile
  m_f.reset(new ofstream(fileName, append ? ios_base::app : ios_base::out));
#endif
  if(m_f == nullptr)
    throw IOError(FERR_IO_GENERAL_WRITE, "Error while opening file for writing", m_fileType, m_fileName.c_str(), m_curLine);
}

void OutFileText::printf(const char * fmt, ...) 
{
  va_list va;
  va_start(va, fmt);

  char buf[LINE_BUF_SIZE];
  vsnprintf(buf, LINE_BUF_SIZE, fmt, va);
  va_end(va);

  print(buf);
}

void OutFileText::println(const char * str)
{
  *m_f << str << '\n';
  if ( (*m_f).bad() )
    throw IOError(FERR_IO_GENERAL_WRITE, "Error while writing a file", m_fileType, m_fileName.c_str(), m_curLine);
}

void OutFileText::print(const char * str)
{
  *m_f << str;
  if ( (*m_f).bad() )
    throw IOError(FERR_IO_GENERAL_WRITE, "Error while writing a file", m_fileType, m_fileName.c_str(), m_curLine);
}

void OutFileText::flush()
{
  (*m_f).flush();
}

}
