// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file test_dag_walker.cpp
 * @brief Unit tests for the DAG walker algorithms
 *
 * This file contains unit tests for the dag_walker namespace functions,
 * testing DAG traversal, node collection, and edge collection functionality.
 */

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <iterator>
#include <sstream>

#include "dag_walker.hpp"
#include "expression_iterator.hpp"
#include "expression_types.hpp"

TEST_CASE("DAGWalker - Walk simple expression tree", "[dag_walker][traversal]") {
    // Simple expression: single variable "x"
    my_expression expr = my_variable{"x"};
    expression_iterator root_iter(expr);

    int visit_count = 0;
    auto visitor = [&visit_count](const dag_walker::NodeInfo<expression_iterator>& node_info) {
        visit_count++;
    };

    dag_walker::walk_dag_topological_order(root_iter, visitor);

    REQUIRE(visit_count == 1);  // Only the variable node
}

TEST_CASE("DAGWalker - Walk AND expression", "[dag_walker][traversal]") {
    // Expression: a AND b
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};
    expression_iterator root_iter(expr);

    int visit_count = 0;
    auto visitor = [&visit_count](const dag_walker::NodeInfo<expression_iterator>& node_info) {
        visit_count++;
    };

    dag_walker::walk_dag_topological_order(root_iter, visitor);

    REQUIRE(visit_count == 3);  // AND node + 2 variable nodes
}

TEST_CASE("DAGWalker - Walk OR expression", "[dag_walker][traversal]") {
    // Expression: x OR y
    my_expression expr = my_or{std::make_unique<my_expression>(my_variable{"x"}),
                               std::make_unique<my_expression>(my_variable{"y"})};
    expression_iterator root_iter(expr);

    int visit_count = 0;
    auto visitor = [&visit_count](const dag_walker::NodeInfo<expression_iterator>& node_info) {
        visit_count++;
    };

    dag_walker::walk_dag_topological_order(root_iter, visitor);

    REQUIRE(visit_count == 3);  // OR node + 2 variable nodes
}

TEST_CASE("DAGWalker - Walk NOT expression", "[dag_walker][traversal]") {
    // Expression: NOT x
    my_expression expr = my_not{std::make_unique<my_expression>(my_variable{"x"})};
    expression_iterator root_iter(expr);

    int visit_count = 0;
    auto visitor = [&visit_count](const dag_walker::NodeInfo<expression_iterator>& node_info) {
        visit_count++;
    };

    dag_walker::walk_dag_topological_order(root_iter, visitor);

    REQUIRE(visit_count == 2);  // NOT node + variable node
}

TEST_CASE("DAGWalker - Walk nested expression", "[dag_walker][traversal]") {
    // Expression: (a AND b) OR c
    my_expression expr = my_or{
        std::make_unique<my_expression>(my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                               std::make_unique<my_expression>(my_variable{"b"})}),
        std::make_unique<my_expression>(my_variable{"c"})};
    expression_iterator root_iter(expr);

    int visit_count = 0;
    auto visitor = [&visit_count](const dag_walker::NodeInfo<expression_iterator>& node_info) {
        visit_count++;
    };

    dag_walker::walk_dag_topological_order(root_iter, visitor);

    REQUIRE(visit_count == 5);  // OR + AND + 3 variables
}

TEST_CASE("DAGWalker - Collect unique nodes topological", "[dag_walker][collection]") {
    // Expression: (a AND b)
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};
    expression_iterator root_iter(expr);

    auto [nodes, edges] = dag_walker::collect_nodes_and_edges_topological(root_iter);
    (void)edges;

    REQUIRE(nodes.size() == 3);
    // In topological order, children come before parents
    // So we expect: variable "a", variable "b", then AND node
}

TEST_CASE("DAGWalker - Collect nodes from expression with repeated variable names",
          "[dag_walker][collection]") {
    // Expression: (a AND b) OR a  - 'a' appears twice as separate nodes
    my_expression expr = my_or{
        std::make_unique<my_expression>(my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                               std::make_unique<my_expression>(my_variable{"b"})}),
        std::make_unique<my_expression>(my_variable{"a"})};
    expression_iterator root_iter(expr);

    auto [nodes, edges] = dag_walker::collect_nodes_and_edges_topological(root_iter);
    (void)edges;

    // In expression trees, each node is unique even if variable names repeat
    // Expect: 5 nodes total (a, b, AND, a, OR) - two separate 'a' nodes
    REQUIRE(nodes.size() == 5);
}

TEST_CASE("DAGWalker - Parent tracking in NodeInfo", "[dag_walker][traversal]") {
    // Expression: NOT x
    my_expression expr = my_not{std::make_unique<my_expression>(my_variable{"x"})};
    expression_iterator root_iter(expr);

    int nodes_with_parent = 0;
    auto visitor =
        [&nodes_with_parent](const dag_walker::NodeInfo<expression_iterator>& node_info) {
            if (node_info.parent != nullptr) {
                nodes_with_parent++;
            }
        };

    dag_walker::walk_dag_topological_order(root_iter, visitor);

    REQUIRE(nodes_with_parent == 1);  // Only the child has a parent
}

TEST_CASE("DAGWalker - Null visitor and invalid inputs", "[dag_walker][negative]") {
    // Ensure walk_dag_topological_order handles a null visitor gracefully (should not crash)
    my_expression expr = my_variable{"x"};
    expression_iterator root_iter(expr);

    // Passing a null function object is not directly supported; simulate by
    // passing a visitor that does nothing and ensure no exceptions are thrown.
    auto noop = [](const dag_walker::NodeInfo<expression_iterator>&) {};
    REQUIRE_NOTHROW(dag_walker::walk_dag_topological_order(root_iter, noop));

    // Ensure collect_nodes_and_edges_topological handles an empty graph-like iterator
    struct empty_iter {
        std::vector<empty_iter> get_children() const {
            return {};
        }
        const void* get_node_address() const {
            return nullptr;
        }
        bool operator==(const empty_iter&) const {
            return true;
        }
        bool operator!=(const empty_iter&) const {
            return false;
        }
    };

    empty_iter ei;
    auto [nodes, edges] = dag_walker::collect_nodes_and_edges_topological(ei);
    (void)edges;
    // Nothing to assert reliably here other than the call succeeded
    SUCCEED();
}
