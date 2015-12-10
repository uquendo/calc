#pragma once
#ifndef _IO_HPP
#define _IO_HPP
#include "config.h"

#include <cstdlib>
#include <cctype>
#include <cstring>
#include <fstream>
#include <regex>
#include <type_traits>
#include <iomanip>

#include "numeric/real.hpp"

namespace Calc {

enum TFileType : int {
    FT_Undefined = -2,
    FT_None = -1,
    FT_MatrixText = 0,
    FT_MatrixBin = 1,
    FT_Csv = 2,
    FT_Count = 3
};

static const int FILE_EXTENSION_WIDTH=10;
static const char TFileExt[FT_Count][FILE_EXTENSION_WIDTH] =
{
    ".dat",
    ".bdat",
    ".csv"
};

#if CALC_DEFAULT_LINE_BUF_SIZE > 1024
static const int LINE_BUF_SIZE=CALC_DEFAULT_LINE_BUF_SIZE;
#else
//4KiB is default page size for most architectures
//modern system though usualy support also the so-called Huge(Super/Large) pages with sizes of several MiB (2,4,16 and so on)
static const int LINE_BUF_SIZE=4*1024*1024;
#endif

namespace IOUtil {
    template <typename T> std::string to_string_hex( T i )
    {
      std::stringstream stream;
      stream << "0x" << std::setfill ('0') << std::setw(sizeof(T)*2)  << std::hex << i;
      return stream.str();
    }

    bool readLine(std::istream& in, std::string& str);
    bool readLine(std::istream& in, char * buf, int bufSize);
    bool tryOpenFile(const char * name, const char * mode);
    bool isOkToReadFile(const std::string& filename);
    bool isOkToWriteFile(const std::string& filename);
    std::string getFileExt(const std::string& filename);
    TFileType guessFileTypeByExt(const std::string& fileName);
    std::string fileGrep(const std::string& fileName, const std::regex& rx, bool firstOnly = true, const unsigned submatchNumber = 0);

    template <typename T>
    inline int scan(const char * str, const int nElem, T * const dest, T (*f)(const char *), const int stride=1){
        const char * start = str;
        const char * end = start + strlen(str);
        int nTok = 0;
        while ( nTok < nElem ) {
            while ( start < end && isspace(*(unsigned const char *)start) ) ++start;
            if ( start >= end )
                return nTok;
            dest[nTok*stride] = f(start);
            nTok++;
            while ( start < end && ! isspace(*(unsigned const char *)start) ) ++start;
        }
        return nTok;
    }

    template <class Array, typename T>
    inline int scan(const char * str, const int nElem, Array& dest, T (*f)(const char *), const int stride=1){
        const char * start = str;
        const char * end = start + strlen(str);
        int nTok = 0;
        while ( nTok < nElem ) {
            while ( start < end && isspace(*(unsigned const char *)start) ) ++start;
            if ( start >= end )
                return nTok;
            dest(nTok*stride) = f(start);
            nTok++;
            while ( start < end && ! isspace(*(unsigned const char *)start) ) ++start;
        }
        return nTok;
    }

    float __atof__(const char * str);
    double __atod__(const char * str);
    long double __atold__(const char * str);
#ifdef HAVE_QUADMATH
    numeric::quad __atoq__(const char * str);
#endif
#ifdef HAVE_MPREAL
    numeric::mpreal __atompfr__(const char * str);
#endif

    template <typename T> inline int scanArray(const char * str, int nElem , T * const dest, int stride = 1) {
      static_assert(std::is_arithmetic<T>::value, "Number required. No support for any other arrays yet.");
      return 0;
    }

    template <> inline int scanArray<int>(const char * str, int nElem, int * dest, int stride)
      { return scan<int>(str, nElem, dest, atoi, stride); }
    template <> inline int scanArray<long>(const char * str, int nElem, long * dest, int stride)
      { return scan<long>(str, nElem, dest, atol, stride); }
    template <> inline int scanArray<long long>(const char * str, int nElem, long long * dest, int stride)
      { return scan<long long>(str, nElem, dest, atoll, stride); }

