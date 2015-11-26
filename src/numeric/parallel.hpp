#pragma once
#ifndef _THREAD_HPP
#define _THREAD_HPP
#include "config.h"

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

}

#endif /* _THREAD_HPP */
