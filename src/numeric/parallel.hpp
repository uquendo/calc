#pragma once
#ifndef _THREAD_HPP
#define _THREAD_HPP
#include "config.h"

# include <memory>

#ifdef HAVE_TBB
# include <tbb/task_scheduler_init.h>
#endif

namespace numeric {

enum TThreading {
  T_Serial,
  T_Std,
  T_Posix,
  T_OpenMP,
  T_Cilk,
  T_TBB,
  T_Undefined
};

//wrapper over std::thread and many other ways to obtain number of processors
unsigned hardware_concurrency();

class ParallelScheduler
{
  static unsigned threadsNumber;
  static TThreading threadingBackend;
  static std::unique_ptr<ParallelScheduler> pGlobalScheduler;
#ifdef HAVE_TBB
  std::unique_ptr<tbb::task_scheduler_init> m_ptbb_scheduler;
#endif
  void initBackend();
  void terminateBackend();
  void setThreadsNumberBackend(unsigned num);
public:
  ParallelScheduler(const TThreading threadingBackend, const unsigned threadsNumber);
  ~ParallelScheduler();
  static void setThreadsNumber(unsigned num);
  static unsigned getThreadsNumber();
private:
  //no copying or copy assignment allowed
  ParallelScheduler(const ParallelScheduler&);
  ParallelScheduler(const ParallelScheduler&&);
  ParallelScheduler& operator= (const ParallelScheduler&);
};

}

#endif /* _THREAD_HPP */
