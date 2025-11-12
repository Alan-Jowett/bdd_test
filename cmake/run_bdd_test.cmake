# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

# CMake test script for BDD generation testing
# This script is executed by CTest for each test case

# Parse command line arguments
if(NOT DEFINED TEST_NAME)
    message(FATAL_ERROR "TEST_NAME not defined")
endif()

if(NOT DEFINED EXPRESSION_FILE)
    message(FATAL_ERROR "EXPRESSION_FILE not defined")
endif()

if(NOT DEFINED EXECUTABLE)
    message(FATAL_ERROR "EXECUTABLE not defined")
endif()

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR not defined")
endif()

if(NOT DEFINED BINARY_DIR)
    message(FATAL_ERROR "BINARY_DIR not defined")
endif()

# Define paths
set(TEST_EXPRESSIONS_DIR "${SOURCE_DIR}/test_expressions")
set(TEST_BUILD_DIR "${BINARY_DIR}/test_output")

# Handle different path formats for EXPRESSION_FILE
if(IS_ABSOLUTE "${EXPRESSION_FILE}")
    # For absolute paths (like mermaid tests), create test-specific directory
    set(SOURCE_EXPRESSION_FILE "${EXPRESSION_FILE}")
    get_filename_component(EXPRESSION_FILENAME "${EXPRESSION_FILE}" NAME)
    set(TEST_CASE_DIR "${TEST_BUILD_DIR}/${TEST_NAME}")
    set(TEST_EXPRESSION_FILE "${TEST_CASE_DIR}/${EXPRESSION_FILENAME}")
    set(OUTPUT_DIR "${TEST_CASE_DIR}")
else()
    # For relative paths (legacy format)
    set(SOURCE_EXPRESSION_FILE "${TEST_EXPRESSIONS_DIR}/${EXPRESSION_FILE}")
    set(TEST_EXPRESSION_FILE "${TEST_BUILD_DIR}/${EXPRESSION_FILE}")
    # Get base name for output files
    get_filename_component(EXPRESSION_FILE_DIR "${EXPRESSION_FILE}" DIRECTORY)
    if(EXPRESSION_FILE_DIR)
        set(OUTPUT_DIR "${TEST_BUILD_DIR}/${EXPRESSION_FILE_DIR}")
    else()
        set(OUTPUT_DIR "${TEST_BUILD_DIR}")
    endif()
endif()

# Get base name for output files
get_filename_component(BASE_NAME "${EXPRESSION_FILE}" NAME_WE)

# Determine which reference files to use based on test type
if(DEFINED MERMAID_TEST AND MERMAID_TEST)
    # For mermaid tests, use the passed REFERENCE_DIR parameter
    if(NOT DEFINED REFERENCE_DIR)
        message(FATAL_ERROR "REFERENCE_DIR not defined for mermaid test")
    endif()
    # REFERENCE_DIR is already an absolute path for mermaid tests
    set(REFERENCE_FILES_DIR "${REFERENCE_DIR}")
    message(STATUS "Running Mermaid generation test")
elseif(DEFINED FORCE_REORDER AND FORCE_REORDER)
    set(REFERENCE_FILES_DIR "${TEST_EXPRESSIONS_DIR}/reordered")
    message(STATUS "Using reordered reference files for force-reorder test")
elseif(DEFINED TEDDY_METHOD AND TEDDY_METHOD)
    set(REFERENCE_FILES_DIR "${TEST_EXPRESSIONS_DIR}/teddy_expected_output")
    message(STATUS "Using teddy_expected_output reference files for TeDDy method test")
elseif(DEFINED CUDD_METHOD AND CUDD_METHOD)
    set(REFERENCE_FILES_DIR "${TEST_EXPRESSIONS_DIR}/cudd_expected_output")
    message(STATUS "Using cudd_expected_output reference files for CUDD method test")
else()
    set(REFERENCE_FILES_DIR "${TEST_EXPRESSIONS_DIR}/teddy_expected_output")
endif()

