cmake_minimum_required(VERSION 2.8)

include(CMakeDependentOption)
include(FeatureSummary)

project(numeric)

#set path for local find modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

#set output path
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# standard build targets
# For non-multi-configuration generators (eg, make, Eclipse)
# The Visual Studio and XCode generators create a single project with all these
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "For single-configuration generators (e.g. make) set the type of build: Release, Debug, RelWithDebugInfo, MinSizeRel")
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "Debug" "RelWithDebugInfo" "MinSizeRel")

#build options
option(BUILD_APPS           "build quest apps"         ON )
option(BUILD_THREADING      "build code that uses multithreading"  ON)
option(BUILD_FASTMATH       "enable non-IEEE754 compliant fp compiler optimisations(in Release mode)"   ON)
option(BUILD_QUAD           "enable quad precision(128 bit) support"    ON)
option(BUILD_VANILLA   	    "build without any external dependencies"  		OFF )
#option(BUILD_GPGPU         "build with GPGPU libraries"     OFF)

#threading options
cmake_dependent_option(USE_PTHREADS    	    "enable POSIX threads support"	    OFF "BUILD_THREADING" OFF)
cmake_dependent_option(USE_OPENMP      	    "enable OpenMP threads support"  	OFF "BUILD_THREADING" OFF)
cmake_dependent_option(USE_TBB      	    "enable Intel TBB threads support"	OFF "BUILD_THREADING; NOT BUILD_VANILLA" OFF)
cmake_dependent_option(USE_CILK      	    "enable Cilk Plus language extension"	OFF "BUILD_THREADING; NOT BUILD_VANILLA" OFF)

#utility libs options
cmake_dependent_option(USE_BOOST            "enable some utility libraries from Boost suite(filesystem, system, program_options, date_time)"  ON "NOT BUILD_VANILLA" OFF)
cmake_dependent_option(USE_TCMALLOC         "use google tcmalloc instead of default one"  OFF "NOT BUILD_VANILLA" OFF)

#math libs options
cmake_dependent_option(USE_MPREAL           "enable MPFR C++ interface support" OFF "NOT BUILD_VANILLA" OFF)

#blas variants(for benchmarks, comparsion and reference)
cmake_dependent_option(USE_BLAS           "enable blas-conforming library support(set BLAS_VENDOR for specific variant)" OFF "NOT BUILD_VANILLA" OFF)
set(BLAS_VENDOR "All" CACHE STRING "if USE_BLAS set, find and use specific vendor of blas library, one of: Any,Generic,Open,Goto,ATLAS,PhiPACK,CXML,DXML,SunPerf,SCSL,SGIMATH,IBMESSL,Intel10_32 (intel mkl v>=10 32 bit),Intel10_64lp (intel mkl v>=10 64 bit,lp thread model, lp64 model), Intel10_64lp_seq (intel mkl v>=10 64 bit,sequential code, lp64 model), Intel( older versions of mkl 32 and 64 bit), ACML,ACML_MP,ACML_GPU,Apple,NAS")
set_property(CACHE BLAS_VENDOR PROPERTY STRINGS "All" "Generic" "Open" "Goto" "ATLAS" "PhiPACK" "CXML" "DXML" "SunPerf" "SCSL" "SGIMATH" "IBMESSL" "Intel10_32" "Intel10_64lp" "Intel10_64lp_seq" "Intel" "ACML" "ACML_MP" "ACML_GPU" "Apple" "NAS" )
#templated blas analogs
cmake_dependent_option(USE_MTL            "enable MTL4 library support" OFF "NOT BUILD_VANILLA" OFF)
cmake_dependent_option(USE_BOOST_UBLAS    "enable Boost uBLAS library support" OFF "NOT BUILD_VANILLA" OFF)
cmake_dependent_option(USE_EIGEN          "enable Eigen library support" OFF "NOT BUILD_VANILLA" OFF)

#gpgpu blas variants
#cmake_dependent_option(USE_CUBLAS      "enable NVIDIA cuBLAS CUDA library support" OFF "NOT BUILD_VANILLA" OFF)
#cmake_dependent_option(USE_NVBLAS      "enable NVIDIA NVBLAS CUDA library support" OFF "NOT BUILD_VANILLA" OFF)
#cmake_dependent_option(USE_CLBLAS      "enable AMD clBLAS OpenCL library support" OFF "NOT BUILD_VANILLA" OFF)


#generic compiler options
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions("-DNDEBUG")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Wall ")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Wall ")
else()
    #compiling with C++11 dialect is slightly more painful for cmake versions older than 3.1
    if("${CMAKE_VERSION}" VERSION_LESS "3.1")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
    endif()
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -march=native -mtune=native -mfpmath=sse -Wall -pedantic")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}  -O3 -march=native -mtune=native -mfpmath=sse -Wall -pedantic")
endif()

find_package(StandardMathLibrary REQUIRED)
if(StandardMathLibrary_FOUND)
    set(common_LIBS ${common_LIBS} ${STANDARD_MATH_LIBRARY})
else()
    message("Can't link to the standard math library.")
endif()

find_package(RestrictKeywordCXX)
if(RestrictKeywordCXX_FOUND)
    set(HAVE_RESTRICT YES)
else()
    message("Can't find any usable variant of restrict keyword")
endif()

set(app_LIBS numeric calcapp)

#build options

if(BUILD_THREADING)
endif()

