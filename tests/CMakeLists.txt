find_package (GTest REQUIRED)

#include(CMakeParseArguments)
function(add_unittest)
    set(options _empty)
    set(oneValueArgs TESTNAME)
    set(multiValueArgs SOURCES LIBRARIES ARGUMENTS)
    cmake_parse_arguments(PARSE_ARGV 0 "impl" "${options}" "${oneValueArgs}" "${multiValueArgs}")

    add_executable(${impl_TESTNAME} ${impl_SOURCES})
    target_link_libraries(${impl_TESTNAME} PRIVATE
        utf8::cpp
        ${impl_LIBRARIES}
        )
    if (NOT ${impl_ARGUMENTS})
        add_test(NAME "${impl_TESTNAME}_test"
            COMMAND "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${impl_TESTNAME}" ${impl_ARGUMENTS})
    else()
        add_test(NAME "${impl_TESTNAME}_test"
            COMMAND "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${impl_TESTNAME}")
    endif()
endfunction()

add_unittest(TESTNAME negative
    SOURCES "negative.cpp"
    ARGUMENTS "${PROJECT_SOURCE_DIR}/tests/test_data/utf8_invalid.txt")

add_unittest(TESTNAME cpp11
    SOURCES "test_cpp11.cpp"
    LIBRARIES GTest::Main)

add_unittest(TESTNAME apitests
    SOURCES
    "test_checked_api.cpp"
    "test_unchecked_api.cpp"
    "test_checked_iterator.cpp"
    "test_unchecked_iterator.cpp"
    LIBRARIES GTest::Main)

add_unittest(TESTNAME noexceptionstests
    SOURCES
    "test_unchecked_api.cpp"
    "test_unchecked_iterator.cpp"
    LIBRARIES GTest::Main)
target_compile_options(noexceptionstests PUBLIC "-fno-exceptions")

