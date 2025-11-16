// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file tests/unit/test_bdd_iterators.cpp
 * @brief Tests comparing TeDDy and CUDD BDD iterator outputs
 *
 * These Catch2 tests parse expression files, convert them to both TeDDy and
 * CUDD BDD representations and verify that the iterator-based traversal
 * produces matching node labels and ordering between implementations.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "cudd_convert.hpp"
#include "cudd_iterator.hpp"
#include "dag_walker.hpp"
#include "expression_parser.hpp"
#include "teddy_convert.hpp"
#include "teddy_iterator.hpp"

void test_expression_to_cudd_and_teddy(const std::string& file_name) {
    // Parse expression from file
    auto expression = read_expression_from_file(file_name);
    REQUIRE(expression != nullptr);

    // Dynamically determine the number of variables needed
    std::unordered_set<std::string> variable_names;
    collect_variables_with_dag_walker(*expression, variable_names);

    // Create sorted variable names for consistent ordering (same as in convert_to_bdd)
    std::vector<std::string> sorted_variable_names(variable_names.begin(), variable_names.end());
    std::ranges::sort(sorted_variable_names);

    // Create a BDD manager with the appropriate number of variables
    teddy::bdd_manager manager(static_cast<int>(variable_names.size()), 1'000);

    auto teddy_bdd = convert_to_bdd(*expression, manager);
    REQUIRE(teddy_bdd.unsafe_get_root() != nullptr);

    auto [cudd_manager, cudd_bdd] = convert_to_cudd_bdd(*expression, variable_names);

    teddy_iterator teddy_it(teddy_bdd.unsafe_get_root(), &sorted_variable_names);

    // Collect unique nodes using dag_walker
    auto teddy_nodes = dag_walker::collect_nodes_topological(teddy_it);

    REQUIRE(!teddy_nodes.empty());

    // Get list of nodes using dag_walker for CUDD BDD in topological order
    cudd_iterator cudd_it(*cudd_manager, cudd_bdd.getNode(), &sorted_variable_names);
    auto cudd_nodes = dag_walker::collect_nodes_topological(cudd_it);
    REQUIRE(!cudd_nodes.empty());

    REQUIRE(teddy_nodes.size() == cudd_nodes.size());

    for (size_t i = 0; i < teddy_nodes.size(); ++i) {
        REQUIRE(teddy_nodes[i].get_label() == cudd_nodes[i].get_label());
    }
}

// Create individual test cases for each file in test_expressions directory, listing them explicitly
#define DECLARE_TEST_CASE(file)                  \
    TEST_CASE(file) {                            \
        test_expression_to_cudd_and_teddy(file); \
    }

DECLARE_TEST_CASE("test_expressions/all_operators.txt")
DECLARE_TEST_CASE("test_expressions/deeply_nested.txt")
DECLARE_TEST_CASE("test_expressions/eight_queens.txt")
DECLARE_TEST_CASE("test_expressions/filter_expression.txt")
DECLARE_TEST_CASE("test_expressions/four_queens.txt")
DECLARE_TEST_CASE("test_expressions/many_variables.txt")
DECLARE_TEST_CASE("test_expressions/multiple_not.txt")
DECLARE_TEST_CASE("test_expressions/my_test_expression.txt")
DECLARE_TEST_CASE("test_expressions/same_variable.txt")
DECLARE_TEST_CASE("test_expressions/simple_expression.txt")
DECLARE_TEST_CASE("test_expressions/single_variable.txt")
DECLARE_TEST_CASE("test_expressions/single_variable_unusual_name.txt")
DECLARE_TEST_CASE("test_expressions/six_queens.txt")
DECLARE_TEST_CASE("test_expressions/test_subdir_expression.txt")
DECLARE_TEST_CASE("test_expressions/underscore_vars.txt")
DECLARE_TEST_CASE("test_expressions/xor_chain.txt")