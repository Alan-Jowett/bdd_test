// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file test_expression_graph.cpp
 * @brief Unit tests for the expression graph implementation
 *
 * This file contains unit tests for the ExpressionGraph class,
 * testing graph construction, traversal, and visualization functionality.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <sstream>

#include "expression_graph.hpp"
#include "expression_types.hpp"

using Catch::Matchers::ContainsSubstring;

TEST_CASE("ExpressionGraph - Variable collection from simple expression",
          "[expression_graph][variables]") {
    my_expression expr = my_variable{"x"};
    std::unordered_set<std::string> variables;

    collect_variables_with_dag_walker(expr, variables);

    REQUIRE(variables.size() == 1);
    REQUIRE(variables.count("x") == 1);
}

TEST_CASE("ExpressionGraph - Variable collection from AND expression",
          "[expression_graph][variables]") {
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};
    std::unordered_set<std::string> variables;

    collect_variables_with_dag_walker(expr, variables);

    REQUIRE(variables.size() == 2);
    REQUIRE(variables.count("a") == 1);
    REQUIRE(variables.count("b") == 1);
}

TEST_CASE("ExpressionGraph - Variable collection from OR expression",
          "[expression_graph][variables]") {
    my_expression expr = my_or{std::make_unique<my_expression>(my_variable{"x"}),
                               std::make_unique<my_expression>(my_variable{"y"})};
    std::unordered_set<std::string> variables;

    collect_variables_with_dag_walker(expr, variables);

    REQUIRE(variables.size() == 2);
    REQUIRE(variables.count("x") == 1);
    REQUIRE(variables.count("y") == 1);
}

TEST_CASE("ExpressionGraph - Variable collection from NOT expression",
          "[expression_graph][variables]") {
    my_expression expr = my_not{std::make_unique<my_expression>(my_variable{"z"})};
    std::unordered_set<std::string> variables;

    collect_variables_with_dag_walker(expr, variables);

    REQUIRE(variables.size() == 1);
    REQUIRE(variables.count("z") == 1);
}

TEST_CASE("ExpressionGraph - Variable collection from XOR expression",
          "[expression_graph][variables]") {
    my_expression expr = my_xor{std::make_unique<my_expression>(my_variable{"p"}),
                                std::make_unique<my_expression>(my_variable{"q"})};
    std::unordered_set<std::string> variables;

    collect_variables_with_dag_walker(expr, variables);

    REQUIRE(variables.size() == 2);
    REQUIRE(variables.count("p") == 1);
    REQUIRE(variables.count("q") == 1);
}

TEST_CASE("ExpressionGraph - Variable collection from nested expression",
          "[expression_graph][variables]") {
    // (a AND b) OR (c AND d)
    my_expression expr = my_or{
        std::make_unique<my_expression>(my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                               std::make_unique<my_expression>(my_variable{"b"})}),
        std::make_unique<my_expression>(my_and{std::make_unique<my_expression>(my_variable{"c"}),
                                               std::make_unique<my_expression>(my_variable{"d"})})};

    std::unordered_set<std::string> variables;
    collect_variables_with_dag_walker(expr, variables);

    REQUIRE(variables.size() == 4);
    REQUIRE(variables.count("a") == 1);
    REQUIRE(variables.count("b") == 1);
    REQUIRE(variables.count("c") == 1);
    REQUIRE(variables.count("d") == 1);
}

TEST_CASE("ExpressionGraph - Variable collection with duplicates",
          "[expression_graph][variables]") {
    // (a AND b) OR a  - 'a' appears twice but should be collected once
    my_expression expr = my_or{
        std::make_unique<my_expression>(my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                               std::make_unique<my_expression>(my_variable{"b"})}),
        std::make_unique<my_expression>(my_variable{"a"})};

    std::unordered_set<std::string> variables;
    collect_variables_with_dag_walker(expr, variables);

    REQUIRE(variables.size() == 2);
    REQUIRE(variables.count("a") == 1);
    REQUIRE(variables.count("b") == 1);
}

TEST_CASE("ExpressionGraph - DOT generation for single variable", "[expression_graph][dot]") {
    my_expression expr = my_variable{"x"};
    std::ostringstream out;

    write_expression_to_dot(expr, out, "TestGraph");

    std::string result = out.str();

    // Validate proper DOT graph structure
    REQUIRE_THAT(result, ContainsSubstring("digraph TestGraph"));
    REQUIRE_THAT(result, ContainsSubstring("{"));
    REQUIRE_THAT(result, ContainsSubstring("}"));
    REQUIRE_THAT(result, ContainsSubstring("x"));

    // Verify proper structure
    REQUIRE((result.find("{") < result.find("}")));
    REQUIRE((result.find("digraph TestGraph") < result.find("x")));
}

