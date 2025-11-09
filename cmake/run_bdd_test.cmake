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
set(SOURCE_EXPRESSION_FILE "${TEST_EXPRESSIONS_DIR}/${EXPRESSION_FILE}")
set(TEST_EXPRESSION_FILE "${TEST_BUILD_DIR}/${EXPRESSION_FILE}")

# Get base name for output files
get_filename_component(BASE_NAME "${EXPRESSION_FILE}" NAME_WE)
get_filename_component(EXPRESSION_FILE_DIR "${EXPRESSION_FILE}" DIRECTORY)
if(EXPRESSION_FILE_DIR)
    set(OUTPUT_DIR "${TEST_BUILD_DIR}/${EXPRESSION_FILE_DIR}")
else()
    set(OUTPUT_DIR "${TEST_BUILD_DIR}")
endif()
set(EXPECTED_NODES_FILE "${TEST_EXPRESSIONS_DIR}/default_ordering/${BASE_NAME}_bdd_nodes.txt")
set(GENERATED_NODES_FILE "${OUTPUT_DIR}/${BASE_NAME}_bdd_nodes.txt")
set(EXPECTED_BDD_DOT_FILE "${TEST_EXPRESSIONS_DIR}/default_ordering/${BASE_NAME}_bdd.dot")
set(GENERATED_BDD_DOT_FILE "${OUTPUT_DIR}/${BASE_NAME}_bdd.dot")
set(EXPECTED_TREE_DOT_FILE "${TEST_EXPRESSIONS_DIR}/default_ordering/${BASE_NAME}_expression_tree.dot")
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

# Copy expression file to test directory, preserving directory structure
get_filename_component(EXPRESSION_FILE_DIR "${EXPRESSION_FILE}" DIRECTORY)
if(EXPRESSION_FILE_DIR)
    file(MAKE_DIRECTORY "${TEST_BUILD_DIR}/${EXPRESSION_FILE_DIR}")
endif()
file(COPY "${SOURCE_EXPRESSION_FILE}" DESTINATION "${TEST_BUILD_DIR}/${EXPRESSION_FILE_DIR}")
message(STATUS "Copied expression file to test directory")

# Run the BDD demo executable
execute_process(
    COMMAND "${EXECUTABLE}" "${TEST_EXPRESSION_FILE}"
    WORKING_DIRECTORY "${TEST_BUILD_DIR}"
    RESULT_VARIABLE EXEC_RESULT
    OUTPUT_VARIABLE EXEC_OUTPUT
    ERROR_VARIABLE EXEC_ERROR
    TIMEOUT 20
)

# Check if executable ran successfully
if(NOT EXEC_RESULT EQUAL 0)
    message(FATAL_ERROR "BDD demo executable failed with code ${EXEC_RESULT}. Error: ${EXEC_ERROR}")
endif()

message(STATUS "BDD demo executed successfully")

# Check if generated files exist
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
