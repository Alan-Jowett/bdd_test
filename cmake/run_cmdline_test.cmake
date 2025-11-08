# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

# CMake test script for command line argument testing
# This script is executed by CTest for each command line test case

# Parse command line arguments
if(NOT DEFINED TEST_NAME)
    message(FATAL_ERROR "TEST_NAME not defined")
endif()

if(NOT DEFINED EXECUTABLE)
    message(FATAL_ERROR "EXECUTABLE not defined")
endif()

if(NOT DEFINED EXPECTED_EXIT_CODE)
    message(FATAL_ERROR "EXPECTED_EXIT_CODE not defined")
endif()

message(STATUS "Running command line test: ${TEST_NAME}")

# Convert semicolon-separated arguments to list
if(DEFINED TEST_ARGS)
    string(REPLACE ";" "\\;" TEST_ARGS_ESCAPED "${TEST_ARGS}")
    string(REPLACE "\\;" ";" TEST_ARGS_LIST "${TEST_ARGS_ESCAPED}")
else()
    set(TEST_ARGS_LIST "")
endif()

# Run the executable with the specified arguments
execute_process(
    COMMAND ${EXECUTABLE} ${TEST_ARGS_LIST}
    RESULT_VARIABLE EXEC_RESULT
    OUTPUT_VARIABLE EXEC_OUTPUT
    ERROR_VARIABLE EXEC_ERROR
    TIMEOUT 10
)

message(STATUS "Command executed with exit code: ${EXEC_RESULT}")
message(STATUS "Expected exit code: ${EXPECTED_EXIT_CODE}")

# Check exit code
if(NOT EXEC_RESULT EQUAL EXPECTED_EXIT_CODE)
    message(STATUS "Standard Output: ${EXEC_OUTPUT}")
    message(STATUS "Standard Error: ${EXEC_ERROR}")
    message(FATAL_ERROR "Exit code mismatch: expected ${EXPECTED_EXIT_CODE}, got ${EXEC_RESULT}")
endif()

# Check output contains expected strings
if(DEFINED OUTPUT_CONTAINS AND OUTPUT_CONTAINS)
    string(REPLACE ";" "\\;" OUTPUT_CONTAINS_ESCAPED "${OUTPUT_CONTAINS}")
    string(REPLACE "\\;" ";" OUTPUT_CONTAINS_LIST "${OUTPUT_CONTAINS_ESCAPED}")

    foreach(EXPECTED_STRING ${OUTPUT_CONTAINS_LIST})
        string(FIND "${EXEC_OUTPUT}" "${EXPECTED_STRING}" FOUND_POS)
        if(FOUND_POS EQUAL -1)
            message(STATUS "Standard Output: ${EXEC_OUTPUT}")
            message(STATUS "Standard Error: ${EXEC_ERROR}")
            message(FATAL_ERROR "Output does not contain expected string: '${EXPECTED_STRING}'")
        endif()
    endforeach()
    message(STATUS "✓ Output contains all expected strings")
endif()

# Check error output contains expected strings
if(DEFINED ERROR_CONTAINS AND ERROR_CONTAINS)
    string(REPLACE ";" "\\;" ERROR_CONTAINS_ESCAPED "${ERROR_CONTAINS}")
    string(REPLACE "\\;" ";" ERROR_CONTAINS_LIST "${ERROR_CONTAINS_ESCAPED}")

    foreach(EXPECTED_STRING ${ERROR_CONTAINS_LIST})
        string(FIND "${EXEC_ERROR}" "${EXPECTED_STRING}" FOUND_POS)
        if(FOUND_POS EQUAL -1)
            message(STATUS "Standard Output: ${EXEC_OUTPUT}")
            message(STATUS "Standard Error: ${EXEC_ERROR}")
            message(FATAL_ERROR "Error output does not contain expected string: '${EXPECTED_STRING}'")
        endif()
    endforeach()
    message(STATUS "✓ Error output contains all expected strings")
endif()

message(STATUS "✓ Test ${TEST_NAME} completed successfully")
