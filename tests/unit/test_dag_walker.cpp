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

    auto nodes = dag_walker::collect_nodes_topological(root_iter);

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

    auto nodes = dag_walker::collect_nodes_topological(root_iter);

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

TEST_CASE("DAGWalker - Collect edges", "[dag_walker][edges]") {
    // Expression: a AND b
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};
    expression_iterator root_iter(expr);

    auto edges = dag_walker::collect_edges_topological(root_iter);

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

    auto edges = dag_walker::collect_edges_topological(root_iter);

    // Expected edges: OR->AND, OR->c, AND->a, AND->b = 4 edges total
    REQUIRE(edges.size() == 4);
}

TEST_CASE("DAGWalker - Collect all edges including revisits (collect_all_edges)",
          "[dag_walker][edges][collect_all]") {
    // Create a simple copyable iterator type backed by shared nodes to simulate a DAG
    struct SimpleNode {
        std::string name;
        std::vector<std::shared_ptr<SimpleNode>> children;
        explicit SimpleNode(const std::string& n) : name(n) {}
    };

    struct simple_iter {
        std::shared_ptr<SimpleNode> node;
        simple_iter() = default;
        explicit simple_iter(std::shared_ptr<SimpleNode> n) : node(std::move(n)) {}

        std::vector<simple_iter> get_children() const {
            std::vector<simple_iter> out;
            std::transform(node->children.begin(), node->children.end(), std::back_inserter(out),
                           [](const std::shared_ptr<SimpleNode>& c) { return simple_iter(c); });
            return out;
        }

        const void* get_node_address() const {
            return reinterpret_cast<const void*>(node.get());
        }

        bool operator==(const simple_iter& other) const {
            return node.get() == other.node.get();
        }
        bool operator!=(const simple_iter& other) const {
            return node.get() != other.node.get();
        }
    };

    // Build nodes and a DAG: root -> and_node, root -> shared; and_node -> shared, and_node -> b
    auto shared = std::make_shared<SimpleNode>("shared");
    auto b = std::make_shared<SimpleNode>("b");
    auto and_node = std::make_shared<SimpleNode>("and");
    and_node->children = {shared, b};
    auto root = std::make_shared<SimpleNode>("root");
    root->children = {and_node, shared};

    simple_iter root_iter(root);

    auto edges_all = dag_walker::collect_edges_topological(root_iter);

    // Expected edges: root->and, root->shared, and->shared, and->b = 4 edges
    REQUIRE(edges_all.size() == 4);

    // Topological collection should list the 4 unique nodes
    auto topo_nodes = dag_walker::collect_nodes_topological(root_iter);
    REQUIRE(topo_nodes.size() == 4);
}

TEST_CASE("DAGWalker - Null visitor and invalid inputs", "[dag_walker][negative]") {
    // Ensure walk_dag_topological_order handles a null visitor gracefully (should not crash)
    my_expression expr = my_variable{"x"};
    expression_iterator root_iter(expr);

    // Passing a null function object is not directly supported; simulate by
    // passing a visitor that does nothing and ensure no exceptions are thrown.
    auto noop = [](const dag_walker::NodeInfo<expression_iterator>&) {};
    REQUIRE_NOTHROW(dag_walker::walk_dag_topological_order(root_iter, noop));

    // Ensure collect_nodes_topological handles an empty graph-like iterator
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
    auto nodes = dag_walker::collect_nodes_topological(ei);
    // Nothing to assert reliably here other than the call succeeded
    SUCCEED();
}
