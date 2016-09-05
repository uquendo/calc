#include "calcapp/cli.hpp"
#include "calcapp/io.hpp"
#include "calcapp/system.hpp"

#include <fstream>
#include <string>

#ifdef HAVE_BOOST
#include <boost/format.hpp>
#endif

using std::string;

namespace Calc {

class DumbLogger : public Logger {
private:
  ios_guard<std::ofstream> m_logFile;
  Logger::LogLevel m_level;
  double m_startTime;

public:
  DumbLogger(Logger::LogLevel severity, const string& logName) {
    if ( ! logName.empty() )
      m_logFile = new std::ofstream(logName.c_str());

    m_level = severity;
    m_startTime = SysUtil::getCurTimeSec();
  };

  virtual void log(Logger::LogLevel level, const char * msg) override {
    if ( level > m_level )
      return;

    string m = "";
#ifdef HAVE_BOOST
    m = (boost::format("%.3f %s: %s")
      % (SysUtil::getCurTimeSec() - m_startTime)
      % ((level >= 0 && level < LOG_LEVEL_NAME_COUNT) ? LOG_LEVEL_NAME[level] : "")
      % msg
    ).str();
#else
    m = std::to_string((double)(SysUtil::getCurTimeSec() - m_startTime));
    m.append(((level >= 0 && level < LOG_LEVEL_NAME_COUNT) ? LOG_LEVEL_NAME[level] : ""));
    m.append(": ").append(msg);
#endif
    if ( m_logFile.get() )
      *m_logFile << m << '\n';

    puts(m.c_str());
  };
};

CliProgress::CliProgress(Logger::LogLevel severity, const string& logName)
  : m_pLogger(severity != Logger::Logger::L_NONE ?
      reinterpret_cast<Logger *>(new DumbLogger(severity, logName)) : reinterpret_cast<Logger *>(new QuietLogger()))
        , m_StopNow(false)
{
  Logger::setSystem(m_pLogger.get());
}


CliProgress::CliProgress(const LoggingOptions& opts)
  : m_pLogger(opts.level != Logger::Logger::L_NONE ?
      reinterpret_cast<Logger *>(new DumbLogger(opts.level, opts.filename)) : reinterpret_cast<Logger *>(new QuietLogger()))
        , m_StopNow(false)
{
  Logger::setSystem(m_pLogger.get());
}

CliProgress::CliProgress():CliProgress(Logger::Logger::L_NONE, string(""))
{
}

CliProgress::CliProgressBar::CliProgressBar(Logger& log, const char * title)
  : m_log(log), m_title(title)
  , m_lower(0), m_upper(0), m_pos(0), m_step(1), m_reportedPercentage(-1)
  , m_startTime(SysUtil::getCurTimeSec())
{}


CliProgress::CliProgressBar::~CliProgressBar() {
  double elapsed = SysUtil::getCurTimeSec() - m_startTime;
  m_log.fdebug("%s: done (%f seconds)", m_title.c_str(), elapsed);
}

void CliProgress::CliProgressBar::setProgressRange(int lower, int upper) {
  m_lower = lower; m_upper = upper;
}

void CliProgress::CliProgressBar::setProgress(int pos) {
  m_pos = pos;
  advancePos(false);
}

void CliProgress::CliProgressBar::setProgressStep(int step) {
  m_step = step;
}

void CliProgress::CliProgressBar::setTitle(const char * title) {
  m_title = title;
}

void CliProgress::CliProgressBar::clearTitle() {
  m_title.clear();
}

void CliProgress::CliProgressBar::stepIt() {
  m_pos += m_step;
  advancePos(false);
}

void CliProgress::CliProgressBar::advancePos(bool reset) {
  if ( m_upper <= m_lower )
    return;

  if ( m_pos > m_upper )
    m_pos = m_upper;
  if ( m_pos < m_lower )
    m_pos = m_lower;

  int curPercentage = 10 * (m_pos - m_lower) / (m_upper - m_lower);
  if ( curPercentage != m_reportedPercentage || reset ) {
    string perc = "";
#ifdef HAVE_BOOST
    perc = str(boost::format("%02i%%...") % (curPercentage * 10));
#else
    perc = std::to_string(curPercentage * 10).append("%...");
#endif
    m_log.fdebug("%s: %s", m_title.c_str(), perc.c_str());
    m_reportedPercentage = curPercentage;
  }
}

void CliProgress::onStartCalc() {
  log().debug("Starting calculation");
}

void CliProgress::onFinishCalc() {
  log().debug("Finishing calculation");
}

void CliProgress::onAbortCalc()  {
  log().warning("Calculation aborted");
}

bool CliProgress::stopNow() {
  return m_StopNow.load(std::memory_order_relaxed);
}

void CliProgress::setStopNow() {
        m_StopNow.store(std::memory_order_release);
}

// Progress bar
ProgressBar * CliProgress::createProgressBar(const char * title) {
  return new CliProgressBar(log(), title);
}

void CliProgress::setStatusText(const char * text) {
  log().fdebug("Status: %s", text);
}

void CliProgress::clearStatusText() {
}

Logger& CliProgress::log() {
  return *m_pLogger;
}

}

