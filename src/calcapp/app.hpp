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
    virtual void setOptions(const AppOptions&);
    virtual void readInput();
    virtual void run();
    inline ProgressCtrl * ctrl() const { return m_pProgress; }
    inline Logger& log() const { return m_pProgress->log(); }
};

}

#endif /* _APP_HPP */
