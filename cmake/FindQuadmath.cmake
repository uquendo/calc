# - Try to find GNU libquadmath 

#search path collection linux-speciefic by now
#TODO: MSVC/MinGW and OS X 
set(QUADMATH_PATHS /usr/lib
               /usr/lib64
               /usr/local/lib
               /usr/local/lib64
               /lib/x86_64-linux-gnu
               /usr/lib/x86_64-linux-gnu
               /usr/lib/gcc/x86_64-linux-gnu/${CMAKE_CXX_COMPILER_VERSION}
               /usr/lib/gcc/x86_64-linux-gnu/5
               /usr/lib/gcc/x86_64-linux-gnu/4.9
               /usr/lib/gcc/x86_64-linux-gnu/4.8
               /usr/lib/gcc/x86_64-linux-gnu/4.7
               /usr/lib/gcc/x86_64-linux-gnu/4.6
               )

find_library(QUADMATH_LIB NAMES quadmath PATHS ${QUADMATH_PATHS})
if(QUADMATH_LIB)
  set(QUADMATH_FOUND TRUE)
endif()


