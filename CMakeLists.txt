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
#set(CMAKE_BUILD_TYPE "Release" CACHE STRING "For single-configuration generators (e.g. make) set the type of build: Release, Debug, RelWithDebugInfo, MinSizeRel")
#set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "Debug" "RelWithDebugInfo" "MinSizeRel")

#build options
option(BUILD_APPS           "build quest apps"         ON )
option(BUILD_THREADING      "build code that uses multithreading"  ON)
option(BUILD_FASTMATH       "enable non-IEEE754 compliant fp compiler optimisations(in Release mode)"   ON)
option(BUILD_QUAD           "enable quad precision(128 bit) support"    ON)
option(BUILD_VANILLA   	    "build without any external dependencies"  		OFF )
#option(BUILD_GPGPU         "build with GPGPU libraries"     OFF)

#threading options
cmake_dependent_option(USE_POSIX      	    "enable POSIX threads support"	    OFF "BUILD_THREADING" OFF)
cmake_dependent_option(USE_OPENMP      	    "enable OpenMP threads support"  	OFF "BUILD_THREADING" OFF)
cmake_dependent_option(USE_TBB      	    "enable Intel TBB threads support"	OFF "BUILD_THREADING; NOT BUILD_VANILLA" OFF)
cmake_dependent_option(USE_CILK      	    "enable Cilk Plus language extension"	OFF "BUILD_THREADING; NOT BUILD_VANILLA" OFF)

#utility libs options
cmake_dependent_option(USE_BOOST            "enable some utility libraries from Boost suite"  ON "NOT BUILD_VANILLA" OFF)
cmake_dependent_option(USE_TCMALLOC         "use google tcmalloc instead of default one"  OFF "NOT BUILD_VANILLA" OFF)

#math libs options
cmake_dependent_option(USE_MPREAL           "enable MPFR C++ interface support" OFF "NOT BUILD_VANILLA" OFF)

#blas variants(for benchmarks, comparsion and reference)
cmake_dependent_option(USE_OPENBLAS       "enable OpenBLAS library support" OFF "NOT BUILD_VANILLA" OFF)
cmake_dependent_option(USE_ACML           "enable AMD ACML library support" OFF "NOT BUILD_VANILLA" OFF)
cmake_dependent_option(USE_MKL            "enable Intel MKL library support" OFF "NOT BUILD_VANILLA" OFF)
cmake_dependent_option(USE_ATLAS          "enable ATLAS library support" OFF "NOT BUILD_VANILLA" OFF)
cmake_dependent_option(USE_MTL            "enable MTL4 library support" OFF "NOT BUILD_VANILLA" OFF)
cmake_dependent_option(USE_BOOST_UBLAS    "enable Boost uBLAS library support" OFF "NOT BUILD_VANILLA" OFF)

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
    #compiling with C++11 dialect is slightly more painful for cmake older than 3.1
    if("${CMAKE_VERSION}" VERSION_LESS "3.1")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
    endif()
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -march=native -mtune=native -mfpmath=sse -Wall -pedantic")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}  -O3 -march=native -mtune=native -mfpmath=sse -Wall -pedantic")
endif()


#build options

if(BUILD_THREADING)
    add_definitions("-DTHREADING")
endif()

if(BUILD_QUAD)
    if("${CMAKE_C_COMPILER_ID}" STREQUAL "Intel")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Qoption,cpp,--extended_float_type")
    endif()
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qoption,cpp,--extended_float_type")
    endif()
    add_definitions("-DQUAD")
endif()

if(BUILD_FASTMATH)
    if(MSVC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /fp:fast")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fast")
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
    add_definitions("-DUSE_PTHREADS")
	find_package (Threads)
endif()

if (USE_OPENMP)
    add_definitions("-DUSE_OPENMP")
endif()

if (USE_TBB)
    add_definitions("-DUSE_TBB")
endif()

if (USE_CILK)
    add_definitions("-DUSE_CILK")
endif()

#utility libs options

if (USE_BOOST)
    add_definitions("-DUSE_BOOST")
endif()

if (USE_TCMALLOC)
    find_package(tcmalloc)
endif()

#math libs options

if (USE_MPREAL)
    add_definitions("-DUSE_MPREAL")
endif()

#blas variants(for benchmarks, comparsion and reference)

if (USE_OPENBLAS)
    add_definitions("-DUSE_OPENBLAS")
endif()

if (USE_ACML)
    add_definitions("-DUSE_ACML")
endif()

if (USE_MKL)
    add_definitions("-DUSE_MKL")
endif()

if (USE_ATLAS)
    add_definitions("-DUSE_ATLAS")
endif()

if (USE_MTL)
    add_definitions("-DUSE_MTL")
endif()

if (USE_BOOST_UBLAS)
    add_definitions("-DUSE_BOOST_UBLAS")
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_definitions("-DBOOST_UBLAS_NDEBUG")
    endif()
endif()

print_enabled_features ( )

# final compiler setup

set (numeric_LIBS 
	${CMAKE_THREAD_LIBS_INIT}
)

set (calcapp_LIBS
    ${CMAKE_THREAD_LIBS_INIT}
)

#prepare general includes
add_subdirectory(include)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/include" "${CMAKE_SOURCE_DIR}/src")
#build generic parts
add_subdirectory(src)
#build task-specific apps
if (BUILD_APPS)
	add_subdirectory(apps)
endif()