if(BUILD_QUAD)
    if("${CMAKE_C_COMPILER_ID}" STREQUAL "Intel")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Qoption,cpp,--extended_float_type")
    endif()
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qoption,cpp,--extended_float_type")
    endif()
endif()

if(BUILD_FASTMATH)
    if(MSVC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /fp:fast")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:fast")
    else()
        if("${CMAKE_C_COMPILER_ID}" STREQUAL "Intel")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fast")
        else()
            set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast")
        endif()
        if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fast")
        else()
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast")
        endif()
    endif()
endif()

#threading options

if (USE_PTHREADS)
	find_package(Threads)
    if(Threads_FOUND AND CMAKE_USE_PTHREADS_INIT)
        set(common_LIBS ${common_LIBS} ${CMAKE_THREAD_LIBS_INIT})
        set(HAVE_PTHREADS YES)
    else()
        message("Can't find pthreads")
    endif()
endif()

if (USE_OPENMP)
  find_package(OpenMP)
  if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(HAVE_OPENMP YES)
  else()
    message("Can't find OpenMP")
  endif()
endif()

if (USE_TBB)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(TBB_USE_DEBUG_BUILD TRUE)
  else()
    set(TBB_USE_DEBUG_BUILD FALSE)
  endif()
  find_package(TBB)
  if(TBB_FOUND)
    include_directories(${TBB_INCLUDE_DIRS})
    set(common_LIBS ${common_LIBS} ${TBB_LIBRARIES})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TBB_DEFINITIONS}")
    set(HAVE_TBB YES)
  else()
    message("Can't find Intel TBB libraries")
  endif()
endif()

if (USE_CILK)
  find_package(Cilk)
  if(CILK_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${Cilk_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Cilk_CXX_FLAGS}")
    set(HAVE_CILK YES)
  else()
    message("Can't find Intel Cilk Plus extension")
  endif()
endif()

#utility libs options

if (USE_BOOST)
   find_package(Boost COMPONENTS program_options date_time filesystem system)
   if(Boost_FOUND)
     include_directories(${Boost_INCLUDE_DIRS})
     set(calcapp_LIBS ${Boost_LIBRARIES} ${calcapp_LIBS})
     set(app_LIBS ${Boost_LIBRARIES} ${app_LIBS})
     set(HAVE_BOOST YES)
   else()
     message(FATAL_ERROR "Can't find Boost library")
   endif()
endif()

if (USE_TCMALLOC)
    find_package(tcmalloc)
    if(TCMALLOC_FOUND)
        include_directories(${TCMALLOC_INCLUDE_DIRS})
        set(common_LIBS ${common_LIBS} ${TCMALLOC_LIBRARIES})
    else()
        message("Can't find tcmalloc")
    endif()
endif()

#math libs options

if (USE_MPREAL)
    find_package(GMP)
    find_package(MPFR)
    if(GMP_FOUND AND MPFR_FOUND)
        include_directories(${GMP_INCLUDES})
        include_directories(${MPFR_INCLUDES})
        include_directories("${CMAKE_SOURCE_DIR}/contrib/mpfrc++")
        set(common_LIBS ${common_LIBS} ${GMP_LIBRARIES} ${MPFR_LIBRARIES})
        set(HAVE_MPREAL YES)
    else()
        message("Can't use mpreal wrapper due to missing prerequisites")
    endif()
endif()

#blas variants(for benchmarks, comparsion and reference)
if (USE_BLAS)
  if(BLAS_VENDOR STREQUAL "Open")
    find_package(OpenBlas)
    if(OPENBLAS_FOUND)
      set(common_LIBS ${common_LIBS} ${OPENBLAS_LIBRARY})
      set(HAVE_BLAS YES)
    else()
      message("Can't find blas library")
    endif()
  else()
    set(BLA_VENDOR ${BLAS_VENDOR})
    find_package(BLAS)
    if(BLAS_FOUND)
      set(common_LIBS ${common_LIBS} ${BLAS_LIBRARIES})
      set(HAVE_BLAS YES)
    else()
      message("Can't find blas library")
    endif()
  endif()
endif()

#templated blas analogs
if (USE_MTL)
  find_package(MTL4)
  if(MTL4_FOUND)
    include_directories(${MTL4_INCLUDE_DIRS})
    set(common_LIBS ${common_LIBS} ${MTL4_LIBRARIES})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MTL4_DEFINITIONS}")
    set(HAVE_MTL YES)
  else()
    message("Can't find Matrix Template Library")
  endif()
endif()

if (USE_BOOST_UBLAS)
  if(NOT Boost_FOUND)
    find_package(Boost)
  endif()
  if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    set(HAVE_BOOST_UBLAS YES)
  else()
    message("Can't find Boost.uBLAS headers")
  endif()
endif()

if(USE_EIGEN)
  find_package(Eigen3)
  if(EIGEN3_FOUND)
    include_directories(${EIGEN3_INCLUDE_DIR})
    set(HAVE_EIGEN YES)
  else()
    message("Can't find Eigen headers")
  endif()
endif()

feature_summary(WHAT ALL)

# final compiler setup

set (numeric_LIBS ${common_LIBS})
set (calcapp_LIBS ${common_LIBS} ${calcapp_LIBS})
set (app_LIBS ${common_LIBS} ${app_LIBS})

#prepare general includes
add_subdirectory(include)
include_directories("${CMAKE_BINARY_DIR}/include" "${CMAKE_SOURCE_DIR}/src")
#build generic parts
add_subdirectory(src)
#build task-specific apps
if (BUILD_APPS)
	add_subdirectory(apps)
endif()


