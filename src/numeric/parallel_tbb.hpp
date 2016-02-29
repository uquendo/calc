#pragma once
#ifndef _PARALLEL_TBB_HPP
#define _PARALLEL_TBB_HPP

#include "config.h"

#include "numeric/parallel.hpp"

#ifdef HAVE_TBB

#include "tbb/parallel_for.h"
#include "tbb/parallel_reduce.h"

namespace numeric
{

template <class Range, class BlockFunction>
void parallelForBlock(const Range & range, BlockFunction const & f)
{
  if ( ParallelScheduler::getThreadsNumber() > 1 )
    tbb::parallel_for(range, f);
  else
    f(range);
}

template <class Index, class BlockFunction>
void parallelForBlock(Index first, Index last, BlockFunction const & f)
{
  tbb::blocked_range<Index> br(first, last);
  if ( ParallelScheduler::getThreadsNumber() > 1 )
    tbb::parallel_for(br, f);
  else
    f(br);
}

template <class Index, class BlockFunction>
void parallelForBlock(Index first, Index last, BlockFunction const & f, Index grainSize)
{
  tbb::blocked_range<Index> br(first, last, grainSize);
  if ( ParallelScheduler::getThreadsNumber() > 1 )
    tbb::parallel_for(br, f);
  else
    f(br);
}

template <class Index, class BlockFunction>
void parallelReduceBlock(Index first, Index last, BlockFunction const & f)
{
  tbb::blocked_range<Index> br(first, last);
  if ( ParallelScheduler::getThreadsNumber() > 1 )
    tbb::parallel_reduce(br, f);
  else
    f(br);
}

template <class Range, class BlockFunction>
void parallelReduceBlock(const Range & range, BlockFunction & f)
{
  if ( ParallelScheduler::getThreadsNumber() > 1 )
    tbb::parallel_reduce(range, f);
  else
    f(range);
}

template <class Index, class ElemFunction>
void parallelForElem(Index first, Index last, ElemFunction const & f) 
{
  parallelForBlock(first, last,
    [&f](const tbb::blocked_range<Index>& br) {
      for ( Index i = br.begin(); i < br.end(); ++i )
        f(i);
    }
  );
}

template <class Index, class ElemFunction>
void parallelForElem(Index first, Index last, ElemFunction & f) 
{
  parallelForBlock(first, last,
    [&f](const tbb::blocked_range<Index>& br) {
      for ( Index i = br.begin(); i < br.end(); ++i )
        f(i);
    }
  );
}

}

#endif /* HAVE_TBB */

#endif /* _PARALLEL_TBB_HPP */
