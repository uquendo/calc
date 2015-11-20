#include "calcapp/infile.hpp"
#include "calcapp/exception.hpp"
#include "calcapp/io.hpp"

#include <cstdio>
#include <cstdarg>
#include <string>
#include <istream>
#include <vector>
#include <memory>
#include <type_traits>

#ifdef __GLIBCXX__
#include <fcntl.h>
#include <ext/stdio_filebuf.h>
#endif

using std::string;
using std::to_string;

#ifdef _MSC_VER
// disable "'fopen': This function or variable may be unsafe. Consider using fopen_s instead."
#pragma warning (disable : 4996)
#endif

namespace Calc {

InFileText::InFileText(const char * fileName, TFileType fileType /* = FT_Undefined */, bool seqAccess /* = false */) 
	: m_f()
	, m_fileName(fileName)
	, m_fileType(fileType)
	, m_lineNum(0)
  , m_seqAccess(seqAccess)
{
	m_line[0] = 0;
#ifdef __GLIBCXX__
  FILE* cfile = fopen(fileName, "r");
  int posix_fd = fileno(cfile);
#if (_XOPEN_SOURCE >= 600 || _POSIX_C_SOURCE >= 200112L)
  //hint os kernel about sequential access
  posix_fadvise(posix_fd, 0, 0, POSIX_FADV_SEQUENTIAL);
#endif
  __gnu_cxx::stdio_filebuf<char> filebuf(posix_fd, std::ios::in);
  m_f.reset(new std::istream(&filebuf));
#else
  //TODO: on MSVC we can try to use winapi-ish CreateFile with FILE_FLAG_SEQUENTIAL_SCAN
  m_f.reset(new ifstream(fileName));
#endif
}

bool InFileText::readNextLineOrEOF()
{
	if ( ! IOUtil::readLine(*m_f, m_line, LINE_BUF_SIZE) )
		return false;

	if ( (*m_f).fail() )
		throwIOError(FERR_IO_GENERAL_READ, "File read error");

	++m_lineNum;
	return true;
}

void InFileText::readNextLine()
{
	if ( ! readNextLineOrEOF() )
		throw PreliminaryEofError("Premature EOF", m_fileType, m_fileName.c_str(), m_lineNum);
}

int InFileText::readNextLine_until(int nStr, ...) 
{
	std::vector<const char *> m;

	va_list va;
	va_start(va, nStr);
	for ( int i = 0; i  < nStr; ++i )
		m.push_back(va_arg(va, const char *));
	va_end(va);

	while ( readNextLineOrEOF() ) {
		for ( int i = 0; i  < nStr; ++i )
			if ( strcmp(m_line, m[i]) == 0 )
				return i;
	}

	string msg("Reached EOF looking for lines: ");
	for ( int i = 0; i  < nStr; ++i ) {
		msg += m[i];
		msg += ", ";
	}
	
	throw PreliminaryEofError(msg.c_str(), m_fileType, m_fileName.c_str(), m_lineNum);

}

int InFileText::readNextLine_expectStr(int nStr, ...)
{
	readNextLine();

	string err = "Expected one of the following lines: ";

	va_list va;
	va_start(va, nStr);
	for ( int i = 0; i  < nStr; ++i ) {
		const char * arg = va_arg(va, const char *);
		if ( strcmp(m_line, arg) == 0 )
			return i;
		err += arg;
		err += ", ";
	}
	va_end(va);

	throwFormatError(FERR_IO_FORMAT_ERROR, err.c_str());
	return 0; // For compiler
}

void InFileText::readNextLine_untilExcept(const char * str, const char * except) 
{
	if ( readNextLine_until(2, str, except) != 0 )
		throwFormatError(FERR_IO_FORMAT_ERROR, string("Expected section ").append(str).append(", but found ").append(except).c_str());
}

template <typename T> int InFileText::readNextLine_scanFloatArray(int minCount, int maxCount, T * dest) 
{
  static_assert(std::is_floating_point<T>::value, "Floating point number expected");
	readNextLine();

	int n = IOUtil::scanArray<T>(m_line, maxCount, dest);
	if ( n < minCount )
		throwIOError(FERR_IO_FORMAT_ERROR, string("Expected ").append(to_string(minCount)).append(" floating point values, got ").append(to_string(n)).c_str());

	return n;
}

template <typename T> int InFileText::readNextLine_scanIntArray(int minCount, int maxCount, T * dest) 
{
  static_assert(std::is_integral<T>::value, "Integer expected");
	readNextLine();

	int n = IOUtil::scanArray<T>(m_line, maxCount, dest);
	if ( n < minCount )
		throwIOError(FERR_IO_FORMAT_ERROR, string("Expected ").append(to_string(minCount)).append(" integer values, got ").append(to_string(n)).c_str());

	return n;
}

template <typename T> int InFileText::readNextLine_scanFloats(int minCount, int maxCount, /* (T *) */...) 
{
  static_assert(std::is_floating_point<T>::value, "Floating point number expected");
	readNextLine();

  std::unique_ptr<T[]> v(new T[maxCount]);
	int n = IOUtil::scanArray<T>(m_line, maxCount, &v[0]);

	va_list va;
	va_start(va, maxCount);
	for ( int i = 0; i < n; ++i )
		*va_arg(va, T *) = v[i];
	va_end(va);

	if ( n < minCount )
		throwIOError(FERR_IO_FORMAT_ERROR, string("Expected ").append(to_string(minCount)).append(" floating point values, got ").append(to_string(n)).c_str());

	return n;
}

template <typename T> int InFileText::readNextLine_scanInts(int minCount, int maxCount, /* (T *) */...) 
{
  static_assert(std::is_integral<T>::value, "Integer expected");
	readNextLine();
  std::unique_ptr<T[]> v(new T[maxCount]);
	int n = IOUtil::scanArray<T>(m_line, maxCount, &v[0]);

	va_list va;
	va_start(va, maxCount);
	for ( int i = 0; i < n; ++i )
		*va_arg(va, T *) = v[i];
	va_end(va);

	if ( n < minCount )
		throwIOError(FERR_IO_FORMAT_ERROR, string("Expected ").append(to_string(minCount)).append(" integer values, got ").append(to_string(n)).c_str());

	return n;
}

#if _MSC_VER
int vsscanf(const char *s, const char *fmt, va_list ap)
{
  void *a[20];
  for (int i=0; i<sizeof(a)/sizeof(a[0]); ++i) a[i] = va_arg(ap, void *);
  return sscanf(s, fmt, a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9], a[10], a[11], a[12], a[13], a[14], a[15], a[16], a[17], a[18], a[19]);
}
#endif

int InFileText::readNextLine_scan(int minCount, const char * format, ...)
{
	va_list va;
	va_start(va, format);
	readNextLine();
	int n = vsscanf(m_line, format, va);
	va_end(va);

	if ( n < minCount )
		throwIOError(FERR_IO_FORMAT_ERROR, string("Expected ").append(to_string(minCount)).append(" values, got ").append(to_string(n)).c_str());

	return n;
}


void InFileText::throwIOError(int code, const char * msg) const
{
	throw IOError(code, msg, m_fileType, m_fileName.c_str(), m_lineNum);
}

void InFileText::throwFormatError(int code, const char * msg) const
{
	throw FileFormatError(code, msg, m_fileType, m_fileName.c_str(), m_lineNum);
}

}

