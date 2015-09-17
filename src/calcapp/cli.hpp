#pragma once
#ifndef _CLI_HPP
#define _CLI_HPP
#include "config.h"

#include <cstring>
#include <string>
#include <atomic>

#include "calcapp/progress.hpp"
#include "calcapp/app.hpp"

namespace Calc {

class CliAppOptions : public AppOptions {
public:
        CliAppOptions();
        virtual ~CliAppOptions();
        //set options from command line arguments
        virtual bool processOptions(int argc, char* argv[]);
        //prepare cli options
        virtual void prepareOptions();
        virtual void prepareLoggingOptions();
        virtual void prepareThreadingOptions();
        virtual void preparePrecisionOptions();
        virtual void prepareInputOptions();
        virtual void prepareOutputOptions();
        virtual void prepareAlgoOptions(); 
        //parse cli options
        virtual bool parseOptions();
        virtual bool parseLoggingOptions();
        virtual bool parseThreadingOptions();
        virtual bool parsePrecisionOptions();
        virtual bool parseInputOptions();
        virtual bool parseOutputOptions();
        virtual bool parseAlgoOptions();
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
		virtual void setProgressRange(int lower, int upper);
		virtual void setProgress(int pos);
		virtual void setProgressStep(int step);
		virtual void setTitle(const char * title);
		virtual void clearTitle();
		virtual void stepIt();
		void advancePos(bool reset);
	};

	CliProgress(Logger::LogLevel severity, const std::string& logName);
	CliProgress(){}

	virtual void onStartCalc();
	virtual void onFinishCalc();
	virtual void onAbortCalc();
	virtual bool stopNow();
	virtual void setStopNow();

	// Progress bar
	virtual ProgressBar * createProgressBar(const char * title);
	virtual void setStatusText(const char * text);
	virtual void clearStatusText();

	virtual Logger& log();
	
private:
	std::auto_ptr<Logger> m_pLogger;
    std::atomic<bool> m_StopNow;
};

}

#endif /* _CLI_HPP */
