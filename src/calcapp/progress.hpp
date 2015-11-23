#pragma once
#ifndef _PROGRESS_HPP
#define _PROGRESS_HPP
#include "config.h"

#include <cstdarg>
#include <string>
#include <utility>
#include <atomic>

#include "calcapp/log.hpp"

namespace Calc {

class ProgressBar {
public:
  virtual ~ProgressBar() {};
  virtual void setProgressRange(int lower, int upper) = 0; 
  virtual void setProgress(int pos) = 0;
  virtual void setProgressStep(int step) = 0; 
  virtual void setTitle(const char * title) = 0; 
  inline void setTitle(const std::string& title) { setTitle(title.c_str()); }
  virtual void clearTitle() = 0; 
  virtual void stepIt() = 0; // advance the progress by one step

  void initProgressParams(long *Step, long* Sum);
};

class ProgressCtrl {
public:
  virtual void onStartCalc() = 0; // Hook called at the calculation start
  virtual void onFinishCalc() = 0;
  virtual void onAbortCalc() = 0; // Hook called at the calculation abort

  virtual bool stopNow() = 0; // Returns true to bail out of the calculation. Called periodically (should be thread-safe)
  virtual void setStopNow() = 0; // Asks ProgressCtrl to stop calculation. (should be thread-safe)

  // Progress bar
  virtual ProgressBar * createProgressBar(const char * title) = 0; // Recreate progress bar
  inline ProgressBar * createProgressBar(const std::string& title) { return createProgressBar(title.c_str()); }

  // Status text
  virtual void setStatusText(const char * text) = 0;
  inline void setStatusText(const std::string& text) { setStatusText(text.c_str()); }
  virtual void clearStatusText() = 0;

  // Logging 
  virtual Logger& log() = 0;
};

}

#endif /* _PROGRESS_HPP */
