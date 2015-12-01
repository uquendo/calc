#pragma once
#ifndef _CLI_HPP
#define _CLI_HPP
#include "config.h"

#include <cstring>
#include <string>
#include <memory>
#include <atomic>

#ifdef HAVE_BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
namespace bpo=boost::program_options;
#endif

#include "calcapp/progress.hpp"
#include "calcapp/app.hpp"
#include "calcapp/options.hpp"

namespace Calc {

class CliAppOptions : public AppOptions {
public:
        CliAppOptions(std::string AppName = std::string("SomeCliApp"), std::string AppVersion = std::string("0.0"));
        //set options from command line arguments
        virtual bool processOptions(int argc, char* argv[]);
        //generic information options
        virtual void printAbout();
        virtual void printHelp();
        virtual const std::string About() const;
        virtual const std::string Help() const;
protected:
        //prepare cli options
        virtual void prepareOptions();
        virtual void prepareLoggingOptions();
        virtual void prepareThreadingOptions();
        virtual void preparePrecisionOptions();
        virtual void prepareInputOptions();
        virtual void prepareOutputOptions();
        virtual void prepareAlgoOptions();
        //parse cli options
        virtual bool parseOptions(int argc, char* argv[]);
        virtual bool parseLoggingOptions();
        virtual bool parseThreadingOptions();
        virtual bool parsePrecisionOptions();
        virtual bool parseInputOptions();
        virtual bool parseOutputOptions();
        virtual bool parseAlgoOptions();
protected:
#ifdef HAVE_BOOST
        bpo::variables_map argMap;
        bpo::options_description allOpt;
        bpo::options_description commonOpt;
        bpo::options_description inputOpt;
        bpo::options_description outputOpt;
        bpo::options_description algoOpt;
#endif
        std::string threadingHelp;
        std::string precisionHelp;
};

class CliProgress;

class CliApp : public App {
private:
    std::unique_ptr<CliProgress> m_CliProgress;
    CliApp();
public:
    CliApp(const CliAppOptions&); // init and maintain logger
    CliApp(const CliAppOptions&, ProgressCtrl * const);
    CliApp(ProgressCtrl* const);
    virtual void setDefaultOptions() = 0;
    virtual void readInput() = 0;
    virtual void run() = 0;
};

class CliProgress : public ProgressCtrl {
public:
    class CliProgressBar : public ProgressBar {
  private:
    Logger& m_log;
    std::string m_title;
    int m_lower, m_upper, m_pos, m_step, m_reportedPercentage;
    double m_startTime;

  public:
    CliProgressBar(Logger& log, const char * title);
    virtual ~CliProgressBar();
    virtual void setProgressRange(int lower, int upper) override;
    virtual void setProgress(int pos) override;
    virtual void setProgressStep(int step) override;
    virtual void setTitle(const char * title) override;
    virtual void clearTitle() override;
    virtual void stepIt() override;
    void advancePos(bool reset);
  };

  CliProgress(Logger::LogLevel severity, const std::string& logName);
  CliProgress(const LoggingOptions& opts);
  CliProgress();

  virtual void onStartCalc() override;
  virtual void onFinishCalc() override;
  virtual void onAbortCalc() override;
  virtual bool stopNow() override;
  virtual void setStopNow() override;

  // Progress bar
  virtual ProgressBar * createProgressBar(const char * title) override;
  virtual void setStatusText(const char * text) override;
  virtual void clearStatusText() override;

  virtual Logger& log() override;

private:
  std::unique_ptr<Logger> m_pLogger;
  std::atomic<bool> m_StopNow;
};

}


#endif /* _CLI_HPP */
