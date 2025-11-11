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

# Define test helper function for force-reorder BDD generation tests
function(add_bdd_force_reorder_test TEST_NAME EXPRESSION_FILE)
    # Create a test that uses --force-reorder and compares against reordered reference files
    add_test(
        NAME ${TEST_NAME}
        COMMAND ${CMAKE_COMMAND}
            -DTEST_NAME=${TEST_NAME}
            -DEXPRESSION_FILE=${EXPRESSION_FILE}
            -DEXECUTABLE=$<TARGET_FILE:bdd_demo>
            -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
            -DBINARY_DIR=${CMAKE_BINARY_DIR}
            -DFORCE_REORDER=TRUE
            -P ${CMAKE_SOURCE_DIR}/cmake/run_bdd_test.cmake
    )

    # Set test properties with longer timeout for reordering operations
    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 60
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endfunction()

# Define test helper function for TeDDy method BDD generation tests
function(add_bdd_teddy_test TEST_NAME EXPRESSION_FILE)
    # Create a test that uses --method=teddy and compares against default reference files
    add_test(
        NAME ${TEST_NAME}
        COMMAND ${CMAKE_COMMAND}
            -DTEST_NAME=${TEST_NAME}
            -DEXPRESSION_FILE=${EXPRESSION_FILE}
            -DEXECUTABLE=$<TARGET_FILE:bdd_demo>
            -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
            -DBINARY_DIR=${CMAKE_BINARY_DIR}
            -DTEDDY_METHOD=TRUE
            -P ${CMAKE_SOURCE_DIR}/cmake/run_bdd_test.cmake
    )

    # Set test properties
    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 30
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endfunction()

# Define test helper function for Mermaid generation tests
function(add_mermaid_test TEST_NAME EXPRESSION_FILE)
    # Create a test that uses --mermaid and verifies the analysis markdown file
    add_test(
        NAME ${TEST_NAME}
        COMMAND ${CMAKE_COMMAND}
            -DTEST_NAME=${TEST_NAME}
            -DEXPRESSION_FILE=${EXPRESSION_FILE}
            -DEXECUTABLE=$<TARGET_FILE:bdd_demo>
            -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
            -DBINARY_DIR=${CMAKE_BINARY_DIR}
            -DREFERENCE_DIR=${CMAKE_SOURCE_DIR}/test_expressions/reference_outputs
            -DMERMAID_TEST=TRUE
            -P ${CMAKE_SOURCE_DIR}/cmake/run_bdd_test.cmake
    )

    # Set test properties
    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 30
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endfunction()

# Function to automatically discover and register BDD test cases
# This function scans for .txt files in test_expressions/ and creates both
# default ordering tests and force-reorder tests where applicable
function(register_bdd_tests)
    # Get all .txt files in test_expressions directory (excluding subdirectories for now)
    file(GLOB TEST_EXPRESSION_FILES "${CMAKE_SOURCE_DIR}/test_expressions/*.txt")

    foreach(EXPRESSION_FILE ${TEST_EXPRESSION_FILES})
        # Get the relative filename
        file(RELATIVE_PATH REL_PATH "${CMAKE_SOURCE_DIR}/test_expressions" "${EXPRESSION_FILE}")
        get_filename_component(BASE_NAME "${REL_PATH}" NAME_WE)

        # Skip generated files (those with _bdd_nodes or _expression_tree in name)
        if(NOT "${BASE_NAME}" MATCHES ".*_(bdd_nodes|expression_tree|bdd)$")
            # Check if reference files exist in default_ordering directory
            set(DEFAULT_NODES_FILE "${CMAKE_SOURCE_DIR}/test_expressions/default_ordering/${BASE_NAME}_bdd_nodes.txt")
            if(EXISTS "${DEFAULT_NODES_FILE}")
                # Create test name, handling cases where BASE_NAME already starts with "test_"
                if("${BASE_NAME}" MATCHES "^test_.*")
                    set(TEST_NAME "${BASE_NAME}")
                else()
                    set(TEST_NAME "test_${BASE_NAME}")
                endif()

                # Add default ordering test
                add_bdd_test("${TEST_NAME}" "${REL_PATH}")
                message(STATUS "Added default ordering test: ${TEST_NAME}")

                # Add TeDDy method test (should produce identical results)
                add_bdd_teddy_test("${TEST_NAME}_teddy" "${REL_PATH}")
                message(STATUS "Added TeDDy method test: ${TEST_NAME}_teddy")

                # Check if reordered reference files exist
                set(REORDERED_NODES_FILE "${CMAKE_SOURCE_DIR}/test_expressions/reordered/${BASE_NAME}_bdd_nodes.txt")

                # Skip force-reorder tests for n-queens problems due to non-deterministic output
                # caused by tie-breaking behavior in TeDDy's heapsort when variables have equal node counts
                if(EXISTS "${REORDERED_NODES_FILE}" AND NOT "${BASE_NAME}" MATCHES ".*queens.*")
                    # Add force-reorder test
                    add_bdd_force_reorder_test("${TEST_NAME}_reorder" "${REL_PATH}")
                    message(STATUS "Added force-reorder test: ${TEST_NAME}_reorder")
                elseif("${BASE_NAME}" MATCHES ".*queens.*")
                    message(STATUS "Skipped force-reorder test for n-queens problem: ${TEST_NAME}_reorder (non-deterministic)")
                endif()
            endif()
        endif()
    endforeach()

    # Handle edge cases subdirectory separately
    file(GLOB EDGE_CASE_FILES "${CMAKE_SOURCE_DIR}/test_expressions/edge_cases/*.txt")

    foreach(EXPRESSION_FILE ${EDGE_CASE_FILES})
        # Get the relative filename from test_expressions root
        file(RELATIVE_PATH REL_PATH "${CMAKE_SOURCE_DIR}/test_expressions" "${EXPRESSION_FILE}")
        get_filename_component(BASE_NAME "${EXPRESSION_FILE}" NAME_WE)

        # Skip generated files
        if(NOT "${BASE_NAME}" MATCHES ".*_(bdd_nodes|expression_tree|bdd)$")
            # Check if reference files exist in default_ordering directory (not in edge_cases subdirectory)
            set(DEFAULT_NODES_FILE "${CMAKE_SOURCE_DIR}/test_expressions/default_ordering/${BASE_NAME}_bdd_nodes.txt")
            if(EXISTS "${DEFAULT_NODES_FILE}")
                # Create test name, handling cases where BASE_NAME already starts with "test_"
                if("${BASE_NAME}" MATCHES "^test_.*")
                    set(TEST_NAME "${BASE_NAME}")
                else()
                    set(TEST_NAME "test_${BASE_NAME}")
                endif()

                # Add default ordering test
                add_bdd_test("${TEST_NAME}" "${REL_PATH}")
                message(STATUS "Added edge case test: ${TEST_NAME}")

                # Add TeDDy method test for edge cases (should produce identical results)
                add_bdd_teddy_test("${TEST_NAME}_teddy" "${REL_PATH}")
                message(STATUS "Added TeDDy method edge case test: ${TEST_NAME}_teddy")
            endif()
        endif()
    endforeach()
endfunction()
