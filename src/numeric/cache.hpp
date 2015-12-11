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

  static constexpr inline std::size_t default_max_hw_vector_size()
  {
    return DEFAULT_MAX_HW_VECTOR_SIZE;
  }

  static constexpr inline std::size_t default_page_size()
  {
    return DEFAULT_PAGE_SIZE;
  }

  static constexpr inline std::size_t default_tlb_page_capacity()
  {
    return DEFAULT_TLB_PAGE_CAPACITY;
  }

  static constexpr inline std::size_t default_tlb_covered_size()
  {
    return DEFAULT_PAGE_SIZE * DEFAULT_TLB_PAGE_CAPACITY;
  }

  static constexpr inline std::size_t default_cache_line_size()
  {
//  return ( DEFAULT_CACHE_LINE_SIZE > alignof(std::max_align_t) ? DEFAULT_CACHE_LINE_SIZE : alignof(std::max_align_t) );
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

  inline std::size_t gcd_size(size_t a, size_t b) noexcept
  {
    size_t c;
    while( a != 0 )
    {
      c = a;
      a = b % a;
      b = c;
    }
    return b;
  }

  inline std::size_t lcm_size(size_t a, size_t b) noexcept
  {
    size_t c = gcd_size(a,b);
    return (a/c)*(b/c)*c;
  }

  inline void* cache_aligned_malloc(std::size_t __size, void** __buf) noexcept
  {
    return aligned_malloc(__size, cache_line_size(), __buf);
  }

  template<typename T> static constexpr size_t getDefaultAlignment()
  {
    return ( numeric::default_cache_line_size() > std::alignment_of<T>::value ? numeric::default_cache_line_size() : std::alignment_of<T>::value );
  }

  template<typename T> static size_t getCacheLineAlignment()
  {
    size_t line_size = numeric::cache_line_size();
    if(line_size <= getDefaultAlignment<T>())
      return getDefaultAlignment<T>();
    //assuring that our alignment is multiple of default one, so that we can safely assume all data aligned to default one
    if( ( line_size % getDefaultAlignment<T>() ) != 0)
    {
      //it's not very healthy situation btw
      line_size = numeric::lcm_size(line_size, getDefaultAlignment<T>());
    }
    return line_size ;
  }

  namespace aligned {

    template<typename T, std::size_t Alignment = getDefaultAlignment<T>()> constexpr size_t pack_size()
    {
      static_assert( Alignment % sizeof(T) == 0, "Alignment should be multiple of type size");
      return ( Alignment == 0 ? 1 : Alignment/sizeof(T) );
    }

/*
   //we're doomed! usage of alignas in typedef was forbidden in initial proposal from '05
   //actual standard for C++11 avoids direct discussion of this issue, so some compilers(GNU) support(?) it, some(Clang, Intel) don't

    template<typename T, std::size_t Alignment> using
      pack alignas(Alignment) = T[pack_size<T,Alignment>()];

    template<typename T, std::size_t Alignment> using
      type alignas(Alignment) = T ;

    template<typename T, int Alignment> using
      ptr = type<T,Alignment> * ;
*/
    //therefore we're stuck with struct
    template<typename T, std::size_t Alignment = getDefaultAlignment<T>()>
      struct raw_pack_storage {
        typedef struct {
            alignas(Alignment) unsigned char data[sizeof(T)*pack_size<T,Alignment>()];
        } type;
        static constexpr size_t size = pack_size<T,Alignment>();
    };

    //can hold pack_size() of T's
    template<typename T, std::size_t Alignment = getDefaultAlignment<T>()> using
      raw_pack = typename raw_pack_storage<T,Alignment>::type;

    template<typename T, std::size_t Alignment = getDefaultAlignment<T>()> using
      raw_pack_ptr = raw_pack<T,Alignment>*;

  }
}

#endif /* _CACHE_HPP */
