cmake_minimum_required(VERSION 3.10)

project(ConfigParserProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(ConfigParserProject
    main.cpp
)

target_include_directories(ConfigParserProject PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})