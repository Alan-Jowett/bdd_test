# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

cmake_minimum_required(VERSION 3.20)
project(BDD_Demo)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add option for code coverage
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)

# Configure coverage flags
if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(STATUS "Enabling code coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
endif()

# Include FetchContent module to download dependencies
include(FetchContent)

# Fetch TeDDy library from GitHub
FetchContent_Declare(
    DecisionDiagrams
    GIT_REPOSITORY https://github.com/MichalMrena/DecisionDiagrams.git
    GIT_TAG v4.1.0
)

FetchContent_MakeAvailable(DecisionDiagrams)

# Create executable
add_executable(bdd_demo main.cpp)

# Link with TeDDy library
target_link_libraries(bdd_demo PRIVATE teddy::teddy)

# Set output directory
set_target_properties(bdd_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Enable testing
enable_testing()

# Include test function definitions
include(cmake/test_functions.cmake)

# Add individual tests for BDD generation
add_bdd_test(test_filter_expression "filter_expression.txt")
add_bdd_test(test_simple_expression "simple_expression.txt")
add_bdd_test(test_my_test_expression "my_test_expression.txt")
add_bdd_test(test_subdir_expression "test_subdir_expression.txt")

# Command line help tests
add_cmdline_test(test_help_long
    ARGS "--help"
    OUTPUT_CONTAINS "TeDDy BDD Demo - Binary Decision Diagram Converter;Usage: bdd_demo"
    EXPECTED_EXIT_CODE 0
)

add_cmdline_test(test_help_short  
    ARGS "-h"
    OUTPUT_CONTAINS "TeDDy BDD Demo - Binary Decision Diagram Converter;Usage: bdd_demo"
    EXPECTED_EXIT_CODE 0
)

# Command line error tests
add_cmdline_test(test_unknown_option
    ARGS "--unknown-option"
    SHOULD_FAIL
    ERROR_CONTAINS "Unknown option: --unknown-option"
    OUTPUT_CONTAINS "Usage: bdd_demo"
)

add_cmdline_test(test_multiple_files
    ARGS "file1.txt;file2.txt"
    SHOULD_FAIL
    ERROR_CONTAINS "Multiple input files specified. Only one file is allowed."
    OUTPUT_CONTAINS "Usage: bdd_demo"
)

# Valid option tests
add_cmdline_test(test_enable_reordering
    ARGS "${CMAKE_SOURCE_DIR}/test_expressions/simple_expression.txt;--enable-reordering"
    OUTPUT_CONTAINS "Automatic variable reordering enabled"
    EXPECTED_EXIT_CODE 0
)

add_cmdline_test(test_disable_reordering
    ARGS "${CMAKE_SOURCE_DIR}/test_expressions/simple_expression.txt;--disable-reordering"
    OUTPUT_CONTAINS "Automatic variable reordering disabled"
    EXPECTED_EXIT_CODE 0
)

add_cmdline_test(test_force_reorder
    ARGS "${CMAKE_SOURCE_DIR}/test_expressions/simple_expression.txt;--force-reorder"
    OUTPUT_CONTAINS "Forcing variable reordering after BDD construction"
    EXPECTED_EXIT_CODE 0
)

# Combined option tests
add_cmdline_test(test_combined_options
    ARGS "${CMAKE_SOURCE_DIR}/test_expressions/simple_expression.txt;--enable-reordering;--force-reorder"
    OUTPUT_CONTAINS "Automatic variable reordering enabled;Forcing variable reordering after BDD construction"
    EXPECTED_EXIT_CODE 0
)

# Test option that overrides previous option
add_cmdline_test(test_override_options
    ARGS "${CMAKE_SOURCE_DIR}/test_expressions/simple_expression.txt;--enable-reordering;--disable-reordering"
    OUTPUT_CONTAINS "Automatic variable reordering disabled"
    EXPECTED_EXIT_CODE 0
)