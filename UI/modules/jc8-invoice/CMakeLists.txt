cmake_minimum_required(VERSION 3.16)

project(JC8InvoiceCore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Core transaction/invoice module. It has no Qt dependency, so it can be
# linked into the team's Qt application or tested on its own.
add_library(jc8invoice_core STATIC
    src/core/Types.cpp
    src/core/InvoiceList.cpp
    src/core/TransactionManager.cpp
)

target_include_directories(jc8invoice_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (MSVC)
    target_compile_options(jc8invoice_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(jc8invoice_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Build the test executable by default only when this is the top-level
# project. When the module is pulled in with add_subdirectory(), the host
# application is not forced to build the tests.
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(JC8_BUILD_TESTS_DEFAULT ON)
else()
    set(JC8_BUILD_TESTS_DEFAULT OFF)
endif()
option(JC8_BUILD_TESTS "Build the JC8Invoice core test executable" ${JC8_BUILD_TESTS_DEFAULT})

if (JC8_BUILD_TESTS)
    enable_testing()

    add_executable(jc8invoice_tests tests/test_core.cpp)
    target_link_libraries(jc8invoice_tests PRIVATE jc8invoice_core)

    if (MSVC)
        target_compile_options(jc8invoice_tests PRIVATE /W4 /permissive-)
    else()
        target_compile_options(jc8invoice_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME jc8invoice_tests COMMAND jc8invoice_tests)
endif()
