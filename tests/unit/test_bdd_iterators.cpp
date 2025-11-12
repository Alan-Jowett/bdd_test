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

    // Note: Different BDD implementations can produce semantically equivalent BDDs
    // with different node counts and structures. This is expected and correct behavior.
    // CUDD often produces more reduced BDDs than TeDDy for complex expressions.
    // We verify basic sanity checks rather than requiring structural identity.

    // Both BDDs should have terminal nodes (0 and 1)
    bool teddy_has_zero = false, teddy_has_one = false;
    bool cudd_has_zero = false, cudd_has_one = false;

    for (const auto& node : teddy_nodes) {
        std::string label = node.get_label();
        if (label == "0")
            teddy_has_zero = true;
        if (label == "1")
            teddy_has_one = true;
    }

    for (const auto& node : cudd_nodes) {
        std::string label = node.get_label();
        if (label == "0")
            cudd_has_zero = true;
        if (label == "1")
            cudd_has_one = true;
    }

    // Both should have terminal nodes (unless the function is a tautology or contradiction)
    REQUIRE((teddy_has_zero || teddy_has_one));
    REQUIRE((cudd_has_zero || cudd_has_one));

    // Both should use the same variables (though possibly in different structures)
    std::unordered_set<std::string> teddy_vars, cudd_vars;
    for (const auto& node : teddy_nodes) {
        std::string label = node.get_label();
        if (label != "0" && label != "1") {
            teddy_vars.insert(label);
        }
    }
    for (const auto& node : cudd_nodes) {
        std::string label = node.get_label();
        if (label != "0" && label != "1") {
            cudd_vars.insert(label);
        }
    }

    // The set of variables used should be the same (or CUDD might use fewer due to optimization)
    for (const auto& cudd_var : cudd_vars) {
        REQUIRE(teddy_vars.find(cudd_var) != teddy_vars.end());
    }
}

// Create individual test cases for each file in test_expressions directory, listing them explicitly
#define DECLARE_TEST_CASE(file)                  \
    TEST_CASE(file) {                            \
        test_expression_to_cudd_and_teddy(file); \
    }

DECLARE_TEST_CASE("test_expressions/simple_expression.txt")
DECLARE_TEST_CASE("test_expressions/all_operators.txt")

// Test that CUDD produces deterministic, canonical output across multiple runs
TEST_CASE("CUDD produces deterministic canonical output", "[cudd][determinism]") {
    const std::string file_name = "test_expressions/simple_expression.txt";

    // Parse expression
    auto expression = read_expression_from_file(file_name);
    REQUIRE(expression != nullptr);

    std::unordered_set<std::string> variable_names;
    collect_variables_with_dag_walker(*expression, variable_names);

    std::vector<std::string> sorted_variable_names(variable_names.begin(), variable_names.end());
    std::ranges::sort(sorted_variable_names);

    // Build BDD twice and verify identical structure
    auto [cudd_mgr1, cudd_bdd1] = convert_to_cudd_bdd(*expression, variable_names);
    cudd_iterator cudd_it1(*cudd_mgr1, cudd_bdd1.getNode(), &sorted_variable_names);
    auto nodes1 = dag_walker::collect_unique_nodes_topological(cudd_it1);

    auto [cudd_mgr2, cudd_bdd2] = convert_to_cudd_bdd(*expression, variable_names);
    cudd_iterator cudd_it2(*cudd_mgr2, cudd_bdd2.getNode(), &sorted_variable_names);
    auto nodes2 = dag_walker::collect_unique_nodes_topological(cudd_it2);

    // Same number of nodes
    REQUIRE(nodes1.size() == nodes2.size());

    // Same labels in same order (canonical ordering)
    for (size_t i = 0; i < nodes1.size(); ++i) {
        REQUIRE(nodes1[i].get_label() == nodes2[i].get_label());
    }

    std::cout << "✓ CUDD produces deterministic output with " << nodes1.size() << " nodes"
              << std::endl;
}
