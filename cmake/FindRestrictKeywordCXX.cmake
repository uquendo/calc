# - Try to check if nonstandard for C++ keyword "restrict" is available.
# Keyword "restrict" is defined in C99 but not yet present in any C++ standard.
# Most compiler have nonstandard replacement like "__restrict" though.
#
# Once done this will define
#
#  RestrictKeywordCXX_FOUND - we found how to successfully use "restrict" keyword in C++
#  RESTRICT_KEYWORD - the actual name of the keyword
#
# Copyright (c) 2015 Buzynin Nickolay <uquendo@gmail.com>
# Redistribution and use is allowed according to the terms of the 2-clause BSD license.


# a little test program for keyword __restrict in c++.
set(RESTRICT_KEYWORD_TEST "${CMAKE_SOURCE_DIR}/cmake/test/restrict.cpp")
set(CMAKE_TRY_COMPILE_CONFIGURATION "Release")

# first try compiling/linking the test program with __restrict variant(seems to be the most popular one)
try_compile(RESTRICT_TEST_COMPILES "${CMAKE_BINARY_DIR}/tmp" ${RESTRICT_KEYWORD_TEST}
    CMAKE_FLAGS "-DCMAKE_BUILD_TYPE=Release"
    COMPILE_DEFINITIONS "-DRESTRICT=__restrict")


if(RESTRICT_TEST_COMPILES)
  set(RESTRICT_KEYWORD "__restrict")
  set(RestrictKeywordCXX_FOUND TRUE)
else()
  try_compile(RESTRICT_TEST_COMPILES "${CMAKE_BINARY_DIR}/tmp" ${RESTRICT_KEYWORD_TEST}
      CMAKE_FLAGS "-DCMAKE_BUILD_TYPE=Release"
      COMPILE_DEFINITIONS "-DRESTRICT=__restrict__")
  if(RESTRICT_TEST_COMPILES)
    set(RESTRICT_KEYWORD "__restrict__")
    set(RestrictKeywordCXX_FOUND TRUE)
  else()
    try_compile(RESTRICT_TEST_COMPILES "${CMAKE_BINARY_DIR}/tmp" ${RESTRICT_KEYWORD_TEST}
        CMAKE_FLAGS "-DCMAKE_BUILD_TYPE=Release"
        COMPILE_DEFINITIONS "-DRESTRICT=restrict")
    if(RESTRICT_TEST_COMPILES)
      set(RESTRICT_KEYWORD "restrict")
      set(RestrictKeywordCXX_FOUND TRUE)
    else()
        set(RESTRICT_KEYWORD "")
        set(RestrictKeywordCXX_FOUND FALSE)
    endif()
  endif()
endif()

