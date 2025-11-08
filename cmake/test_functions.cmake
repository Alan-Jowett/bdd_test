# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

# CMake test function definitions for BDD Demo project
# This file contains reusable test functions for both BDD generation tests
# and command line argument tests.

# Define test helper function for BDD generation tests
function(add_bdd_test TEST_NAME EXPRESSION_FILE)
    # Create a test that copies the expression file and generates outputs
    add_test(
        NAME ${TEST_NAME}
        COMMAND ${CMAKE_COMMAND}
            -DTEST_NAME=${TEST_NAME}
            -DEXPRESSION_FILE=${EXPRESSION_FILE}
            -DEXECUTABLE=$<TARGET_FILE:bdd_demo>
            -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
            -DBINARY_DIR=${CMAKE_BINARY_DIR}
            -P ${CMAKE_SOURCE_DIR}/cmake/run_bdd_test.cmake
    )

    # Set test properties
    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 30
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endfunction()

# Define command line test helper function
function(add_cmdline_test TEST_NAME)
    set(options SHOULD_FAIL)
    set(oneValueArgs EXPECTED_EXIT_CODE)
    set(multiValueArgs ARGS OUTPUT_CONTAINS ERROR_CONTAINS)
    cmake_parse_arguments(CMDLINE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Set default exit code
    if(NOT DEFINED CMDLINE_EXPECTED_EXIT_CODE)
        if(CMDLINE_SHOULD_FAIL)
            set(CMDLINE_EXPECTED_EXIT_CODE 1)
        else()
            set(CMDLINE_EXPECTED_EXIT_CODE 0)
        endif()
    endif()

    add_test(
        NAME ${TEST_NAME}
        COMMAND ${CMAKE_COMMAND}
            -DTEST_NAME=${TEST_NAME}
            -DEXECUTABLE=$<TARGET_FILE:bdd_demo>
            -DEXPECTED_EXIT_CODE=${CMDLINE_EXPECTED_EXIT_CODE}
            "-DTEST_ARGS=${CMDLINE_ARGS}"
            "-DOUTPUT_CONTAINS=${CMDLINE_OUTPUT_CONTAINS}"
            "-DERROR_CONTAINS=${CMDLINE_ERROR_CONTAINS}"
            -P ${CMAKE_SOURCE_DIR}/cmake/run_cmdline_test.cmake
    )

    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 10
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endfunction()
