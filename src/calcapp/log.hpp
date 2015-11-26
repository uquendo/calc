#pragma once
#ifndef _LOG_HPP
#define _LOG_HPP
#include "config.h"

#include <cstdarg>
#include <string>
#include <iostream>
#include <sstream>
#include <atomic>

#ifdef HAVE_BOOST
#include <boost/format.hpp>
#endif

#ifdef ENABLE_PERFMETER
#define PERF_METER(log, name) Calc::ExecTimeMeter __etm(log, name);
#else
#define PERF_METER(log, name)
#endif

namespace Calc {

static const int LOG_LEVEL_NAME_COUNT=7;
extern const char * LOG_LEVEL_NAME[LOG_LEVEL_NAME_COUNT];

class BaseException;

class Logger {
public:
  enum LogLevel { L_TRACE = 6, L_DEBUG = 5, L_NOTE = 4, L_WARNING = 3, L_ERROR = 2, L_FATAL = 1, L_NONE = 0 };

  virtual void log(LogLevel level, const char * msg) = 0;

protected: 
  // serialize message to log via Logger::log()
  class LogMessage {
  public:
      ~LogMessage();
      inline static LogMessage slog(Logger& , LogLevel , std::ostringstream& , std::atomic_bool& );
      template<class T> LogMessage& operator<<(const T& );
  private:
      LogMessage(Logger& , LogLevel , std::ostringstream& , std::atomic_bool& );
      Logger& m_log;
      LogLevel m_level;
      std::ostringstream& m_oss;
      std::atomic_bool& m_oss_in_use;
      std::string m_buf;
  };

public:
  // logging via c++ streams
  LogMessage slog(LogLevel level);

  // syntax sugar for streams logging
  inline LogMessage slog(); 
  inline LogMessage serror(); 
  inline LogMessage swarning(); 
  inline LogMessage sdebug(); 

  // syntax sugar for c-style logging
  inline void log(LogLevel level, const std::string& msg)         { log(level, msg.c_str()); }
  inline void flog(LogLevel  level, const char * format, ...)    { va_list va; va_start(va, format); vflog(level, format, va); va_end(va); }
  void vflog(LogLevel  level, const char * format, va_list va);

  inline void error(const char * msg)                { log(L_ERROR, msg); }
  inline void ferror(const char * format, ...)      { va_list va; va_start(va, format); vflog(L_ERROR, format, va); va_end(va); }
  inline void error(const std::exception& e, const char * msg)  { ferror("%s: %s", msg, e.what()); }
  inline void error(const std::string& e)            { error(e.c_str()); }
  void error(BaseException * e);

  inline void fwarning(const char * format, ...)    { va_list va; va_start(va, format); vflog(L_WARNING, format, va); va_end(va); }
  inline void warning(const char * msg)              { log(L_WARNING, msg); }
  inline void warning(const std::string& msg)        { log(L_WARNING, msg); }

  inline void fdebug(const char * format, ...)      { va_list ap; va_start(ap, format); vflog(L_DEBUG, format, ap); va_end(ap); }
  inline void debug(const char * msg)                { log(L_DEBUG, msg);  }
  inline void debug(const std::string& msg)          { log(L_DEBUG, msg);  }

  // default loglevel management
  void setLogLevel(LogLevel level) { m_level = level; }
  LogLevel getLogLevel() { return m_level; }

  // default system logger
  inline static Logger& system()                  { return *m_pSystem; }
  inline static void setSystem(Logger * sysLog)          { m_pSystem = sysLog; }
  Logger():m_level(L_NONE){}

protected:
  static Logger * m_pSystem;
  LogLevel m_level;
  std::ostringstream m_oss;
  std::atomic_bool m_oss_in_use;
};


class ExecTimeMeter {
public: 
    ExecTimeMeter(Logger& log, const char * name);
    ~ExecTimeMeter();
private:
    Logger& m_log;
    const char * m_name;
    double m_startTime;
};

template <typename T, typename LT> class Statistics {
    T m_count, m_min, m_max;
    LT m_sum;

public:
    Statistics() : m_count(0), m_min(0), m_max(0), m_sum(0) {}

    void reset() {
        m_count = m_min = m_max = 0;
        m_sum = 0;
    }

    void sample(T t) {
        m_sum += t;
        m_count++;
        if ( m_count == 1 ) {
            m_min = m_max = t;
        } else {
            m_min = min(m_min, t);
            m_max = max(m_max, t);
        }
    }

    inline T count() const { return m_count; }
    inline T avg() const { if ( m_count != 0 ) return (T) m_sum / m_count; else return 0; }
    inline LT sum() const { return m_sum; }
    inline T max_val() const { return m_max; }
    inline T min_val() const { return m_min; }

    std::string toString() const {
      std::string r = "";
#ifdef HAVE_BOOST
      r = (boost::format("average=%1%; min=%2%; max=%3%; samples=%4%; ") % avg() % min_val() % max_val() % count()).str();
#else
      r.append("average=").append(std::to_string(avg())).append("; ");
      r.append("min=").append(std::to_string(min_val())).append("; ");
      r.append("max=").append(std::to_string(max_val())).append("; ");
      r.append("samples=").append(std::to_string(count())).append("; ");
#endif
      return r;
    }
};

typedef Statistics<long, long long> LongStatistics;

}

#endif /* _LOG_HPP */
