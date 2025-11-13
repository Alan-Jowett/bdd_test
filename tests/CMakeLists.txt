# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

# Unit testing with Catch2
cmake_minimum_required(VERSION 3.20)

# Fetch Catch2 library
include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.1  # Use a stable version
)

FetchContent_MakeAvailable(Catch2)

# Add the tests module path for find_package(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

# Create a library with the source files we want to test (excluding main.cpp)
add_library(bdd_lib
    ../src/teddy_graph.cpp
    ../src/cudd_graph.cpp
    ../src/expression_graph.cpp
    ../src/expression_parser.cpp
    # Header dependencies for proper rebuild on changes
    ../include/teddy_graph.hpp
    ../include/cudd_graph.hpp
    ../include/expression_graph.hpp
    ../include/expression_types.hpp
    ../include/graph_iterator_concepts.hpp
    ../include/dot_graph_generator.hpp
    ../include/mermaid_graph_generator.hpp
    ../include/node_table_generator.hpp
    ../include/dag_walker.hpp
    ../include/expression_parser.hpp
)

# Add include directories for the library
target_include_directories(bdd_lib PUBLIC
    ../include
    ${CMAKE_BINARY_DIR}/_deps/cudd-src/include
)

# Link with TeDDy and CUDD libraries
target_link_libraries(bdd_lib PUBLIC teddy cudd)

# Set C++20 standard for the library
target_compile_features(bdd_lib PUBLIC cxx_std_20)

# Apply the same compiler flags as the main executable
if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(bdd_lib PRIVATE --coverage -fprofile-arcs -ftest-coverage)
    target_link_options(bdd_lib PRIVATE --coverage)
endif()

# Apply sanitizer flags
if(ENABLE_ASAN)
    target_compile_options(bdd_lib PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(bdd_lib PRIVATE -fsanitize=address)
endif()

if(ENABLE_UBSAN)
    target_compile_options(bdd_lib PRIVATE -fsanitize=undefined)
    target_link_options(bdd_lib PRIVATE -fsanitize=undefined)
endif()

if(ENABLE_MSAN)
    target_compile_options(bdd_lib PRIVATE -fsanitize=memory -fno-omit-frame-pointer)
    target_link_options(bdd_lib PRIVATE -fsanitize=memory)
endif()

if(ENABLE_TSAN)
    target_compile_options(bdd_lib PRIVATE -fsanitize=thread)
    target_link_options(bdd_lib PRIVATE -fsanitize=thread)
endif()

# Create the unit test executable
add_executable(unit_tests
    unit/test_main.cpp
    unit/test_expression_parser.cpp
    unit/test_expression_adapter.cpp
    unit/test_teddy_graph.cpp
    unit/test_cudd_graph.cpp
    unit/test_expression_graph.cpp
    unit/test_bdd_iterators.cpp
    # Uncomment to include example tests for demonstration:
    # unit/test_example.cpp
)

# Link the test executable with Catch2 and our library
target_link_libraries(unit_tests PRIVATE
    Catch2::Catch2WithMain
    bdd_lib
)

# Set C++20 standard for tests
target_compile_features(unit_tests PRIVATE cxx_std_20)

# Set output directory for test executable
set_target_properties(unit_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
)

# Include Catch2's CMake integration
include(CTest)
include(Catch)

# Discover and register Catch2 tests
# Set working directory to project root so test expressions can be found
catch_discover_tests(unit_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Create a custom target to run only unit tests
add_custom_target(run_unit_tests
    COMMAND $<TARGET_FILE:unit_tests>
    DEPENDS unit_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running unit tests with Catch2"
)

# Create a custom target to run unit tests with verbose output
add_custom_target(run_unit_tests_verbose
    COMMAND $<TARGET_FILE:unit_tests> -s
    DEPENDS unit_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running unit tests with Catch2 (verbose)"
)