# - Find OpenBlas library
# 
# This module defines
#  OPENBLAS_LIBRARY, libraries to link against to use Openblas.
#  OPENBLAS_FOUND, If false, do not try to use Openblas.

SET(OPEN_BLAS_SEARCH_PATHS  /lib/ /lib64/  /usr/lib /usr/lib64 /usr/local/lib /usr/local/lib64 /opt/OpenBLAS/lib $ENV{OPENBLAS_HOME}/lib /usr/lib/openblas-base)
FIND_LIBRARY(OPENBLAS_LIBRARY NAMES openblas PATHS ${OPEN_BLAS_SEARCH_PATHS})

# handle the QUIETLY and REQUIRED arguments and set OPENBLAS_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OpenBlas DEFAULT_MSG  OPENBLAS_LIBRARY)

MARK_AS_ADVANCED(OPENBLAS_LIBRARY)

