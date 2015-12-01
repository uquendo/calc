#include "calcapp/log.hpp"
#include "calcapp/exception.hpp"
#include "calcapp/system.hpp"
#include "calcapp/io.hpp"

#include <cstdarg>
#include <cstdio>
#include <memory>

namespace Calc {

const char * LOG_LEVEL_NAME[LOG_LEVEL_NAME_COUNT] = {
  " ", // 0
  "FATAL", // 1
  "ERROR", // 2
  "WARNING", // 3
  "NOTE", // 4
  "DEBUG", // 5
  "TRACE"  // 6
};

Logger * Logger::m_pSystem = NULL;

Logger::LogMessage::LogMessage(Logger& log, LogLevel level, std::ostringstream& oss, std::atomic_bool& oss_in_use):
  m_log(log),m_level(level),m_oss(oss),m_oss_in_use(oss_in_use)
{
  // acquire m_oss_in_use
  bool acquired=false;
  while(!m_oss_in_use.compare_exchange_strong(acquired,true,std::memory_order_acq_rel)){
    acquired=false;
  }
}

Logger::LogMessage::~LogMessage(){
  // release m_oss_in_use
  m_oss_in_use.store(false, std::memory_order_release);
  // log actial message
  m_log.log(m_level,m_buf.c_str());
}

inline Logger::LogMessage Logger::LogMessage::slog(Logger& log, LogLevel level, std::ostringstream& oss, std::atomic_bool& oss_in_use)
{
  return LogMessage(log,level,oss,oss_in_use);
}

template<> Logger::LogMessage& Logger::LogMessage::operator<<(const std::string& obj){
  if(!m_buf.empty())
    m_buf += " ";
  m_buf += obj;
  return *this;
}

template<class T> Logger::LogMessage& Logger::LogMessage::operator<<(const T& obj){
  m_oss.str(std::string());
  m_oss << obj;
  if(!m_buf.empty())
    m_buf += " ";
  m_buf += m_oss.str();
  return *this;
}

Logger::LogMessage Logger::slog(LogLevel level) {
  return LogMessage::slog(*this,level,m_oss,m_oss_in_use);
}

// syntactic sugar for streams logging
inline Logger::LogMessage Logger::slog() { return slog(m_level); }
inline Logger::LogMessage Logger::serror() { return slog(L_ERROR); }
inline Logger::LogMessage Logger::swarning() { return slog(L_WARNING); }
inline Logger::LogMessage Logger::sdebug() { return slog(L_DEBUG); }

void Logger::vflog(LogLevel level, const char * format, va_list va) {
  static const int BUF_SIZE=1024;
  char * buf = new char[1024];
  vsnprintf(buf, BUF_SIZE, format, va);
  log(level, buf);
  delete[] buf;
}

class DefaultLogger : public Logger {
public:
  DefaultLogger() {
    setSystem(this);
  }

  virtual void log(LogLevel level, const char * msg) {
    fputs(LOG_LEVEL_NAME[level], stderr);
    fputs(msg, stderr);
    fputs("\n", stderr);
  };
} defLogger;

void Logger::error(BaseException * e)
{ 
  std::string r="";
#ifdef HAVE_BOOST
  r = (boost::format("%6x: %s") % e->code() % e->what()).str();
#else
  r.append(IOUtil::to_string_hex(e->code())).append(": ").append(e->what());
#endif
  IOError * ioe = dynamic_cast<IOError *>(e);
  if ( ioe ) {   
#ifdef HAVE_BOOST
    r += (boost::format(": FORMAT=%i FILE='%s' LINE=%i") % ioe->fileType() % ioe->fileName() % ioe->fileLine()).str();
#else
    r.append(": FORMAT=").append(std::to_string(ioe->fileType())).append(" FILE='").append(ioe->fileName()).append("' LINE=");
    r.append(std::to_string(ioe->fileLine()));
#endif
  }
  error(r.c_str()); 
}

ExecTimeMeter::ExecTimeMeter(Logger& log, const char * name) 
  : m_log(log), m_name(name), m_startTime(SysUtil::getCurTimeSec())
{
}

ExecTimeMeter::~ExecTimeMeter() 
{
  double endTime = SysUtil::getCurTimeSec() - m_startTime;
  m_log.fdebug("PERF: %s: %f seconds", m_name, endTime);
}


}
