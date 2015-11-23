#pragma once
#ifndef _EXCEPTION_HPP
#define _EXCEPTION_HPP
#include "config.h"

#include <cstddef>
#include <string>

#ifdef HAVE_BOOST
#include <boost/format.hpp>
#endif

#include "calcapp/io.hpp"

namespace Calc {

/* Error codes */
static const int FERR_NO_ERROR         = 0;
static const int FERR_GENERAL_ERROR    = 0x001;
static const int FERR_BAD_FILE_TYPE    = 0x002;
static const int FERR_IO_GENERAL_READ  = 0x110;
static const int FERR_IO_GENERAL_WRITE = 0x120;
static const int FERR_IO_FILE_ACCESS   = 0x130;
static const int FERR_IO_NO_DISK_SPACE = 0x140;
static const int FERR_IO_FORMAT_ERROR  = 0x150;
static const int FERR_IO_PRELIMINARY_EOF = 0x160;
static const int FERR_OUT_OF_MEMORY    = 0x200;
static const int FERR_DATA_ERROR       = 0x300;


class BaseException : public std::exception {
protected:
  int m_code;
  std::string m_errStr;
  std::exception m_cause;
public:
  BaseException(int code, const char * errStr, const std::exception * cause = 0) : m_code(code), m_errStr(errStr), m_cause(cause ? *cause : exception()) {}
  virtual ~BaseException() throw () {}

  virtual const char * what() const throw () { return m_errStr.c_str(); }
  inline int code() const { return m_code; }
  inline const std::exception& cause() const { return m_cause; }
};

class ParameterError : public BaseException {
public:
  ParameterError(const char * errStr) : BaseException(FERR_DATA_ERROR, errStr) {}
};


class IOError : public BaseException {
protected:
  TFileType m_fileType;
  std::string m_fileName, m_fullStr;
    std::size_t m_lineNum;
public:
  IOError(int code, const char * errStr, TFileType fileType, const char * fileName, std::size_t line) 
    : BaseException(code, errStr), m_fileType(fileType), m_fileName(fileName), m_lineNum(line)
  {
#ifdef HAVE_BOOST
    m_fullStr = (boost::format("File %s (type %s), line %li: %s") % fileName % TFileExt[fileType] % line % errStr).str();
#else
    m_fullStr = "";
    m_fullStr.append("File ").append(fileName).append(" (type ").append(TFileExt[fileType]).append("), line ");
    m_fullStr.append(std::to_string(line)).append(": ").append(errStr);
#endif
  }

  IOError(int code, const char * errStr, TFileType fileType, const char * fileName) 
    : BaseException(code, errStr), m_fileType(fileType), m_fileName(fileName), m_lineNum(0)
  {}

  virtual ~IOError() throw() {}

  inline TFileType fileType() const { return m_fileType; }
  inline const char * fileName() const { return m_fileName.c_str(); }
  inline std::size_t fileLine() const { return m_lineNum; }

  virtual const char * what() const throw () { return m_fullStr.c_str(); }
};

class PreliminaryEofError : public IOError {
public:
  PreliminaryEofError(const char * errStr, TFileType fileType, const char * fileName, std::size_t line) : IOError(FERR_IO_PRELIMINARY_EOF, errStr, fileType, fileName, line) {}
};

class FileFormatError : public IOError {
public:
  FileFormatError(int code, const char * errStr, TFileType fileType, const char * fileName, std::size_t line) : IOError(code, errStr, fileType, fileName, line) {}
};

class FileFormatValueBoundsError : public FileFormatError {
public:
  FileFormatValueBoundsError(const char * errStr, TFileType fileType, const char * fileName, std::size_t line) : FileFormatError(FERR_DATA_ERROR, errStr, fileType, fileName, line) {}
};

class OutOfDiskSpaceError : public IOError {
protected:
  unsigned long m_freeMB, m_requiredMB;
public:
  OutOfDiskSpaceError(const char * errStr, TFileType fileType, const char * fileName, unsigned long freeMB, unsigned long requiredMB) 
    : IOError(FERR_IO_NO_DISK_SPACE, errStr, fileType, fileName) 
    , m_freeMB(freeMB), m_requiredMB(requiredMB)
  {}
};

class InternalError : public BaseException {
public:
  InternalError(int code, const char * errStr, std::exception * e = 0) : BaseException(code, errStr, e) {}
};

class OOMError : public InternalError {
public:
  OOMError(const char * errStr, std::exception * cause = 0) : InternalError(FERR_OUT_OF_MEMORY, errStr, cause) {}
};

}
#endif /* _EXCEPTION_HPP */