# Check if this is an edge case and adjust reference path accordingly
if("${EXPRESSION_FILE}" MATCHES "^edge_cases/")
    if(DEFINED FORCE_REORDER AND FORCE_REORDER)
        set(REFERENCE_FILES_DIR "${TEST_EXPRESSIONS_DIR}/reordered/edge_cases")
    elseif(DEFINED CUDD_METHOD AND CUDD_METHOD)
        set(REFERENCE_FILES_DIR "${TEST_EXPRESSIONS_DIR}/cudd_expected_output/edge_cases")
    else()
        set(REFERENCE_FILES_DIR "${TEST_EXPRESSIONS_DIR}/teddy_expected_output/edge_cases")
    endif()
endif()

set(EXPECTED_NODES_FILE "${REFERENCE_FILES_DIR}/${BASE_NAME}_bdd_nodes.txt")
set(GENERATED_NODES_FILE "${OUTPUT_DIR}/${BASE_NAME}_bdd_nodes.txt")
set(EXPECTED_BDD_DOT_FILE "${REFERENCE_FILES_DIR}/${BASE_NAME}_bdd.dot")
set(GENERATED_BDD_DOT_FILE "${OUTPUT_DIR}/${BASE_NAME}_bdd.dot")
set(EXPECTED_TREE_DOT_FILE "${REFERENCE_FILES_DIR}/${BASE_NAME}_expression_tree.dot")
set(GENERATED_TREE_DOT_FILE "${OUTPUT_DIR}/${BASE_NAME}_expression_tree.dot")

message(STATUS "Running test: ${TEST_NAME}")
message(STATUS "Expression file: ${EXPRESSION_FILE}")
message(STATUS "Test build directory: ${TEST_BUILD_DIR}")

# Create test output directory
file(MAKE_DIRECTORY "${TEST_BUILD_DIR}")

# Check if source expression file exists
if(NOT EXISTS "${SOURCE_EXPRESSION_FILE}")
    message(FATAL_ERROR "Source expression file not found: ${SOURCE_EXPRESSION_FILE}")
endif()

# Skip traditional file checks for mermaid tests
if(NOT DEFINED MERMAID_TEST OR NOT MERMAID_TEST)
    # Check if expected output files exist
    if(NOT EXISTS "${EXPECTED_NODES_FILE}")
        message(FATAL_ERROR "Expected BDD nodes file not found: ${EXPECTED_NODES_FILE}")
    endif()

    if(NOT EXISTS "${EXPECTED_BDD_DOT_FILE}")
        message(FATAL_ERROR "Expected BDD DOT file not found: ${EXPECTED_BDD_DOT_FILE}")
    endif()

    if(NOT EXISTS "${EXPECTED_TREE_DOT_FILE}")
        message(FATAL_ERROR "Expected expression tree DOT file not found: ${EXPECTED_TREE_DOT_FILE}")
    endif()
endif()

# Copy expression file to test directory, preserving directory structure
if(IS_ABSOLUTE "${EXPRESSION_FILE}")
    # For absolute paths, create test-specific directory and copy there
    file(MAKE_DIRECTORY "${TEST_CASE_DIR}")
    file(COPY "${SOURCE_EXPRESSION_FILE}" DESTINATION "${TEST_CASE_DIR}")
else()
    # For relative paths, preserve directory structure
    get_filename_component(EXPRESSION_FILE_DIR "${EXPRESSION_FILE}" DIRECTORY)
    if(EXPRESSION_FILE_DIR)
        file(MAKE_DIRECTORY "${TEST_BUILD_DIR}/${EXPRESSION_FILE_DIR}")
    endif()
    file(COPY "${SOURCE_EXPRESSION_FILE}" DESTINATION "${TEST_BUILD_DIR}/${EXPRESSION_FILE_DIR}")
endif()
message(STATUS "Copied expression file to test directory")

# Run the BDD demo executable
if(DEFINED MERMAID_TEST AND MERMAID_TEST)
    execute_process(
        COMMAND "${EXECUTABLE}" "${TEST_EXPRESSION_FILE}" "--mermaid"
        WORKING_DIRECTORY "${TEST_BUILD_DIR}"
        RESULT_VARIABLE EXEC_RESULT
        OUTPUT_VARIABLE EXEC_OUTPUT
        ERROR_VARIABLE EXEC_ERROR
        TIMEOUT 30
    )
    message(STATUS "Running with --mermaid option")
