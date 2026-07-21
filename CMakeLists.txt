cmake_minimum_required(VERSION 3.20)

project(Ketsa VERSION 0.2.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
)

add_executable(ketsa ${SOURCES})

target_include_directories(ketsa PRIVATE
    src
)

if(MSVC)
    target_compile_options(ketsa PRIVATE /W4 /utf-8 /Z7)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT Embedded)
else()
    target_compile_options(ketsa PRIVATE -Wall -Wextra -pedantic -g)
endif()
