# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

cmake_minimum_required(VERSION 3.20)
project(BDD_Demo)

# Install Git hooks if enabled and .git/hooks directory exists
option(BDD_INSTALL_GIT_HOOKS "Install git hooks for pre-commit formatting checks" ON)
if (BDD_INSTALL_GIT_HOOKS AND EXISTS "${PROJECT_SOURCE_DIR}/.git/hooks")
    # Install Git pre-commit hook
    file(COPY scripts/pre-commit
        DESTINATION "${PROJECT_SOURCE_DIR}/.git/hooks"
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    message(STATUS "Installing git pre-commit hook to .git/hooks/")
endif()

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

# Create executable with source files and header dependencies
add_executable(bdd_demo
    src/main.cpp
    src/bdd_graph.cpp
    src/expression_graph.cpp
    src/expression_parser.cpp
    # Header dependencies for proper rebuild on changes
    include/bdd_graph.hpp
    include/expression_graph.hpp
    include/expression_types.hpp
    include/dot_graph_generator.hpp
    include/dag_walker.hpp
    include/expression_parser.hpp
)

# Add include directories
target_include_directories(bdd_demo PRIVATE include)

# Link with TeDDy library
target_link_libraries(bdd_demo PRIVATE teddy::teddy)

# Set output directory for executable
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

# Add new tests to improve coverage
add_bdd_test(test_single_variable "single_variable.txt")
add_bdd_test(test_deeply_nested "deeply_nested.txt")
add_bdd_test(test_multiple_not "multiple_not.txt")
add_bdd_test(test_all_operators "all_operators.txt")
add_bdd_test(test_many_variables "many_variables.txt")
add_bdd_test(test_same_variable "same_variable.txt")
add_bdd_test(test_xor_chain "xor_chain.txt")
add_bdd_test(test_underscore_vars "underscore_vars.txt")
add_bdd_test(test_single_variable_unusual_name "single_variable_unusual_name.txt")

# Stress test with complex constraint satisfaction
add_bdd_test(test_four_queens "four_queens.txt")
add_bdd_test(test_six_queens "six_queens.txt")
add_bdd_test(test_eight_queens "eight_queens.txt")

# Regression tests for PR #22 - robust expression parser fixes
add_bdd_test(test_boundary_detection_regression "edge_cases/boundary_detection_regression.txt")
add_bdd_test(test_operator_keywords_in_vars "edge_cases/operator_keywords_in_vars.txt")
add_bdd_test(test_unicode_chars_regression "edge_cases/unicode_chars_regression.txt")
add_bdd_test(test_complex_boundary_regression "edge_cases/complex_boundary_regression.txt")

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

# Error handling tests to improve coverage
add_cmdline_test(test_nonexistent_file
    ARGS "nonexistent_file.txt"
    SHOULD_FAIL
    ERROR_CONTAINS "Error reading expression file: Could not open file: nonexistent_file.txt"
    EXPECTED_EXIT_CODE 1
)

add_cmdline_test(test_empty_expression_file
    ARGS "${CMAKE_SOURCE_DIR}/test_expressions/edge_cases/comments_only.txt"
    SHOULD_FAIL
    ERROR_CONTAINS "Error reading expression file: No expression found in file"
    EXPECTED_EXIT_CODE 1
)

# Regression test for readable error messages (PR #22)
add_cmdline_test(test_error_message_regression
    ARGS "${CMAKE_SOURCE_DIR}/test_expressions/edge_cases/missing_paren_regression.txt"
    SHOULD_FAIL
    ERROR_CONTAINS "Expected ')' but got end of input"
    EXPECTED_EXIT_CODE 1
)
