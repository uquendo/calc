#pragma once
#ifndef _APP_HPP
#define _APP_HPP
#include "config.h"

#include "calcapp/options.hpp"
#include "calcapp/progress.hpp"

namespace Calc {

class App {
private:
    ProgressCtrl* m_pProgress;
public:
    App();
    App(ProgressCtrl*);
    virtual ~App();
    virtual void setDefaultOptions() = 0;
    virtual void readInput() = 0;
    virtual void run() = 0;
    inline ProgressCtrl * ctrl() const { return m_pProgress; }
    inline Logger& log() const { return m_pProgress->log(); }
};

}

#endif /* _APP_HPP */
