set(APP_NAME quest01-2)
set(APP_VERSION 0.1)
set(APP_SRCNAME quest)

configure_file("appconfig.h.in" "appconfig.h")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

set(APP_SOURCES
    ${APP_SRCNAME}.cpp
    matmul.cpp
    main.cpp
    )

set(APP_HEADERS
    ${APP_SRCNAME}.hpp
    matmul.hpp
    )

add_executable(${APP_NAME} ${APP_SOURCES} ${APP_HEADERS})
set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(${APP_NAME} ${app_LIBS})
