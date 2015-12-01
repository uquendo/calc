#pragma once
#ifndef _CACHE_HPP
#define _CACHE_HPP
#include "config.h"

//unique_ptr
#include <memory>

//c types for std::align
#include <cstdint>
#include <cstddef>

//aligned_alloc
#include "stdlib.h"
#include "malloc.h"

//headers for runtime cache line size detection
#if defined(_WIN32)
# include <windows.h>
#elif defined(__linux__)
# include <unistd.h>
#elif defined(__APPLE__)
# include <sys/sysctl.h>
#endif

//intrinsics
#if defined(_MSC_VER)
  #include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
  #include <x86intrin.h>
#endif

namespace numeric {

  // std::align borrowed from gnu libgstdc++ trunk
  inline void* align(std::size_t __align, std::size_t __size, void*& __ptr, std::size_t& __space) noexcept
  {
    const auto __intptr = reinterpret_cast<std::uintptr_t>(__ptr);
    const auto __aligned = (__intptr - 1u + __align) & -__align;
    const auto __diff = __aligned - __intptr;
    if ((__size + __diff) > __space)
      return nullptr;
    else
      {
        __space -= __diff;
        return __ptr = reinterpret_cast<void*>(__aligned);
      }
  }

  //aligned_malloc wrapper. __buf should be used to free memory(if __buf != <returned value>)
  inline void* aligned_malloc(std::size_t __size, std::size_t __align, void** __buf) noexcept
  {
    void* __aligned;
#if _MSC_VER >= 1400
    __aligned = _aligned_malloc(__size, __align);
    *__buf = __aligned;
// turns out, aligned_malloc from C11 requires size to be multiple of align. what a pity.
//#elif defined(__GNUC__) && defined(_ISOC11_SOURCE)
//    __aligned = aligned_malloc(__align, __size);
//    __buf = __aligned;
#elif _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600 || HAVE_POSIX_MEMALIGN
    if(posix_memalign(&__aligned, __align, __size)) __aligned = nullptr;
    *__buf = __aligned;
#else
   size_t __bufsize = size+align-1;
   *__buf = malloc(__bufsize);
   __aligned = align(__align, __size, *__buf, __bufsize);
  if(__aligned == nullptr)
  {
    free(*__buf);
    *__buf = nullptr;
  }
#endif
    return __aligned;
  }

  //aligned_free wrapper. should be called with __buf from function above(if __buf != __aligned)
  inline void aligned_free(void* ptr) noexcept
  {
#ifdef _MSC_VER 
    _aligned_free(ptr);
#else 
    free(ptr);
#endif
  }

  struct AlignedBufDeleter
  {
    void operator()(void* p) const noexcept
    {
      aligned_free(p);
    }
  };

  typedef std::unique_ptr<void,AlignedBufDeleter> unique_aligned_buf_ptr;

  static constexpr inline std::size_t default_cache_line_size()
  {
    return DEFAULT_CACHE_LINE_SIZE ;
  }


  inline std::size_t cache_line_size() noexcept
  {
    std::size_t line_size = 0;

#if defined(_WIN32)

    DWORD buffer_size = 0;
    DWORD i = 0;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION * buffer = 0;

    GetLogicalProcessorInformation(0, &buffer_size);
    buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *)malloc(buffer_size);
    GetLogicalProcessorInformation(&buffer[0], &buffer_size);

    for (i = 0; i != buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
        if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) {
            line_size = buffer[i].Cache.LineSize;
            break;
        }
    }

    free(buffer);

#elif defined(__linux__)
    long sysconf_line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    if(sysconf_line_size != -1)
      line_size = sysconf_line_size;
#elif defined(__APPLE__)
    std::size_t sizeof_line_size = sizeof(line_size);
    sysctlbyname("hw.cachelinesize", &line_size, &sizeof_line_size, 0, 0);
#endif
    if(line_size > 0)
      return line_size;
    else
      return default_cache_line_size();
  }

  inline void* cache_aligned_malloc(std::size_t __size, void** __buf) noexcept
  {
    return aligned_malloc(__size, cache_line_size(), __buf);
  }

}

#endif /* _CACHE_HPP */
