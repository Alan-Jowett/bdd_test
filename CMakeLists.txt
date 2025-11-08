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

# Define test helper function
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

# Add individual tests
add_bdd_test(test_filter_expression "filter_expression.txt")
add_bdd_test(test_simple_expression "simple_expression.txt")
add_bdd_test(test_my_test_expression "my_test_expression.txt")
add_bdd_test(test_subdir_expression "test_subdir_expression.txt")