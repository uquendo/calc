
configure_file("config.h.in" "config.h")
if(MSVC)
    configure_file("stdafx.h.in" "stdafx.h")
endif()
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
