#include "numeric/parallel.hpp"

#include <exception>
#include <string>
#include <thread>

//headers for runtime cpu count detection
#if defined(_WIN32)
# include <windows.h>
#elif defined(__linux__) || defined(HAVE_UNISTD_H)
# include <unistd.h>
#elif defined(__APPLE__) || defined(__FreeBSD__)
# include <sys/sysctl.h>
#elif defined(PTW32_VERSION) || defined(__hpux)
# include <pthread.h>
#endif

//[some] headers for threading backends
#ifdef HAVE_PTHREADS
# include <pthread.h>
#endif
#ifdef HAVE_OPENMP
# include <omp.h>
#endif
#ifdef HAVE_CILK
# include <cilk/cilk_api.h>
#endif

namespace numeric {

unsigned hardware_concurrency()
{
  //trying standard way first
  int count = std::thread::hardware_concurrency();
  if(count > 0)
    return (unsigned)count;
  //trying system-dependent way then
#if defined(_WIN32)
  SYSTEM_INFO sysinfo{{0}};
  GetSystemInfo( &sysinfo );
  count = sysinfo.dwNumberOfProcessors;
#elif defined(PTW32_VERSION) || defined(__hpux)
  count = pthread_num_processors_np();
#elif defined(__APPLE__) || defined(__FreeBSD__)
  size_t size=sizeof(count);
  count = ( sysctlbyname("hw.ncpu",&count,&size,NULL,0) ? 0 : count );
#elif defined(HAVE_UNISTD_H) && defined(_SC_NPROCESSORS_ONLN)
  count = sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(HAVE_UNISTD_H) && defined(_SC_NPROC_ONLN)
  count = sysconf(_SC_NPROC_ONLN);
#endif
  if(count > 0)
    return (unsigned)count;
  //ok, trying library-dependent ways
#ifdef HAVE_TBB
  count = tbb::task_scheduler_init::default_num_threads();
  if(count > 0)
    return (unsigned)count;
#endif
#ifdef HAVE_OPENMP
  count = omp_get_num_procs();
  if(count > 0)
    return (unsigned)count;
#endif
  //ok, somehow all of it has failed, but we still has the cpu we're running on!
  return 1;
}

unsigned ParallelScheduler::threadsNumber = 1;
TThreading ParallelScheduler::threadingBackend = T_Serial;
std::unique_ptr<ParallelScheduler> ParallelScheduler::pGlobalScheduler = nullptr;

class ParallelException : public std::exception {
  std::string m_errStr;
public:
  ParallelException(const char * errStr) : m_errStr(errStr) {}
  virtual ~ParallelException() throw () {}
  virtual const char * what() const throw () { return m_errStr.c_str(); }
};


ParallelScheduler::ParallelScheduler(const TThreading _threadingBackend, const unsigned _threadsNumber)
#ifdef HAVE_TBB
  : m_ptbb_scheduler(nullptr)
#endif
{
  if(pGlobalScheduler != nullptr)
    throw ParallelException("Not allowed to create second ParallelScheduler instance");

  pGlobalScheduler.reset(this);
  threadingBackend = _threadingBackend;
  threadsNumber = _threadsNumber;
  if(threadsNumber == 0)
    threadsNumber = hardware_concurrency();

  initBackend();
}

ParallelScheduler::~ParallelScheduler()
{
  terminateBackend(); //TODO: do we actually need this?
  pGlobalScheduler.release();
}

void ParallelScheduler::setThreadsNumber(unsigned num)
{
  if ( threadsNumber == num )
    return;

  threadsNumber = num;

  if(pGlobalScheduler != nullptr)
    pGlobalScheduler->setThreadsNumberBackend(num);
}

unsigned ParallelScheduler::getThreadsNumber()
{
 return threadsNumber;
}

void ParallelScheduler::initBackend()
{
  switch(threadingBackend)
  {
    case T_Serial:
      return;
    case T_Std:
      return;
#ifdef HAVE_PTHREADS
    case T_Posix:
      return;
#endif
#ifdef HAVE_OPENMP
    case T_OpenMP:
      return;
#endif
#ifdef HAVE_CILK
    case T_Cilk:
      if(threadsNumber > 0)
        __cilkrts_set_param("nworkers",std::to_string(threadsNumber).c_str());
      __cilkrts_init();
      return;
#endif
#ifdef HAVE_TBB
  case T_TBB:
    if(threadsNumber > 0)
      m_ptbb_scheduler.reset(new tbb::task_scheduler_init(threadsNumber));
    else
      m_ptbb_scheduler.reset(new tbb::task_scheduler_init());
    return;
#endif
  default:
    return;
  }
}

void ParallelScheduler::terminateBackend()
{
  switch(threadingBackend)
  {
    case T_Serial:
      return;
    case T_Std:
      return;
#ifdef HAVE_PTHREADS
    case T_Posix:
      return;
#endif
#ifdef HAVE_OPENMP
    case T_OpenMP:
      return;
#endif
#ifdef HAVE_CILK
    case T_Cilk:
      __cilkrts_end_cilk();
      return;
#endif
#ifdef HAVE_TBB
    case T_TBB:
      m_ptbb_scheduler->terminate();
      m_ptbb_scheduler.release();
      return;
#endif
    default:
      return;
  }
}

void ParallelScheduler::setThreadsNumberBackend(unsigned num)
{
  switch(threadingBackend)
  {
    case T_Serial:
      return;
    case T_Std:
      return;
#ifdef HAVE_PTHREADS
    case T_Posix:
      return;
#endif
#ifdef HAVE_OPENMP
    case T_OpenMP:
      omp_set_num_threads(num);
      return;
#endif
#ifdef HAVE_CILK
    case T_Cilk:
      __cilkrts_end_cilk();
      if(num > 0)
        __cilkrts_set_param("nworkers",std::to_string(num).c_str());
      __cilkrts_init();
      return;
#endif
#ifdef HAVE_TBB
    case T_TBB:
      m_ptbb_scheduler->terminate();
      if(num > 0)
        m_ptbb_scheduler->initialize(num);
      return;
#endif
    default:
      return;
  }
}

}
