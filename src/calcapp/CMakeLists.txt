set(calcapp_SOURCES
    app.cpp
    cli_log.cpp
    cli_opt.cpp
    infile.cpp
    io.cpp
    log.cpp
    options.cpp
    system.cpp
    )

set(calcapp_HEADERS
    app.hpp
    cli.hpp
    exception.hpp
    infile.hpp
    io.hpp
    log.hpp
    options.hpp
    progress.hpp
    system.hpp
    )

add_library(calcapp ${calcapp_SOURCES} ${calcapp_HEADERS})
target_link_libraries(calcapp "${calcapp_LIBS}")
