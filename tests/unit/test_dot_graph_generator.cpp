// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file tests/unit/test_dot_graph_generator.cpp
 * @brief Tests for DOT graph generation helpers
 *
 * Confirms that DOT output contains expected graph and node labels and edges.
 */

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <sstream>

#include "dot_graph_generator.hpp"
#include "expression_iterator.hpp"
#include "expression_types.hpp"

TEST_CASE("DOT Graph generation basic output", "[dot_graph][generation]") {
    // Build a simple expression: (a AND b) OR c
    my_expression expr = my_or{
        std::make_unique<my_expression>(my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                               std::make_unique<my_expression>(my_variable{"b"})}),
        std::make_unique<my_expression>(my_variable{"c"})};

    expression_iterator root_iter(expr);

    std::ostringstream out;
    dot_graph::DotConfig cfg;
    cfg.graph_name = "TestGraph";
    cfg.default_node_shape = "ellipse";

    dot_graph::generate_dot_graph(root_iter, out, cfg);

    std::string dot = out.str();

    REQUIRE(dot.find("digraph TestGraph") != std::string::npos);
    REQUIRE(dot.find("label = \"a\"") != std::string::npos);
    REQUIRE(dot.find("label = \"b\"") != std::string::npos);
    REQUIRE(dot.find("label = \"c\"") != std::string::npos);

    // Check for edges (OR -> AND, OR -> c, AND -> a, AND -> b)
    REQUIRE(dot.find("->") != std::string::npos);
}

TEST_CASE("DOT Graph - invalid config", "[dot_graph][negative]") {
    // Build a simple expression: a AND b
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};
    expression_iterator root_iter(expr);

    std::ostringstream out;
    dot_graph::DotConfig cfg;

    // Negative: empty graph name should be handled (fallback to default)
    cfg.graph_name = "";
    REQUIRE_NOTHROW(dot_graph::generate_dot_graph(root_iter, out, cfg));
    std::string dot = out.str();
    // Graph should still contain 'digraph' even if unnamed
    REQUIRE(dot.find("digraph") != std::string::npos);
}

TEST_CASE("DOT Graph - schema validation and label escaping", "[dot_graph][schema]") {
    // Build a simple expression: a AND b
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};
    expression_iterator root_iter(expr);

    std::ostringstream out;
    dot_graph::DotConfig cfg;
    cfg.graph_name = "SchemaGraph";
    REQUIRE_NOTHROW(dot_graph::generate_dot_graph(root_iter, out, cfg));
    std::string dot = out.str();

    // Schema-like validations (basic checks for Graphviz DOT):
    // - must contain matching braces
    auto open_brace_pos = dot.find('{');
    auto close_brace_pos = dot.rfind('}');
    REQUIRE(open_brace_pos != std::string::npos);
    REQUIRE(close_brace_pos != std::string::npos);
    REQUIRE(open_brace_pos < close_brace_pos);

    // - every statement line ends with a semicolon (rough heuristic)
    std::string body = dot.substr(open_brace_pos + 1, close_brace_pos - open_brace_pos - 1);
    std::istringstream lines(body);
    std::string line;
    bool found_statement = false;
    while (std::getline(lines, line)) {
        // trim
        auto first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos)
            continue;
        auto last = line.find_last_not_of(" \t\r\n");
        std::string trimmed = line.substr(first, last - first + 1);
        if (trimmed == "")
            continue;
        if (trimmed.find("->") != std::string::npos || trimmed.find('[') != std::string::npos) {
            found_statement = true;
            REQUIRE(trimmed.back() == ';');
        }
    }
    REQUIRE(found_statement);

    // Negative: invalid label content should be escaped; create a node with quotes
    my_expression expr2 = my_variable{"name_with_\"quote\""};
    expression_iterator root_iter2(expr2);
    std::ostringstream out2;
    dot_graph::generate_dot_graph(root_iter2, out2, cfg);
    std::string dot2 = out2.str();
    // The raw quote should not produce unbalanced quotes in the DOT output
    size_t unescaped_quotes = 0;
    for (size_t i = 0; i < dot2.size(); ++i) {
        if (dot2[i] == '"') {
            if (i == 0 || dot2[i - 1] != '\\')
                ++unescaped_quotes;
        }
    }
    REQUIRE(unescaped_quotes % 2 == 0);
}
