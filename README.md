[![Build Status](https://travis-ci.org/uquendo/calc.svg?branch=master)](https://travis-ci.org/uquendo/calc)
## calc
code for numerical methods course.

### description

Main objectives(from the course) were:
* create simple yet general enough portable framework for apps performing numerical computations(mostly cli)
* implement some standard numerical methods on top of it
* explore possibilities to archieve peak performance
* keep in mind numerical stability and precision issues
* code should be usable on variety of platforms and without any third party dependencies

Auxiliary objectives(self-driven) were:
* write all numerical algotithms as type-generic, desirable variant should be selected at runtime(i.e. via cli options)
* implement third party variants of numerical algorithms for comparsion of performance and simplicity of use
* implement all algorithms for different threading variants:
  serial, c++11 std::threads and/or pthreads(via hand written thread pool),  openmp, cilk, tbb
* implement all core numerical routines in standalone minimal library
* implement all core numerical routines in c++11/c11/fortran2008 for comparsion of compilers performance, code size and readability
* explore some not-so-widely-portable ways to boost performance: cpu intrinsics, tuning for cpu cache size etc.
* explore some recently standartized language support for memory alignment in c++11(alignas(), std::align etc),
  see how it influents compiler descisions about vectorization, compare with compiler-speciefic ways(i.e. __builtin_assume_aligned)

Some of the requirements(mostly auxiliary) still aren't met.

### install

#### requirements
* **Cmake** 2.8 or later
* **C++ compiler with C++11 support** - Compilers that have been tested include:
  * GCC 4.8.4 and later(GCC 4.9 or later is highly recommended), 
  * Clang 3.5 and later, 
  * Intel C/C++ Compiler 16 and later.
* **C11 and FORTRAN compilers** (still optional at the moment)
* **Boost** (optional, but highly recommended) - Version 1.54.0 or later for cli options handling mostly, used packages:
  * program_options (cli options)
  * system (used by filesystem)
  * filesystem (filesystem usage checks)
  * multiprecision (for quad and MPFR wrappers)

#### build
The code uses cmake as its build system. To prepare build, as usual, do:
```
$ git clone https://github.com/uquendo/calc.git
$ mkdir calc-build
$ cd calc-build
```
Cmake script uses optional variable **BUILD_VARIANT** to select predefined set of options, allowed values for it are:
* basic (standard build with boost and quadmath support enabled if found)
* vanilla (build without any external dependencies)
* full (build with all optional external dependencies)

So, to prepare, for example, vanilla build, one should say:
```
$ BUILD_VARIANT=vanilla cmake ../calc
```

In order to select build configuration one can also use standard ncurses cmake interface:
```
$ ccmake ../calc
```
After preparing the build, do:
```
$ make
```

#### optional dependencies
threading:
* **POSIX threads**
* **OpenMP**
* **TBB**
* **CILK** (currently disabled for intel compiler due to compiler bug)

multiprecision:
* **MPFR**

math:
* **MTL**
* **Eigen**
* **ARMADILLO**
* **GSL**
* **BLAS** , any of: Generic, Open, Goto, ATLAS, PhiPACK, CXML, DXML, SunPerf, SCSL, SGIMATH, IBMESSL, IntelMKL, ACML, ACML_MP, ACML_GPU, Apple, NAS

utility:
* **tcmalloc**

To force some speciefic BLAS variant, use environment variable BLAS_VENDOR.

#### usage

If built with boost::program_options, -h option should give a clue, for example:
```
calc-build-full-intel> ./bin/quest01-1 -h
Allowed options:

Common options:
  -h [ --help ]                print help message
  -V [ --version ]             print build and version information
  -v [ --verbose ] arg (=4)    log verbosity level, 0..6
  -L [ --logfile ] arg         specify name of the log file
  -l [ --threading ] arg (=s)  Threading model to use: s=Serial, std=C++11 
                               standard library threads, omp=OpenMP, cilk=Intel
                               CILK, tbb=Intel TBB
  -t [ --threads ] arg (=0)    number of threads to use. 0=auto
  -p [ --precision ] arg (=64) Floating point number precision to use: 
                               32=32-bit float, 64=64-bit double, 80=80-bit 
                               long double, 128=128-bit quad, mpfr=fixed 
                               decimal digits MPFR
  -d [ --digits ] arg (=10)    MPFR number of decimal digits to use

Input options:
  -i [ --in ] arg (=dat)     Input file format to use: 
                             dat= Read matrices in .dat format,
                             csv= Read matrices in .csv format
  -A [ --in-A ] arg (=data1) name of matrix A file(without an extension if 
                             format is specified)
  -B [ --in-B ] arg (=data2) name of matrix B file(without an extension if 
                             format is specified)

Output options:
  -o [ --out ] arg (=dat)      Output file format to use: 
                               dat= Write matrices in .dat format,
                               csv= Write matrices in .csv format
  -C [ --out-C ] arg (=result) name of matrix C(results) file(without an 
                               extension if format is specified)

Algorithm options:
  -s [ --algorithm ] arg (=num-cpp) Computation algorithm to use: 
                                    num-cpp= numeric-cpp,
                                    num-c= numeric-c,
                                    num-f= numeric-fortran,
                                    num-cpp-s= numeric-cpp-simple,
                                    num-c-s= numeric-c-simple,
                                    num-f-s= numeric-fortran-simple,
                                    num-cpp-st= numeric-cpp-simple-transpose,
                                    num-c-st= numeric-c-simple-transpose,
                                    num-f-st= numeric-fortran-simple-transpose,
                                    num-cpp-v= numeric-cpp-valarray,
                                    num-cpp-vt= numeric-cpp-valarray-transpose,
                                    num-cpp-b= numeric-cpp-block,
                                    num-cpp-sn= numeric-cpp-strassen,
                                    num-c-sn= numeric-c-strassen,
                                    num-f-sn= numeric-fortran-strassen,
                                    num-f-int= numeric-fortran-internal,
                                    ext-cpp-boost= contrib-cpp-boost-ublas,
                                    ext-cpp-eigen= contrib-cpp-eigen,
                                    ext-cpp-mtl= contrib-cpp-mtl,
                                    ext-cpp-arm= contrib-cpp-armadillo,
                                    ext-c-blas= contrib-c-blas(BLAS variant: 
                                    All),
                                    ext-f-blas= contrib-fortran-blas
                                    Note:
                                    * check --version for available threading 
                                    options per algorithm
                                    
```

Vanilla build sets all options but algorithm to their defaults at the moment, for example:
```
calc-build-vanilla-gcc> ./bin/quest02 -h
App was compiled without boost::program_options, so no fancy cli options yet.
Only very limited subset of options is supported at the moment. sorry.
Run without any options to use default parameters.

Allowed options:
Common options:
  -h [ --help ]                print help message
  -V [ --version ]             print build and version information

Algorithm options:
  -s [ --algorithm ] arg (=num-cpp-che)
Computation algorithm to use: 
num-cpp-che= libnumeric c++ variant for Chebyshev grid,
num-cpp-uni= libnumeric c++ variant for uniform grid

```
Option **--version** lists system, verion and build information:
```
calc-build-full-intel> ./bin/quest01-1 --version
quest01-1 version 0.1

System information:
running on Linux 3.13.0-49-generic #83-Ubuntu SMP Fri Apr 10 20:11:33 UTC 2015 x86_64; libc version: 2.19
detected cpu : Intel(R) Core(TM) i5-2430M CPU @ 2.40GHz
detected 4 cpu cores

Build information:
build toolchain : Intel 16.0.0.20150815
build type : Release
git sha1 : 1df4d73748012480165b3214b8a744f817883f63
build options :  THREADING QUAD FASTMATH APPS
third parties options :  QUADMATH PTHREADS OPENMP TBB CILK BOOST MPREAL BOOST_MPREAL BLAS MTL BOOST_UBLAS EIGEN ARMADILLO GSL
CMAKE_C_FLAGS:  -std=gnu11 -Wall -pedantic  -Qoption,cpp,--extended_float_type -openmp   -std=gnu11 -Wall -pedantic  -Qoption,cpp,--extended_float_type -openmp  -std=gnu11 -Wall -pedantic  -Qoption,cpp,--extended_float_type -openmp  -std=gnu11 -Wall -pedantic  -Qoption,cpp,--extended_float_type -openmp  -std=gnu11 -Wall -pedantic  -Qoption,cpp,--extended_float_type -openmp  -O3 -DNDEBUG -march=native -mtune=native -mfpmath=sse -ipo -restrict -xHost -static-intel -no-prec-div -fp-model fast=2 -unroll-aggressive -march=native -mtune=native -mfpmath=sse -ipo -restrict -xHost -static-intel -no-prec-div -fp-model fast=2 -unroll-aggressive -march=native -mtune=native -mfpmath=sse -ipo -restrict -xHost -static-intel -no-prec-div -fp-model fast=2 -unroll-aggressive -march=native -mtune=native -mfpmath=sse -ipo -restrict -xHost -static-intel -no-prec-div -fp-model fast=2 -unroll-aggressive -march=native -mtune=native -mfpmath=sse -ipo -restrict -xHost -static-intel -no-prec-div -fp-model fast=2 -unroll-aggressive
CMAKE_CXX_FLAGS:  -std=gnu++11 -Wall -pedantic  -Qoption,cpp,--extended_float_type -openmp   -std=gnu++11 -Wall -pedantic  -Qoption,cpp,--extended_float_type -openmp  -std=gnu++11 -Wall -pedantic  -Qoption,cpp,--extended_float_type -openmp  -std=gnu++11 -Wall -pedantic  -Qoption,cpp,--extended_float_type -openmp  -std=gnu++11 -Wall -pedantic  -Qoption,cpp,--extended_float_type -openmp  -O3 -DNDEBUG -march=native -mtune=native -mfpmath=sse -ipo -restrict -xHost -static-intel  -no-prec-div -fp-model fast=2 -unroll-aggressive -march=native -mtune=native -mfpmath=sse -ipo -restrict -xHost -static-intel  -no-prec-div -fp-model fast=2 -unroll-aggressive -march=native -mtune=native -mfpmath=sse -ipo -restrict -xHost -static-intel  -no-prec-div -fp-model fast=2 -unroll-aggressive -march=native -mtune=native -mfpmath=sse -ipo -restrict -xHost -static-intel  -no-prec-div -fp-model fast=2 -unroll-aggressive -march=native -mtune=native -mfpmath=sse -ipo -restrict -xHost -static-intel  -no-prec-div -fp-model fast=2 -unroll-aggressive
extra compilation options: -fast -cilk-serialize

run with -h or --help to see available options
```

If **--verbose** is supplied with high enough values, one should finaly see something like this:
```
0.000 DEBUG: Command line: ../../../../../calc-build-basic-gcc/bin/quest01-1 -A data1024 -B data1024 --verbose=6 --precision=64 --algorithm=num-cpp-b 
0.001 DEBUG: Launch time: Fri Dec 25 05:11:47 2015

0.004 DEBUG: Running dense matrix multiplication algorithm numeric-cpp-block (Serial variant) using 64-bit double precision
0.004 DEBUG: creating input matrices...
0.004 DEBUG: found input matrices: A ( 1024 x 1024 ), B ( 1024 x 1024 )
0.004 DEBUG: sanity check of matrix sizes...
0.004 DEBUG: creating output matrix C ( 1024 x 1024 )...
0.008 DEBUG: reading input matrices...
0.148 DEBUG: 
0.148 DEBUG: running main task...
0.321 DEBUG: PERF: matmul::perform: 0.173000 seconds
0.321 DEBUG: writing output matrix...
0.891 DEBUG: have a nice day.
```
