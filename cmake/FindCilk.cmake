# ============================================================================
#  Copyright (c) 2013-2015, Yan Zhou
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#    Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#
#    Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS AS IS
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
# ============================================================================

# Find Intel Cilk Plus support
#
# The following variable is set
#
# CILK_FOUND          - TRUE if Cilk Plus is found and work correctly
# Cilk_CXX_FLAGS      - Flags to add to the CXX compiler for Cilk Plus support

IF (DEFINED CILK_FOUND)
    RETURN ()
ENDIF (DEFINED CILK_FOUND)

FILE (READ "${CMAKE_SOURCE_DIR}/cmake/test/cilk.cpp" CILK_TEST_SOURCE)

INCLUDE (CheckCXXSourceRuns)

SET (Cilk_FLAG_CANDIDATES
    " "          # Intel
    "-fcilkplus" # GNU or Clang
    )

IF(${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
  SET(Cilk_FLAG_CANDIDATES " ")
ELSEIF(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  SET (Cilk_FLAG_CANDIDATES "-fcilkplus" )
ENDIF()

FOREACH (flag ${Cilk_FLAG_CANDIDATES})
    IF (NOT CILK_FOUND)
        SET (SAFE_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
        SET (CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS} ${flag})
        UNSET (CILK_FOUND CACHE)
        MESSAGE (STATUS "Try Cilk Plus flag = [${flag}]")
        CHECK_CXX_SOURCE_RUNS ("${CILK_TEST_SOURCE}" CILK_FOUND)
        SET (CMAKE_REQUIRED_FLAGS ${SAFE_CMAKE_REQUIRED_FLAGS})
        IF (CILK_FOUND)
            SET (Cilk_CXX_FLAGS ${flag})
        ENDIF (CILK_FOUND)
    ENDIF (NOT CILK_FOUND)
ENDFOREACH (flag ${Cilk_FLAG_CANDIDATES})

IF (CILK_FOUND)
    MESSAGE (STATUS "Found Cilk Plus support [${Cilk_CXX_FLAGS}]")
ELSE (CILK_FOUND)
    MESSAGE (STATUS "NOT Found Cilk Plus support")
ENDIF (CILK_FOUND)

