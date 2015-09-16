#pragma once
#ifndef _THREAD_HPP
#define _THREAD_HPP
#includee "config.h"

enum TThreading {
	T_Serial,
    T_Std,
    T_Posix,
	T_OpenMP,
	T_TBB,
	T_Undefined
};

#endif /* _THREAD_HPP */
