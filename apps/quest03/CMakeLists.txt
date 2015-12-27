set(APP_NAME_1 quest03-gauss)
set(APP_NAME_2 quest03-jordan)
set(APP_NAME_3 quest03-full-pivoting)
set(APP_VERSION 0.1)
set(APP_SRCNAME quest)

set(DEFAULT_ALGO_1 A_NumCppGauss)
set(DEFAULT_ALGO_2 A_NumCppJordan)
set(DEFAULT_ALGO_3 A_NumCppFullPivoting)

configure_file("appconfig.h.in" "appconfig.h")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

set(APP_SOURCES
    ${APP_SRCNAME}.cpp
    dense_linear_solve.cpp
    main.cpp
    )

set(APP_HEADERS
    ${APP_SRCNAME}.hpp
    dense_linear_solve.hpp
    )

add_executable(${APP_NAME_1} ${APP_SOURCES} ${APP_HEADERS})
add_executable(${APP_NAME_2} ${APP_SOURCES} ${APP_HEADERS})
add_executable(${APP_NAME_3} ${APP_SOURCES} ${APP_HEADERS})

set_property(TARGET ${APP_NAME_1} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${APP_NAME_1} PROPERTY CXX_STANDARD_REQUIRED ON)
target_compile_definitions(${APP_NAME_1} PRIVATE APP_NAME=\"${APP_NAME_1}\";QUESTAPP_OPT_DEFAULT_ALGO=${DEFAULT_ALGO_1})
target_link_libraries(${APP_NAME_1} ${app_LIBS})

set_property(TARGET ${APP_NAME_2} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${APP_NAME_2} PROPERTY CXX_STANDARD_REQUIRED ON)
target_compile_definitions(${APP_NAME_2} PRIVATE APP_NAME=\"${APP_NAME_2}\";QUESTAPP_OPT_DEFAULT_ALGO=${DEFAULT_ALGO_2})
target_link_libraries(${APP_NAME_2} ${app_LIBS})

set_property(TARGET ${APP_NAME_3} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${APP_NAME_3} PROPERTY CXX_STANDARD_REQUIRED ON)
target_compile_definitions(${APP_NAME_3} PRIVATE APP_NAME=\"${APP_NAME_3}\";QUESTAPP_OPT_DEFAULT_ALGO=${DEFAULT_ALGO_3})
target_link_libraries(${APP_NAME_3} ${app_LIBS})
