cmake_minimum_required (VERSION 3.5)
project(utfcppbench LANGUAGES CXX)

include_directories("${PROJECT_SOURCE_DIR}/../source")

if (MSVC)
    # warning level 4
    add_compile_options(/W4)
else()
    # additional warnings
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

add_executable(benchmark benchmark.cpp)

set_target_properties(benchmark
                      PROPERTIES
                      CXX_STANDARD 11
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)