// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file tests/unit/test_mermaid_graph_generator.cpp
 * @brief Tests for Mermaid flowchart generation
 *
 * Ensures Mermaid output contains expected nodes, edges, and class definitions
 * for different operator types and variable nodes.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <sstream>

#include "expression_graph.hpp"
#include "expression_types.hpp"

using Catch::Matchers::ContainsSubstring;

TEST_CASE("Mermaid generation - single variable and simple AND/OR", "[mermaid]") {
    my_expression expr = my_variable{"x"};
    std::ostringstream out;

    write_expression_to_mermaid(expr, out, "TestGraph");
    auto result = out.str();
    REQUIRE_THAT(result, ContainsSubstring("flowchart TD"));
    REQUIRE_THAT(result, ContainsSubstring("x"));

    expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                  std::make_unique<my_expression>(my_variable{"b"})};
    out.str(std::string());
    out.clear();
    write_expression_to_mermaid(expr, out, "ANDGraph");
    result = out.str();
    REQUIRE_THAT(result, ContainsSubstring("flowchart TD"));
    REQUIRE_THAT(result, ContainsSubstring("AND"));
    REQUIRE_THAT(result, ContainsSubstring("a"));
    REQUIRE_THAT(result, ContainsSubstring("b"));
}

TEST_CASE("Mermaid generation - class assignments and shape hints", "[mermaid]") {
    my_expression expr = my_or{std::make_unique<my_expression>(my_variable{"a"}),
                               std::make_unique<my_expression>(my_variable{"b"})};
    std::ostringstream out;
    write_expression_to_mermaid(expr, out, "ClassGraph");
    auto result = out.str();

    REQUIRE_THAT(result, ContainsSubstring("class"));
    REQUIRE_THAT(result, ContainsSubstring("variable"));
}

TEST_CASE("Mermaid generation - negative cases: empty title and long/quoted labels",
          "[mermaid][negative]") {
    // Empty title should be handled
    my_expression expr = my_variable{"x"};
    std::ostringstream out;
    write_expression_to_mermaid(expr, out, "");
    auto result = out.str();
    REQUIRE_THAT(result, ContainsSubstring("flowchart"));

    // Very long label
    std::string longname(1024, 'a');
    my_expression long_expr = my_variable{longname};
    out.str(std::string());
    out.clear();
    write_expression_to_mermaid(long_expr, out, "LongGraph");
    result = out.str();
    REQUIRE_THAT(result, ContainsSubstring("flowchart TD"));

    // Label with quotes should be escaped
    my_expression qexpr = my_variable{"bad\"name"};
    out.str(std::string());
    out.clear();
    write_expression_to_mermaid(qexpr, out, "QuoteGraph");
    result = out.str();
    // Ensure quotes are balanced (heuristic)
    size_t unescaped_quotes = 0;
    for (size_t i = 0; i < result.size(); ++i) {
        if (result[i] == '"' && (i == 0 || result[i - 1] != '\\'))
            ++unescaped_quotes;
    }
    REQUIRE(unescaped_quotes % 2 == 0);
}