    template <> inline int scanArray<float>(const char * str, int nElem, float * dest, int stride)
      { return scan<float>(str, nElem, dest, __atof__, stride); }
    template <> inline int scanArray<double>(const char * str, int nElem, double * dest, int stride)
      { return scan<double>(str, nElem, dest, __atod__, stride); }
    template <> inline int scanArray<long double>(const char * str, int nElem, long double * dest, int stride)
      { return scan<long double>(str, nElem, dest, __atold__, stride); }
#ifdef HAVE_QUADMATH
    template <> inline int scanArray<numeric::quad>(const char * str, int nElem, numeric::quad * dest, int stride)
      { return scan<numeric::quad>(str, nElem, dest, __atoq__, stride); }
#endif
#ifdef HAVE_MPREAL
    template <> inline int scanArray<numeric::mpreal>(const char * str, int nElem, numeric::mpreal * dest, int stride)
      { return scan<numeric::mpreal>(str, nElem, dest, __atompfr__, stride); }
#endif

    template <typename T> inline constexpr const char* getNumFmt()
    { return "%.*g"; }
    template <> inline constexpr const char* getNumFmt< numeric::TraitBuiltin<numeric::P_LongDouble>::type >()
    { return "%.*Lg"; }
#ifdef HAVE_QUADMATH
    template <> inline constexpr const char* getNumFmt< numeric::TraitBuiltin<numeric::P_Quad>::type >()
    { return "%.*Qg"; }
#endif
#ifdef HAVE_MPREAL
    template <> inline constexpr const char* getNumFmt< numeric::TraitBuiltin<numeric::P_MPFR>::type >()
    { return "%.*Rg"; }
#endif

    template <typename T> inline bool printNumber(char * const str, const size_t len, const T& n, const int digits = 6)
    {
      static_assert(std::is_arithmetic<T>::value, "Number required. No support for any other arrays yet.");
      return ( len > size_t(::snprintf(str, len, getNumFmt<T>(), digits, n)) );
    }
#ifdef HAVE_QUADMATH
    template <> inline bool printNumber(char * const str, const size_t len, const numeric::quad& n, const int digits)
    {
      return ( len > size_t(::quadmath_snprintf(str, len, getNumFmt<numeric::quad>(), digits, n.backend().value())));
    }
#endif
#ifdef HAVE_MPREAL
    template <> inline bool printNumber(char * const str, const size_t len, const numeric::mpreal& n, const int digits)
    {
# ifdef HAVE_BOOST_MPREAL
      return ( len > size_t(::mpfr_snprintf(str, len, getNumFmt<numeric::mpreal>(), digits, n.backend().data())));
# else
      return ( len > size_t(::mpfr_snprintf(str, len, getNumFmt<numeric::mpreal>(), digits, n.mpfr_srcptr())));
# endif
    }
#endif

}

template <class S> class ios_guard {
private:
    S * m_ptr;    // refers to the object it holds (if any)

public:
    // default constructor: let the holder refer to nothing
    ios_guard() : m_ptr(0) {}

    // constructor for a pointer: let the holder refer to where the pointer refers
    explicit ios_guard(S * p) : m_ptr(p) {}

    // destructor: releases the object to which it refers (if any)
    ~ios_guard() { close(); }

    void close() {
        if ( !m_ptr )
            return;
        delete m_ptr;
        m_ptr = 0;
    }

    // let stream loose
    S * release() {
        S * oldp = m_ptr;
        m_ptr = 0; 
        return oldp; 
    }

    // close the old stream and assign a new one
    void reset(S * newPtr) {
        if ( m_ptr )
            delete m_ptr;
        m_ptr = newPtr;
    }

    // lose control over the old stream and take ownership of the new
    inline ios_guard& operator= (S * p) {
        m_ptr = p;
        return *this;
    }

    inline S * get () const {
        return m_ptr;
    }

    inline S& operator -> () const {
        return *m_ptr;
    }

    inline S& operator * () const {
        return *m_ptr;
    }
private:
    // no copying and copy assignment allowed
    ios_guard(const ios_guard&);
    ios_guard(const ios_guard&&);
    ios_guard& operator= (const ios_guard&);
};

}
#endif /* _IO_HPP */
