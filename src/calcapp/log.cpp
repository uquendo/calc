#include "config.h"

#include <memory>
#include <cstdarg>
#include <cstdio>

#include <boost/format.hpp>

#include "calcapp/log.hpp"
#include "calcapp/exception.hpp"
#include "calcapp/system.hpp"

namespace Calc {

const int LOG_LEVEL_NAME_COUNT=7;
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

void Logger::error(Exception * e)
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