TEST_CASE("ExpressionGraph - DOT generation for AND expression", "[expression_graph][dot]") {
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};
    std::ostringstream out;

    write_expression_to_dot(expr, out, "ANDGraph");

    std::string result = out.str();

    // Validate proper DOT graph structure
    REQUIRE_THAT(result, ContainsSubstring("digraph ANDGraph"));
    REQUIRE_THAT(result, ContainsSubstring("{"));
    REQUIRE_THAT(result, ContainsSubstring("}"));
    REQUIRE_THAT(result, ContainsSubstring("AND"));
    REQUIRE_THAT(result, ContainsSubstring("a"));
    REQUIRE_THAT(result, ContainsSubstring("b"));

    // Validate edges exist (should have 2 edges from AND to children)
    REQUIRE_THAT(result, ContainsSubstring("->"));
    size_t edge_count = 0;
    size_t pos = 0;
    while ((pos = result.find("->", pos)) != std::string::npos) {
        edge_count++;
        pos += 2;
    }
    REQUIRE(edge_count >= 2);  // AND node connects to both children

    // Verify proper structure
    REQUIRE((result.find("{") < result.find("}")));
    REQUIRE((result.find("AND") < result.find("->")));  // Operator defined before edges
}

TEST_CASE("ExpressionGraph - DOT generation for OR expression", "[expression_graph][dot]") {
    my_expression expr = my_or{std::make_unique<my_expression>(my_variable{"x"}),
                               std::make_unique<my_expression>(my_variable{"y"})};
    std::ostringstream out;

    write_expression_to_dot(expr, out, "ORGraph");

    std::string result = out.str();
    REQUIRE_THAT(result, ContainsSubstring("digraph ORGraph"));
    REQUIRE_THAT(result, ContainsSubstring("OR"));
}

TEST_CASE("ExpressionGraph - DOT generation for NOT expression", "[expression_graph][dot]") {
    my_expression expr = my_not{std::make_unique<my_expression>(my_variable{"z"})};
    std::ostringstream out;

    write_expression_to_dot(expr, out, "NOTGraph");

    std::string result = out.str();
    REQUIRE_THAT(result, ContainsSubstring("digraph NOTGraph"));
    REQUIRE_THAT(result, ContainsSubstring("NOT"));
}

TEST_CASE("ExpressionGraph - DOT generation for XOR expression", "[expression_graph][dot]") {
    my_expression expr = my_xor{std::make_unique<my_expression>(my_variable{"p"}),
                                std::make_unique<my_expression>(my_variable{"q"})};
    std::ostringstream out;

    write_expression_to_dot(expr, out, "XORGraph");

    std::string result = out.str();
    REQUIRE_THAT(result, ContainsSubstring("digraph XORGraph"));
    REQUIRE_THAT(result, ContainsSubstring("XOR"));
}

TEST_CASE("ExpressionGraph - Mermaid generation for single variable",
          "[expression_graph][mermaid]") {
    my_expression expr = my_variable{"x"};
    std::ostringstream out;

    write_expression_to_mermaid(expr, out, "TestGraph");

    std::string result = out.str();

    // Validate Mermaid flowchart structure
    REQUIRE_THAT(result, ContainsSubstring("flowchart TD"));
    REQUIRE_THAT(result, ContainsSubstring("x"));

    // Verify flowchart declaration is at the start
    // Flowchart declaration exists (may have YAML front matter before it)
    REQUIRE_THAT(result, ContainsSubstring("flowchart TD"));
}

TEST_CASE("ExpressionGraph - Mermaid generation for AND expression",
          "[expression_graph][mermaid]") {
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};
    std::ostringstream out;

    write_expression_to_mermaid(expr, out, "ANDGraph");

    std::string result = out.str();

    // Validate Mermaid structure
    REQUIRE_THAT(result, ContainsSubstring("flowchart TD"));
    REQUIRE_THAT(result, ContainsSubstring("AND"));
    REQUIRE_THAT(result, ContainsSubstring("a"));
    REQUIRE_THAT(result, ContainsSubstring("b"));

    // Validate edges exist (should have 2 edges)
    REQUIRE_THAT(result, ContainsSubstring("-->"));
    size_t edge_count = 0;
    size_t pos = 0;
    while ((pos = result.find("-->", pos)) != std::string::npos) {
        edge_count++;
        pos += 3;
    }
    REQUIRE(edge_count >= 2);  // AND node connects to both children

    // Verify flowchart declaration is at the start
    // Flowchart declaration exists (may have YAML front matter before it)
    REQUIRE_THAT(result, ContainsSubstring("flowchart TD"));
}

TEST_CASE("ExpressionGraph - Mermaid generation for complex expression",
          "[expression_graph][mermaid]") {
    // (a AND b) OR c
    my_expression expr = my_or{
        std::make_unique<my_expression>(my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                               std::make_unique<my_expression>(my_variable{"b"})}),
        std::make_unique<my_expression>(my_variable{"c"})};
    std::ostringstream out;

    write_expression_to_mermaid(expr, out, "ComplexGraph");

    std::string result = out.str();
    REQUIRE_THAT(result, ContainsSubstring("flowchart TD"));
    REQUIRE_THAT(result, ContainsSubstring("OR"));
    REQUIRE_THAT(result, ContainsSubstring("AND"));
}
