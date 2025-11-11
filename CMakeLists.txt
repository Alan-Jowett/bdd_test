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
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
endif()

# Add sanitizer options
# These sanitizers help detect various runtime errors during development and testing:
# - ASAN: Detects memory errors (buffer overflows, use-after-free, etc.)
# - UBSAN: Detects undefined behavior (signed integer overflow, etc.)
# - MSAN: Detects uninitialized memory reads (Clang only)
# - TSAN: Detects data races in multi-threaded code
#
# Usage examples:
#   cmake -DENABLE_ASAN=ON ..          # AddressSanitizer
#   cmake -DENABLE_UBSAN=ON ..         # UndefinedBehaviorSanitizer
#   cmake -DENABLE_ASAN=ON -DENABLE_UBSAN=ON ..  # Both (compatible)
#
# Note: Some sanitizers are mutually exclusive (ASAN+MSAN, ASAN+TSAN, MSAN+TSAN)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ENABLE_MSAN "Enable MemorySanitizer (Clang only)" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)

# Configure AddressSanitizer (ASan)
if(ENABLE_ASAN)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "Enabling AddressSanitizer (ASan) for GCC/Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        message(STATUS "Enabling AddressSanitizer (ASan) for MSVC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fsanitize=address")
        # MSVC ASan requires specific runtime library
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
    else()
        message(WARNING "AddressSanitizer not supported for compiler: ${CMAKE_CXX_COMPILER_ID}")
    endif()
endif()

# Configure UndefinedBehaviorSanitizer (UBSan)
if(ENABLE_UBSAN)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "Enabling UndefinedBehaviorSanitizer (UBSan) for GCC/Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -fno-omit-frame-pointer -g")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined -fno-omit-frame-pointer -g")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=undefined")
    else()
        message(WARNING "UndefinedBehaviorSanitizer not supported for compiler: ${CMAKE_CXX_COMPILER_ID}")
    endif()
endif()

# Configure MemorySanitizer (MSan) - Clang only
if(ENABLE_MSAN)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(STATUS "Enabling MemorySanitizer (MSan) for Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory -fno-omit-frame-pointer -g")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=memory")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=memory")
    else()
        message(WARNING "MemorySanitizer only supported with Clang, not ${CMAKE_CXX_COMPILER_ID}")
    endif()
endif()

# Configure ThreadSanitizer (TSan)
if(ENABLE_TSAN)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "Enabling ThreadSanitizer (TSan) for GCC/Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -fno-omit-frame-pointer -g")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=thread")
    else()
        message(WARNING "ThreadSanitizer not supported for compiler: ${CMAKE_CXX_COMPILER_ID}")
    endif()
endif()

# Sanitizer compatibility check
if((ENABLE_ASAN OR ENABLE_MSAN OR ENABLE_TSAN) AND ENABLE_COVERAGE)
    message(WARNING "Sanitizers and coverage may not work together reliably")
endif()

if(ENABLE_ASAN AND ENABLE_MSAN)
    message(FATAL_ERROR "AddressSanitizer and MemorySanitizer cannot be used together")
endif()

if(ENABLE_ASAN AND ENABLE_TSAN)
    message(FATAL_ERROR "AddressSanitizer and ThreadSanitizer cannot be used together")
endif()

if(ENABLE_MSAN AND ENABLE_TSAN)
    message(FATAL_ERROR "MemorySanitizer and ThreadSanitizer cannot be used together")
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

# Apply patches to TeDDy library
set(TEDDY_SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/decisiondiagrams-src")
set(PATCHES_DIR "${CMAKE_SOURCE_DIR}/patches")

# Find and apply all patch files in order
file(GLOB PATCH_FILES "${PATCHES_DIR}/*.patch")
list(SORT PATCH_FILES)

if(PATCH_FILES)
    list(LENGTH PATCH_FILES PATCH_COUNT)
    message(STATUS "Found ${PATCH_COUNT} TeDDy patches to apply")
    find_program(GIT_EXECUTABLE git)
    if(GIT_EXECUTABLE)
        foreach(PATCH_FILE ${PATCH_FILES})
            get_filename_component(PATCH_NAME "${PATCH_FILE}" NAME)

            # Check if patch is already applied using --check flag
            execute_process(
                COMMAND ${GIT_EXECUTABLE} apply --check "${PATCH_FILE}"
                WORKING_DIRECTORY "${TEDDY_SOURCE_DIR}"
                RESULT_VARIABLE PATCH_CHECK_RESULT
                OUTPUT_QUIET
                ERROR_QUIET
            )

            if(PATCH_CHECK_RESULT EQUAL 0)
                # Patch can be applied, so apply it
                message(STATUS "Applying patch: ${PATCH_NAME}")
                execute_process(
                    COMMAND ${GIT_EXECUTABLE} apply "${PATCH_FILE}"
                    WORKING_DIRECTORY "${TEDDY_SOURCE_DIR}"
                    RESULT_VARIABLE PATCH_RESULT
                    OUTPUT_VARIABLE PATCH_OUTPUT
                    ERROR_VARIABLE PATCH_ERROR
                    OUTPUT_QUIET
                    ERROR_QUIET
                )
                if(PATCH_RESULT EQUAL 0)
                    message(STATUS "  ✓ ${PATCH_NAME} applied successfully")
                else()
                    # Try applying with --ignore-whitespace flag for robustness
                    execute_process(
                        COMMAND ${GIT_EXECUTABLE} apply --ignore-whitespace "${PATCH_FILE}"
                        WORKING_DIRECTORY "${TEDDY_SOURCE_DIR}"
                        RESULT_VARIABLE PATCH_RESULT_RETRY
                        OUTPUT_VARIABLE PATCH_OUTPUT_RETRY
                        ERROR_VARIABLE PATCH_ERROR_RETRY
                        OUTPUT_QUIET
                        ERROR_QUIET
                    )
                    if(PATCH_RESULT_RETRY EQUAL 0)
                        message(STATUS "  ✓ ${PATCH_NAME} applied successfully (with whitespace ignore)")
                    else()
                        message(WARNING "  ✗ Failed to apply patch ${PATCH_NAME}")
                        message(WARNING "     First attempt error: ${PATCH_ERROR}")
                        message(WARNING "     Retry attempt error: ${PATCH_ERROR_RETRY}")
                    endif()
                endif()
            else()
                # Patch cannot be applied, likely already applied or conflicts
                message(STATUS "  ○ ${PATCH_NAME} already applied or not applicable")
            endif()
        endforeach()
    else()
        message(WARNING "Git not found, cannot apply TeDDy patches")
    endif()
else()
    message(STATUS "No TeDDy patches found in ${PATCHES_DIR}")
endif()

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

# Automatically discover and register all BDD test cases
# This scans test_expressions/ for .txt files and creates both default ordering
# and force-reorder tests based on available reference files
register_bdd_tests()

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

# Note: Force-reorder tests may exhibit non-deterministic output across different environments
# due to tie-breaking behavior in TeDDy's heapsort implementation when variables have equal node counts.
# This can cause CI failures where the same logical optimization produces different node orderings.

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

# Mermaid analysis file generation regression tests
register_mermaid_tests()
