#pragma once
#ifndef _IO_HPP
#define _IO_HPP
#include "config.h"

#include <cstdlib>
#include <cctype>
#include <cstring>
#include <fstream>
#include <regex>

namespace Calc {

enum TFileType {
    FT_None =-1,
    FT_Matrix = 0,
    FT_Csv = 1,
    FT_Count = 2
};

static const char TFileExt[FT_Count][5] =
{
    ".dat",
    ".csv"
};

static const int LINE_BUF_SIZE=250;

class IOUtil {
public:
    static bool readLine(std::ifstream& in, std::string& str);
    static bool readLine(std::ifstream& in, char * buf, int bufSize);
    static bool tryOpenFile(const char * name, const char * mode);
    static std::string fileGrep(const char * fileName, const std::regex& rx, bool firstOnly = true);

    template <typename T>
    static int scan(const char * str, int nElem, T * dest, T (*f)(const char *)){
        const char * start = str;
        const char * end = start + strlen(str);
        int nTok = 0;
        while ( nTok < nElem ) {
            while ( start < end && isspace(*(unsigned const char *)start) ) ++start;
            if ( start >= end )
                return nTok;
            dest[nTok] = f(start);
            nTok++;
            while ( start < end && ! isspace(*(unsigned const char *)start) ) ++start;
        }
        return nTok;
    }

    static inline int scanDoubleArray(const char * str, int nElem, double * dest) { return scan<double>(str, nElem, dest, atof); }
    static inline int scanLongArray(const char * str, int nElem, long * dest) { return scan<long>(str, nElem, dest, atol); }

};

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
    ios_guard& operator= (const ios_guard&);
};

}
#endif /* _IO_HPP */
