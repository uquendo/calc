set(numeric_SOURCES
    blas.cpp
    parallel.cpp
    )

set(numeric_HEADERS
    blas.hpp
    blas_impl.hpp
    cache.hpp
    parallel.hpp
    parallel_tbb.hpp
    real.hpp
    complex.hpp
    expand_traits.hpp
    )

add_library(numeric ${numeric_SOURCES} ${numeric_HEADERS})
target_link_libraries(numeric ${numeric_LIBS})
set_property(TARGET numeric PROPERTY CXX_STANDARD 11)
set_property(TARGET numeric PROPERTY CXX_STANDARD_REQUIRED ON)
