#pragma once
#ifndef _PARALLEL_PROGRESS_HPP
#define _PARALLEL_PROGRESS_HPP
#include "config.h"

#include "numeric/parallel.hpp"
#include "calcapp/progress.hpp"

# ifdef HAVE_TBB
#include "numeric/parallel_tbb.hpp"

#include <tbb/atomic.h>


namespace CalcApp {

template <class Index, class ElemFunction>
class ParallelBlockExecutorWithProgressBar {
public:
  typedef long Counter;
  typedef tbb::atomic<Counter> AtomicCounter;

  static const Counter PROGRESS_BAR_TICKS = 10;

  ParallelBlockExecutorWithProgressBar(long tick, AtomicCounter& counter, AtomicCounter& updateLock, ProgressBar * pb, ElemFunction const & f) 
    : tick_(tick), counter_(counter), updateLock_(updateLock), f_(f), pb_(pb)
  {
    updateLock_ = 0;
  }

  void operator () (const tbb::blocked_range<Index>& br) const {
    Counter curTick = tick_;

    for ( Index i = br.begin(); i < br.end(); ++i ) {
      // Call function
      f_(i);

      // Do progress bar maintenance
      if ( --curTick <= 0 ) {
        updateProgress(tick_);
        curTick = tick_;
      }
    }

    updateProgress(tick_ - curTick);
  }

  void updateProgress(Counter increment) const {
    Counter progress = (counter_ += increment);

    if ( ++updateLock_ == 1 )
      pb_->setProgress(progress);

    --updateLock_;
  }

private:
  const ElemFunction & f_;
  ProgressBar * pb_;
  AtomicCounter& counter_;
  AtomicCounter& updateLock_;
  Counter tick_;
};

template <class Index, class ElemFunction>
void parallelForElem(Index first, Index last, ElemFunction const & f, ProgressBar * pb) 
{
  typedef ParallelBlockExecutorWithProgressBar<Index, ElemFunction> Exec;

  typename Exec::Counter distance = last - first;
  pb->setProgressRange(0, distance);
  pb->setProgress(0);

  typename Exec::AtomicCounter counter, updateLock;
  counter = 0;

  parallelForBlock(first, last, Exec(distance / Exec::PROGRESS_BAR_TICKS, counter, updateLock, pb, f));

  pb->setProgress(distance);
}

}

# endif

#endif /* _PARALLEL_PROGRESS_HPP */
