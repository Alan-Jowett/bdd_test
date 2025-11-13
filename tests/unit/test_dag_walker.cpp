// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file test_dag_walker.cpp
 * @brief Unit tests for the DAG walker algorithms
 *
 * This file contains unit tests for the dag_walker namespace functions,
 * testing DAG traversal, node collection, and edge collection functionality.
 */

#include <catch2/catch_test_macros.hpp>
#include <sstream>

#include "dag_walker.hpp"
#include "expression_iterator.hpp"
#include "expression_types.hpp"

TEST_CASE("DAGWalker - WalkConfig default values", "[dag_walker][config]") {
    dag_walker::WalkConfig config;

    REQUIRE(config.track_unique_nodes == true);
    REQUIRE(config.call_visitor_on_revisit == false);
    REQUIRE(config.collect_all_edges == false);
}

TEST_CASE("DAGWalker - WalkConfig custom constructor", "[dag_walker][config]") {
    dag_walker::WalkConfig config(false);

    REQUIRE(config.track_unique_nodes == false);
}

TEST_CASE("DAGWalker - WalkConfig comparison", "[dag_walker][config]") {
    dag_walker::WalkConfig config1;
    dag_walker::WalkConfig config2;

    REQUIRE(config1 == config2);

    config1.track_unique_nodes = false;
    REQUIRE(config1 != config2);
}

TEST_CASE("DAGWalker - Walk simple expression tree", "[dag_walker][traversal]") {
    // Simple expression: single variable "x"
    my_expression expr = my_variable{"x"};
    expression_iterator root_iter(expr);

    int visit_count = 0;
    auto visitor = [&visit_count](const dag_walker::NodeInfo<expression_iterator>& node_info) {
        visit_count++;
    };

    dag_walker::walk_dag(root_iter, visitor);

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

    dag_walker::walk_dag(root_iter, visitor);

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

    dag_walker::walk_dag(root_iter, visitor);

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

    dag_walker::walk_dag(root_iter, visitor);

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

    dag_walker::walk_dag(root_iter, visitor);

    REQUIRE(visit_count == 5);  // OR + AND + 3 variables
}

TEST_CASE("DAGWalker - Node depth tracking", "[dag_walker][traversal]") {
    // Expression: (a AND b)
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};
    expression_iterator root_iter(expr);

    std::vector<size_t> depths;
    auto visitor = [&depths](const dag_walker::NodeInfo<expression_iterator>& node_info) {
        depths.push_back(node_info.depth);
    };

    dag_walker::walk_dag(root_iter, visitor);

    REQUIRE(depths.size() == 3);
    REQUIRE(depths[0] == 0);  // Root (AND) at depth 0
    REQUIRE(depths[1] == 1);  // First child at depth 1
    REQUIRE(depths[2] == 1);  // Second child at depth 1
}

TEST_CASE("DAGWalker - Collect unique nodes topological", "[dag_walker][collection]") {
    // Expression: (a AND b)
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};
    expression_iterator root_iter(expr);

    auto nodes = dag_walker::collect_unique_nodes_topological(root_iter);

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

    auto nodes = dag_walker::collect_unique_nodes_topological(root_iter);

    // In expression trees, each node is unique even if variable names repeat
    // Expect: 5 nodes total (a, b, AND, a, OR) - two separate 'a' nodes
    REQUIRE(nodes.size() == 5);
}

TEST_CASE("DAGWalker - Walk with custom config", "[dag_walker][config]") {
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};
    expression_iterator root_iter(expr);

    int visit_count = 0;
    auto visitor = [&visit_count](const dag_walker::NodeInfo<expression_iterator>& node_info) {
        visit_count++;
    };

    dag_walker::WalkConfig config;
    config.track_unique_nodes = false;  // Visit all nodes even if seen before

    dag_walker::walk_dag(root_iter, visitor, config);

    REQUIRE(visit_count == 3);
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

    dag_walker::walk_dag(root_iter, visitor);

    REQUIRE(nodes_with_parent == 1);  // Only the child has a parent
}

TEST_CASE("DAGWalker - Revisit flag detection", "[dag_walker][traversal]") {
    // Expression with shared node: (a AND b) OR a
    my_expression expr = my_or{
        std::make_unique<my_expression>(my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                               std::make_unique<my_expression>(my_variable{"b"})}),
        std::make_unique<my_expression>(my_variable{"a"})};
    expression_iterator root_iter(expr);

    int revisit_count = 0;
    auto visitor = [&revisit_count](const dag_walker::NodeInfo<expression_iterator>& node_info) {
        if (node_info.is_revisit) {
            revisit_count++;
        }
    };

    dag_walker::walk_dag(root_iter, visitor);

    // Note: In a tree structure like expressions, there are no revisits
    // because each node is unique even if it contains the same variable name
    REQUIRE(revisit_count == 0);
}

TEST_CASE("DAGWalker - Collect edges", "[dag_walker][edges]") {
    // Expression: a AND b
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};
    expression_iterator root_iter(expr);

    auto edges = dag_walker::collect_edges(root_iter);

    // Should have 2 edges: AND->a and AND->b
    REQUIRE(edges.size() == 2);
}

TEST_CASE("DAGWalker - Collect edges from nested expression", "[dag_walker][edges]") {
    // Expression: (a AND b) OR c
    my_expression expr = my_or{
        std::make_unique<my_expression>(my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                               std::make_unique<my_expression>(my_variable{"b"})}),
        std::make_unique<my_expression>(my_variable{"c"})};
    expression_iterator root_iter(expr);

    auto edges = dag_walker::collect_edges(root_iter);

    // Expected edges: OR->AND, OR->c, AND->a, AND->b = 4 edges total
    REQUIRE(edges.size() == 4);
}
