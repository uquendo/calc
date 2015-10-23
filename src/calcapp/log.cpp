#include "config.h"

#include <cstdarg>
#include <cstdio>
#include <memory>

#include "calcapp/log.hpp"
#include "calcapp/exception.hpp"
#include "calcapp/system.hpp"

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

LogMessage::LogMessage(Logger& log, LogLevel level, std::ostringstream& oss):m_log(log),m_level(level),m_oss(oss)
{
}

LogMessage::~LogMessage(){
  m_log.log(m_level,m_buf.c_str());
}

inline LogMessage LogMessage::slog(Logger& log, LogLevel level, std::ostringstream& oss)
{
  return LogMessage(log,level,oss);
}

template<class T> LogMessage& LogMessage::operator<<(const T& obj){
  m_oss.str(std::string());
  m_oss << obj;
  if(!m_buf.empty())
    m_buf += " ";
  m_buf += m_oss.str();
  return *this;
}

LogMessage Logger::slog(LogLevel level) {
  return LogMessage::slog(*this,level,m_oss);
}

// syntax sugar for streams logging
inline LogMessage Logger::slog() { return slog(m_level); }
inline LogMessage Logger::serror() { return slog(L_ERROR); }
inline LogMessage Logger::swarning() { return slog(L_WARNING); }
inline LogMessage Logger::sdebug() { return slog(L_DEBUG); }

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
		fputs(msg, stderr);
	};
} defLogger;

void Logger::error(BaseException * e)
{ 
	std::string r = (boost::format("%6x: %s") % e->code() % e->what()).str();
	IOError * ioe = dynamic_cast<IOError *>(e);
	if ( ioe ) {
		r += (boost::format(": FORMAT=%i FILE='%s' LINE=%i") % ioe->fileType() % ioe->fileName() % ioe->fileType()).str();
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
