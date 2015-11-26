# - Try to check if nonstandard keyword to force inlining is available.
#
# Once done this will define
#
#  ForceInlineKeywordCXX_FOUND - we found keyword to force inlining in C++
#  FORCEINLINE_KEYWORD - the actual name of the keyword
#
# Copyright (c) 2015 Buzynin Nickolay <uquendo@gmail.com>
# Redistribution and use is allowed according to the terms of the 2-clause BSD license.


# a little test program for forced inline in c++.
set(FORCEINLINE_KEYWORD_TEST "${CMAKE_SOURCE_DIR}/cmake/test/forceinline.cpp")
set(CMAKE_TRY_COMPILE_CONFIGURATION "Release")

# first try compiling/linking the test program with GCC/Clang variant
try_compile(FORCEINLINE_TEST_COMPILES "${CMAKE_BINARY_DIR}/tmp" ${FORCEINLINE_KEYWORD_TEST}
    CMAKE_FLAGS "-DCMAKE_BUILD_TYPE=Release"
    COMPILE_DEFINITIONS "-DFORCEINLINE=__attribute__\\(\\(always_inline\\)\\)")

if(FORCEINLINE_TEST_COMPILES)
  set(FORCEINLINE_KEYWORD "__attribute__((always_inline))")
  set(ForceInlineKeywordCXX_FOUND TRUE)
else()
  #try MSVC variant
  try_compile(FORCEINLINE_TEST_COMPILES "${CMAKE_BINARY_DIR}/tmp" ${FORCEINLINE_KEYWORD_TEST}
      CMAKE_FLAGS "-DCMAKE_BUILD_TYPE=Release"
      COMPILE_DEFINITIONS "-DFORCEINLINE=__forceinline")
  if(FORCEINLINE_TEST_COMPILES)
    set(FORCEINLINE_KEYWORD "__forceinline")
    set(ForceInlineKeywordCXX_FOUND TRUE)
  endif()
endif()

