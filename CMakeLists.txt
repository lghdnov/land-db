cmake_minimum_required(VERSION 3.16)
project(land_registry LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(land_db
    main.cpp
    features/registry/LandRegistry.cpp
    features/terminal/TerminalApp.cpp
)

target_include_directories(land_db PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/features)

if(MSVC)
    target_compile_options(land_db PRIVATE /W4)
else()
    target_compile_options(land_db PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_source_files_properties(main.cpp features/terminal/TerminalApp.cpp PROPERTIES
    COMPILE_OPTIONS "-Wno-deprecated-declarations")
