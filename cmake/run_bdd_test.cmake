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
set(EXPECTED_NODES_FILE "${TEST_EXPRESSIONS_DIR}/default_ordering/${BASE_NAME}_bdd_nodes.txt")
set(GENERATED_NODES_FILE "${TEST_BUILD_DIR}/${BASE_NAME}_bdd_nodes.txt")

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

# Copy expression file to test directory
file(COPY "${SOURCE_EXPRESSION_FILE}" DESTINATION "${TEST_BUILD_DIR}")
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

message(STATUS "Generated BDD nodes file found")

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
message(STATUS "✓ Test ${TEST_NAME} completed successfully")