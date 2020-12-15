cmake_minimum_required(VERSION 3.16)

project(HGL-ArgParse)

option(TEST "build tests" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(TEST)
    set(CMAKE_BUILD_TYPE Debug)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()

aux_source_directory(src SRCS)
add_library(hgargparse STATIC ${SRCS})
target_include_directories(hgargparse PUBLIC include)
unset(SRCS)

if(TEST)
    add_subdirectory(test)
endif()
