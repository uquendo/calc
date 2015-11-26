#pragma once
#ifndef _CACHE_HPP
#define _CACHE_HPP
#include "config.h"

#include <cstdint>
#include <cstddef>

namespace numeric {
  // std::align from gnu libgstdc++ trunk
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

}

#endif /* _CACHE_HPP */
