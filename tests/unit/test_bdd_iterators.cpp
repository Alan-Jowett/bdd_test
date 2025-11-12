// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

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
    auto teddy_nodes = dag_walker::collect_unique_nodes_topological(teddy_it);

    REQUIRE(!teddy_nodes.empty());

    // Print node labels in order
    std::cout << "TeDDy BDD nodes in visited order:" << std::endl;
    for (const auto& iter : teddy_nodes) {
        std::cout << "Node ID: " << iter.get_label() << std::endl;
    }

    // Get list of nodes using dag_walker for CUDD BDD in topological order
    cudd_iterator cudd_it(*cudd_manager, cudd_bdd.getNode(), &sorted_variable_names);
    auto cudd_nodes = dag_walker::collect_unique_nodes_topological(cudd_it);
    REQUIRE(!cudd_nodes.empty());

    // Print each node label in order
    std::cout << "CUDD BDD nodes in topological order:" << std::endl;
    for (const auto iter : cudd_nodes) {
        std::cout << "Node ID: " << iter.get_label() << std::endl;
    }

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

// TODO: Fix working directory configuration for these unit tests
// DECLARE_TEST_CASE("test_expressions/simple_expression.txt")
// DECLARE_TEST_CASE("test_expressions/all_operators.txt")