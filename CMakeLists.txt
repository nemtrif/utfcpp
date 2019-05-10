cmake_minimum_required(VERSION 3.5)
project (utf8cpp
    DESCRIPTION "UTF-8 with C++ in a Portable Way"
    VERSION 3.1
    LANGUAGES CXX)

option(UTF8_TESTS "Enable tests for UTF8-CPP" On)
option(UTF8_INSTALL "Enable installation for UTF8-CPP" On)
option(UTF8_SAMPLES "Enable building samples for UTF8-CPP" On)

###############
# Conan support
###############

if (UTF8_TESTS)
    if (EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
        include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
        message(STATUS "conan_basic_setup()")
        conan_basic_setup(TARGETS)
        set(USE_CONAN True)
    endif()
endif()

###############################
# Check compiler's capabilities
###############################

include (CheckCXXCompilerFlag)

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    set (CMAKE_COMPILER_IS_CLANG true)
elseif(CMAKE_C_COMPILER_ID MATCHES "MSVC")
    set (CMAKE_COMPILER_IS_MSVC true)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
    find_program (LINKER_BIN gold)
    if(LINKER_BIN)
        set(LINKER_BIN "gold")
    else()
        find_program (LINKER_BIN ld.gold)
        if(LINKER_BIN)
            set(LINKER_BIN "ld.gold")
        else()
            set(LINKER_BIN "ld")
        endif()
    endif()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra")
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")

    CHECK_CXX_COMPILER_FLAG("-fuse-ld=${LINKER_BIN}" USE_LINKER_FLAG)
    if(USE_LINKER_FLAG)
        set (CMAKE_SHARED_LINKER_FLAGS "-fuse-ld=${LINKER_BIN} ${CMAKE_SHARED_LINKER_FLAGS}")
    endif()
elseif(CMAKE_COMPILER_IS_MSVC)
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_SECURE_SCL=0")
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CRT_SECURE_NO_DEPRECATE")
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CRT_NONSTDC_NO_DEPRECATE")
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_SCL_SECURE_NO_DEPRECATE")
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Zi")
endif()

message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
set (CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include")

add_library(utf8cpp INTERFACE)
target_include_directories(utf8cpp INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/source>"
    $<INSTALL_INTERFACE:include/utf8cpp>
)
add_library(utf8::cpp ALIAS utf8cpp)

if(UTF8_INSTALL)
    if(WIN32 AND NOT CYGWIN)
        set(DEF_INSTALL_CMAKE_DIR CMake)
    else()
        include(GNUInstallDirs) # define CMAKE_INSTALL_*
        set(DEF_INSTALL_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/utf8cpp)
    endif()

    install(DIRECTORY source/ DESTINATION include)
    install(TARGETS utf8cpp EXPORT utf8cppConfig)
    install(EXPORT utf8cppConfig DESTINATION ${DEF_INSTALL_CMAKE_DIR})
endif()

if(UTF8_SAMPLES)
    add_subdirectory(samples)
endif()

if(UTF8_TESTS)
    enable_testing()
    if (NOT USE_CONAN)
        add_subdirectory(extern/gtest)
    endif()
    add_subdirectory(tests)
endif()