elseif(DEFINED FORCE_REORDER AND FORCE_REORDER)
    execute_process(
        COMMAND "${EXECUTABLE}" "${TEST_EXPRESSION_FILE}" "--force-reorder"
        WORKING_DIRECTORY "${TEST_BUILD_DIR}"
        RESULT_VARIABLE EXEC_RESULT
        OUTPUT_VARIABLE EXEC_OUTPUT
        ERROR_VARIABLE EXEC_ERROR
        TIMEOUT 60
    )
    message(STATUS "Running with --force-reorder option")
elseif(DEFINED TEDDY_METHOD AND TEDDY_METHOD)
    execute_process(
        COMMAND "${EXECUTABLE}" "${TEST_EXPRESSION_FILE}" "--method=teddy"
        WORKING_DIRECTORY "${TEST_BUILD_DIR}"
        RESULT_VARIABLE EXEC_RESULT
        OUTPUT_VARIABLE EXEC_OUTPUT
        ERROR_VARIABLE EXEC_ERROR
        TIMEOUT 30
    )
    message(STATUS "Running with --method=teddy option")
elseif(DEFINED CUDD_METHOD AND CUDD_METHOD)
    execute_process(
        COMMAND "${EXECUTABLE}" "${TEST_EXPRESSION_FILE}" "--method=cudd"
        WORKING_DIRECTORY "${TEST_BUILD_DIR}"
        RESULT_VARIABLE EXEC_RESULT
        OUTPUT_VARIABLE EXEC_OUTPUT
        ERROR_VARIABLE EXEC_ERROR
        TIMEOUT 30
    )
    message(STATUS "Running with --method=cudd option")
else()
    execute_process(
        COMMAND "${EXECUTABLE}" "${TEST_EXPRESSION_FILE}"
        WORKING_DIRECTORY "${TEST_BUILD_DIR}"
        RESULT_VARIABLE EXEC_RESULT
        OUTPUT_VARIABLE EXEC_OUTPUT
        ERROR_VARIABLE EXEC_ERROR
        TIMEOUT 20
    )
endif()

# Check if executable ran successfully
if(NOT EXEC_RESULT EQUAL 0)
    message(FATAL_ERROR "BDD demo executable failed with code ${EXEC_RESULT}. Error: ${EXEC_ERROR}")
endif()

message(STATUS "BDD demo executed successfully")

# Handle Mermaid test verification (different from regular tests)
if(DEFINED MERMAID_TEST AND MERMAID_TEST)
    # For mermaid tests, compare the generated analysis markdown file against reference
    set(GENERATED_ANALYSIS_FILE "${OUTPUT_DIR}/${BASE_NAME}_analysis.md")
    set(REFERENCE_ANALYSIS_FILE "${REFERENCE_DIR}/${BASE_NAME}_analysis.md")

    if(NOT EXISTS "${GENERATED_ANALYSIS_FILE}")
        message(FATAL_ERROR "Generated analysis markdown file not found: ${GENERATED_ANALYSIS_FILE}")
    endif()

    if(NOT EXISTS "${REFERENCE_ANALYSIS_FILE}")
        message(FATAL_ERROR "Reference analysis markdown file not found: ${REFERENCE_ANALYSIS_FILE}")
    endif()

    # Read both files for comparison
    file(READ "${GENERATED_ANALYSIS_FILE}" GENERATED_CONTENT)
    file(READ "${REFERENCE_ANALYSIS_FILE}" REFERENCE_CONTENT)

    # Compare the files
    if(NOT "${GENERATED_CONTENT}" STREQUAL "${REFERENCE_CONTENT}")
        message(FATAL_ERROR "Generated analysis file differs from reference:\n"
                           "Generated: ${GENERATED_ANALYSIS_FILE}\n"
                           "Reference: ${REFERENCE_ANALYSIS_FILE}\n"
                           "Files are not identical.")
    endif()

    message(STATUS "✓ Mermaid analysis file matches reference")
    message(STATUS "✓ Test ${TEST_NAME} completed successfully - analysis file is identical to reference")
    return()
