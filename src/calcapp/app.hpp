#pragma once
#ifndef _APP_HPP
#define _APP_HPP
#include "config.h"

#include <memory>

#include "calcapp/options.hpp"
#include "calcapp/progress.hpp"

namespace Calc {

class App {
protected:
    std::string m_AppName;
    std::string m_AppVersion;
    LoggingOptions m_logging;
    ThreadingOptions m_threading;
    PrecisionOptions m_precision;
private:
    ProgressCtrl * const m_pProgress;
    void init_helper(const ProgressCtrl * const);
    App();
public:
    App(const AppOptions&, ProgressCtrl * const);
    App(ProgressCtrl * const);
    virtual void setDefaultOptions() = 0;
    virtual void readInput() = 0;
    virtual void writeOutput() = 0;
    virtual void run() = 0;
    inline ProgressCtrl * ctrl() const { return m_pProgress; }
    inline Logger& log() const { return m_pProgress->log(); }
};

}

#endif /* _APP_HPP */
