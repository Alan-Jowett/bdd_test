# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

# Python BDD Integration for CMake
# This file contains all Python BDD related CMake configuration
# to keep the main CMakeLists.txt clean and organized.

# Python BDD Demo Support
# ========================
# Add option to enable Python BDD comparison tests
option(ENABLE_PYTHON_BDD "Enable Python BDD comparison tests" OFF)

if(ENABLE_PYTHON_BDD)
    # Try to find Python
    find_package(Python3 COMPONENTS Interpreter QUIET)

    if(Python3_FOUND)
        # Check if dd library is available
        execute_process(
            COMMAND "${Python3_EXECUTABLE}" -c "import dd"
            RESULT_VARIABLE DD_IMPORT_RESULT
            OUTPUT_QUIET ERROR_QUIET
        )

        if(DD_IMPORT_RESULT EQUAL 0)
            message(STATUS "Python BDD support enabled with 'dd' library")

            # Add Python BDD comparison tests for key expressions
            function(add_python_bdd_test TEST_NAME EXPRESSION_FILE)
                add_test(
                    NAME "python_${TEST_NAME}"
                    COMMAND "${Python3_EXECUTABLE}"
                            "${CMAKE_SOURCE_DIR}/python_bdd/bdd_demo_python.py"
                            "${CMAKE_SOURCE_DIR}/test_expressions/${EXPRESSION_FILE}"
                    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/test_output"
                )
                set_tests_properties("python_${TEST_NAME}" PROPERTIES
                    TIMEOUT 30
                    LABELS "python;bdd;comparison"
                )
            endfunction()

            # Function to add Python vs TeDDy comparison tests
            function(add_python_teddy_comparison_test TEST_NAME EXPRESSION_FILE)
                # Create a test that runs both Python and TeDDy and compares their node table outputs
                set(COMPARISON_SCRIPT "${CMAKE_BINARY_DIR}/compare_python_teddy_${TEST_NAME}.cmake")

                # Generate comparison script
                file(WRITE "${COMPARISON_SCRIPT}" "
# Comparison test for ${TEST_NAME}
set(PYTHON_EXECUTABLE \"${Python3_EXECUTABLE}\")
set(PYTHON_SCRIPT \"${CMAKE_SOURCE_DIR}/python_bdd/bdd_demo_python.py\")

# Determine the correct executable path based on configuration
if(CMAKE_BUILD_TYPE)
    set(BUILD_CONFIG \"\${CMAKE_BUILD_TYPE}\")
else()
    set(BUILD_CONFIG \"Release\")
endif()

if(WIN32)
    set(CPP_EXECUTABLE \"${CMAKE_BINARY_DIR}/bin/\${BUILD_CONFIG}/bdd_demo.exe\")
else()
    set(CPP_EXECUTABLE \"${CMAKE_BINARY_DIR}/bin/bdd_demo\")
endif()

set(EXPRESSION_FILE \"${CMAKE_SOURCE_DIR}/test_expressions/${EXPRESSION_FILE}\")
set(TEST_OUTPUT_DIR \"${CMAKE_BINARY_DIR}/test_output\")
set(TEST_NAME \"${TEST_NAME}\")

# Ensure test output directory exists
file(MAKE_DIRECTORY \"\${TEST_OUTPUT_DIR}\")

# Run Python BDD demo
message(\"Running Python BDD for \${TEST_NAME}...\")
execute_process(
    COMMAND \"\${PYTHON_EXECUTABLE}\" \"\${PYTHON_SCRIPT}\" \"\${EXPRESSION_FILE}\"
    WORKING_DIRECTORY \"\${TEST_OUTPUT_DIR}\"
    OUTPUT_VARIABLE PYTHON_OUTPUT
    ERROR_VARIABLE PYTHON_ERROR
    RESULT_VARIABLE PYTHON_RESULT
)

if(NOT PYTHON_RESULT EQUAL 0)
    message(FATAL_ERROR \"Python BDD failed for \${TEST_NAME}: \${PYTHON_ERROR}\")
endif()

# Run TeDDy C++ implementation
message(\"Running TeDDy BDD for \${TEST_NAME}...\")
execute_process(
    COMMAND \"\${CPP_EXECUTABLE}\" \"\${EXPRESSION_FILE}\" --method=teddy --quiet
    WORKING_DIRECTORY \"\${TEST_OUTPUT_DIR}\"
    OUTPUT_VARIABLE TEDDY_OUTPUT
    ERROR_VARIABLE TEDDY_ERROR
    RESULT_VARIABLE TEDDY_RESULT
)

if(NOT TEDDY_RESULT EQUAL 0)
    message(FATAL_ERROR \"TeDDy BDD failed for \${TEST_NAME}: \${TEDDY_ERROR}\")
endif()

# Extract base filename for comparison
get_filename_component(BASE_NAME \"\${EXPRESSION_FILE}\" NAME_WE)

# Compare node table files (Python output vs TeDDy expected output)
set(PYTHON_NODES_FILE \"${CMAKE_SOURCE_DIR}/test_expressions/\${BASE_NAME}_bdd_nodes.txt\")
set(TEDDY_NODES_FILE \"${CMAKE_SOURCE_DIR}/test_expressions/expected_output/\${BASE_NAME}_bdd_nodes.txt\")

# Check if both files exist
if(NOT EXISTS \"\${PYTHON_NODES_FILE}\")
    message(FATAL_ERROR \"Python node table file not found: \${PYTHON_NODES_FILE}\")
endif()

if(NOT EXISTS \"\${TEDDY_NODES_FILE}\")
    message(FATAL_ERROR \"TeDDy node table file not found: \${TEDDY_NODES_FILE}\")
endif()

# Read and compare the files
file(READ \"\${PYTHON_NODES_FILE}\" PYTHON_NODES_CONTENT)
file(READ \"\${TEDDY_NODES_FILE}\" TEDDY_NODES_CONTENT)

# Normalize line endings for comparison
string(REPLACE \"\\r\\n\" \"\\n\" PYTHON_NODES_CONTENT \"\${PYTHON_NODES_CONTENT}\")
string(REPLACE \"\\r\\n\" \"\\n\" TEDDY_NODES_CONTENT \"\${TEDDY_NODES_CONTENT}\")

# Remove trailing whitespace and empty lines
string(STRIP \"\${PYTHON_NODES_CONTENT}\" PYTHON_NODES_CONTENT)
string(STRIP \"\${TEDDY_NODES_CONTENT}\" TEDDY_NODES_CONTENT)

# Compare the content
if(NOT PYTHON_NODES_CONTENT STREQUAL TEDDY_NODES_CONTENT)
    message(FATAL_ERROR \"Node table mismatch for \${TEST_NAME}!
Python output: \${PYTHON_NODES_FILE}
TeDDy output:  \${TEDDY_NODES_FILE}
The canonical BDD structures should be identical.\")
else()
    message(\"✓ Node table comparison passed for \${TEST_NAME}\")
endif()

# Additional validation: Check node count consistency
string(REGEX MATCHALL \"\\n\" PYTHON_LINES \"\${PYTHON_NODES_CONTENT}\")
string(REGEX MATCHALL \"\\n\" TEDDY_LINES \"\${TEDDY_NODES_CONTENT}\")
list(LENGTH PYTHON_LINES PYTHON_LINE_COUNT)
list(LENGTH TEDDY_LINES TEDDY_LINE_COUNT)

if(NOT PYTHON_LINE_COUNT EQUAL TEDDY_LINE_COUNT)
    message(FATAL_ERROR \"Line count mismatch: Python=\${PYTHON_LINE_COUNT}, TeDDy=\${TEDDY_LINE_COUNT}\")
endif()

message(\"✓ Python vs TeDDy comparison test passed for \${TEST_NAME}\")
")

                # Add the comparison test
                add_test(
                    NAME "compare_python_teddy_${TEST_NAME}"
                    COMMAND "${CMAKE_COMMAND}" -P "${COMPARISON_SCRIPT}"
                )

                # Set test properties
                set_tests_properties("compare_python_teddy_${TEST_NAME}" PROPERTIES
                    TIMEOUT 60
                    LABELS "python;teddy;comparison;validation"
                    DEPENDS "bdd_demo"
                )
            endfunction()

            # Add basic Python BDD tests for key expressions
            add_python_bdd_test(simple_expression "simple_expression.txt")
            add_python_bdd_test(all_operators "all_operators.txt")
            add_python_bdd_test(filter_expression "filter_expression.txt")
            add_python_bdd_test(multiple_not "multiple_not.txt")
            add_python_bdd_test(single_variable "single_variable.txt")

            # Add comprehensive Python vs TeDDy comparison tests
            # These tests verify that Python and TeDDy produce identical canonical BDD structures

            # Get list of all expression files for comprehensive testing
            file(GLOB EXPRESSION_FILES
                 RELATIVE "${CMAKE_SOURCE_DIR}/test_expressions"
                 "${CMAKE_SOURCE_DIR}/test_expressions/*.txt")

            # Filter out files that don't have TeDDy reference outputs
            foreach(EXPR_FILE ${EXPRESSION_FILES})
                get_filename_component(BASE_NAME "${EXPR_FILE}" NAME_WE)
                set(TEDDY_REF_FILE "${CMAKE_SOURCE_DIR}/test_expressions/expected_output/${BASE_NAME}_bdd_nodes.txt")

                # Skip eight_queens test due to timeout issues
                if(BASE_NAME STREQUAL "eight_queens")
                    message(STATUS "Skipping Python vs TeDDy comparison test: ${BASE_NAME} (timeout issues)")
                    continue()
                endif()

                # Skip single_variable_unusual_name test due to variable naming compatibility
                if(BASE_NAME STREQUAL "single_variable_unusual_name")
                    message(STATUS "Skipping Python vs TeDDy comparison test: ${BASE_NAME} (variable naming compatibility)")
                    continue()
                endif()

                if(EXISTS "${TEDDY_REF_FILE}")
                    # Add comparison test for this expression
                    string(REPLACE ".txt" "" TEST_NAME "${EXPR_FILE}")
                    add_python_teddy_comparison_test("${TEST_NAME}" "${EXPR_FILE}")

                    message(STATUS "Added Python vs TeDDy comparison test: ${TEST_NAME}")
                else()
                    message(STATUS "Skipping ${EXPR_FILE} - no TeDDy reference output found")
                endif()
            endforeach()

        else()
            message(WARNING "Python 'dd' library not found. Install with: pip install dd")
            message(STATUS "Python BDD comparison tests disabled")
        endif()
    else()
        message(WARNING "Python3 not found. Python BDD comparison tests disabled")
    endif()
endif()