endif()

# Check if generated files exist (non-mermaid tests)
if(NOT EXISTS "${GENERATED_NODES_FILE}")
    message(FATAL_ERROR "Generated BDD nodes file not found: ${GENERATED_NODES_FILE}")
endif()

if(NOT EXISTS "${GENERATED_BDD_DOT_FILE}")
    message(FATAL_ERROR "Generated BDD DOT file not found: ${GENERATED_BDD_DOT_FILE}")
endif()

if(NOT EXISTS "${GENERATED_TREE_DOT_FILE}")
    message(FATAL_ERROR "Generated expression tree DOT file not found: ${GENERATED_TREE_DOT_FILE}")
endif()

message(STATUS "All generated files found")

# Read and compare BDD nodes files
file(READ "${EXPECTED_NODES_FILE}" EXPECTED_NODES_CONTENT)
file(READ "${GENERATED_NODES_FILE}" GENERATED_NODES_CONTENT)

# Compare node table files directly (they should be identical)
if(NOT "${EXPECTED_NODES_CONTENT}" STREQUAL "${GENERATED_NODES_CONTENT}")
    message(STATUS "Expected BDD nodes:\n${EXPECTED_NODES_CONTENT}")
    message(STATUS "Generated BDD nodes:\n${GENERATED_NODES_CONTENT}")
    message(FATAL_ERROR "Generated BDD nodes file differs from expected")
endif()

message(STATUS "✓ BDD nodes file comparison passed")

# Read and compare DOT files
file(READ "${EXPECTED_BDD_DOT_FILE}" EXPECTED_BDD_DOT_CONTENT)
file(READ "${GENERATED_BDD_DOT_FILE}" GENERATED_BDD_DOT_CONTENT)

# Compare BDD DOT files directly (they should be identical)
if(NOT "${EXPECTED_BDD_DOT_CONTENT}" STREQUAL "${GENERATED_BDD_DOT_CONTENT}")
    message(STATUS "Expected BDD DOT:\n${EXPECTED_BDD_DOT_CONTENT}")
    message(STATUS "Generated BDD DOT:\n${GENERATED_BDD_DOT_CONTENT}")
    message(FATAL_ERROR "Generated BDD DOT file differs from expected")
endif()

message(STATUS "✓ BDD DOT file comparison passed")

# Validate that BDD DOT file uses deterministic node IDs
if("${GENERATED_BDD_DOT_CONTENT}" MATCHES "[0-9]{10,}")
    message(FATAL_ERROR "Generated BDD DOT file contains memory addresses (long numbers) instead of deterministic node IDs")
endif()

if(NOT "${GENERATED_BDD_DOT_CONTENT}" MATCHES "node[0-9]+")
    message(FATAL_ERROR "Generated BDD DOT file does not contain expected deterministic node IDs (node0, node1, etc.)")
endif()

message(STATUS "✓ BDD DOT file uses deterministic node IDs")

# Read and compare expression tree DOT files
file(READ "${EXPECTED_TREE_DOT_FILE}" EXPECTED_TREE_DOT_CONTENT)
file(READ "${GENERATED_TREE_DOT_FILE}" GENERATED_TREE_DOT_CONTENT)

# Compare expression tree DOT files directly (they should be identical)
if(NOT "${EXPECTED_TREE_DOT_CONTENT}" STREQUAL "${GENERATED_TREE_DOT_CONTENT}")
    message(STATUS "Expected tree DOT:\n${EXPECTED_TREE_DOT_CONTENT}")
    message(STATUS "Generated tree DOT:\n${GENERATED_TREE_DOT_CONTENT}")
    message(FATAL_ERROR "Generated expression tree DOT file differs from expected")
endif()

message(STATUS "✓ Expression tree DOT file comparison passed")
message(STATUS "✓ Test ${TEST_NAME} completed successfully - all files match and use deterministic format")
