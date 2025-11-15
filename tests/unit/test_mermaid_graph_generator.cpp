<<<<<<< HEAD
// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file tests/unit/test_mermaid_graph_generator.cpp
 * @brief Tests for Mermaid flowchart generation
 *
 * Ensures Mermaid output contains expected nodes, edges, and class definitions
 * for different operator types and variable nodes.
 */

=======
>>>>>>> a7337d6 (tests: add multiple unit tests and update test configuration)
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
