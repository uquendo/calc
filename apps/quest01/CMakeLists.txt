set(quest01_SOURCES
    quest.cpp
    main.cpp
    )

set(quest01_HEADERS
    quest.hpp
    )

add_executable(quest01 ${quest01_SOURCES} ${quest01_HEADERS})
set_property(TARGET quest01 PROPERTY CXX_STANDARD 11)
set_property(TARGET quest01 PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(quest01 ${quest_LIBS})
