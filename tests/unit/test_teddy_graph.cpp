// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file test_teddy_graph.cpp
 * @brief Unit tests for the TeDDy BDD graph implementation
 *
 * This file contains unit tests for the TeddyGraph class,
 * testing BDD construction, optimization, and analysis functionality.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <libteddy/core.hpp>
#include <sstream>

#include "teddy_graph.hpp"

using Catch::Matchers::ContainsSubstring;

TEST_CASE("TeddyGraph - Basic BDD manager creation", "[teddy_graph][basic]") {
    teddy::bdd_manager manager(2, 1000);
    // Successfully created manager with 2 variables and cache size 1000
    auto var0 = manager.variable(0);
    auto var1 = manager.variable(1);
    REQUIRE(var0.unsafe_get_root() != nullptr);
    REQUIRE(var1.unsafe_get_root() != nullptr);
}

TEST_CASE("TeddyGraph - Create single variable BDD", "[teddy_graph][basic]") {
    teddy::bdd_manager manager(1, 1000);
    auto var0 = manager.variable(0);

    REQUIRE(var0.unsafe_get_root() != nullptr);
}

TEST_CASE("TeddyGraph - AND operation", "[teddy_graph][operations]") {
    using namespace teddy::ops;
    teddy::bdd_manager manager(2, 1000);
    auto var0 = manager.variable(0);
    auto var1 = manager.variable(1);

    auto result = manager.apply<AND>(var0, var1);

    REQUIRE(result.unsafe_get_root() != nullptr);
    // AND of two variables should create additional nodes beyond terminals
}

TEST_CASE("TeddyGraph - OR operation", "[teddy_graph][operations]") {
    using namespace teddy::ops;
    teddy::bdd_manager manager(2, 1000);
    auto var0 = manager.variable(0);
    auto var1 = manager.variable(1);

    auto result = manager.apply<OR>(var0, var1);

    REQUIRE(result.unsafe_get_root() != nullptr);
}

TEST_CASE("TeddyGraph - XOR operation", "[teddy_graph][operations]") {
    using namespace teddy::ops;
    teddy::bdd_manager manager(2, 1000);
    auto var0 = manager.variable(0);
    auto var1 = manager.variable(1);

    auto result = manager.apply<XOR>(var0, var1);

    REQUIRE(result.unsafe_get_root() != nullptr);
}

TEST_CASE("TeddyGraph - NOT operation", "[teddy_graph][operations]") {
    using namespace teddy::ops;
    teddy::bdd_manager manager(1, 1000);
    auto var0 = manager.variable(0);
    auto one = manager.constant(1);

    auto result = manager.apply<XOR>(var0, one);  // NOT is XOR with 1

    REQUIRE(result.unsafe_get_root() != nullptr);
}

TEST_CASE("TeddyGraph - Complex nested operations", "[teddy_graph][operations]") {
    using namespace teddy::ops;
    teddy::bdd_manager manager(3, 1000);
    auto var0 = manager.variable(0);
    auto var1 = manager.variable(1);
    auto var2 = manager.variable(2);

    // (var0 AND var1) OR var2
    auto and_result = manager.apply<AND>(var0, var1);
    auto final_result = manager.apply<OR>(and_result, var2);

    REQUIRE(final_result.unsafe_get_root() != nullptr);
}

TEST_CASE("TeddyGraph - DOT generation for single variable", "[teddy_graph][dot]") {
    teddy::bdd_manager manager(1, 1000);
    auto var0 = manager.variable(0);
    std::vector<std::string> var_names = {"x"};

    std::ostringstream out;
    write_teddy_to_dot(manager, var0, var_names, out, "TestGraph");

    std::string result = out.str();
    REQUIRE_THAT(result, ContainsSubstring("digraph TestGraph"));
    REQUIRE_THAT(result, ContainsSubstring("x"));
}

TEST_CASE("TeddyGraph - DOT generation for AND operation", "[teddy_graph][dot]") {
    using namespace teddy::ops;
    teddy::bdd_manager manager(2, 1000);
    auto var0 = manager.variable(0);
    auto var1 = manager.variable(1);
    auto and_result = manager.apply<AND>(var0, var1);

    std::vector<std::string> var_names = {"a", "b"};

    std::ostringstream out;
    write_teddy_to_dot(manager, and_result, var_names, out, "ANDGraph");

    std::string result = out.str();
    REQUIRE_THAT(result, ContainsSubstring("digraph ANDGraph"));
    REQUIRE_THAT(result, ContainsSubstring("a"));
    REQUIRE_THAT(result, ContainsSubstring("b"));
}

TEST_CASE("TeddyGraph - Mermaid generation for single variable", "[teddy_graph][mermaid]") {
    teddy::bdd_manager manager(1, 1000);
    auto var0 = manager.variable(0);
    std::vector<std::string> var_names = {"x"};

    std::ostringstream out;
    write_teddy_to_mermaid(manager, var0, var_names, out, "TestGraph");

    std::string result = out.str();
    REQUIRE_THAT(result, ContainsSubstring("flowchart TD"));
    REQUIRE_THAT(result, ContainsSubstring("x"));
}

TEST_CASE("TeddyGraph - Mermaid generation for OR operation", "[teddy_graph][mermaid]") {
    using namespace teddy::ops;
    teddy::bdd_manager manager(2, 1000);
    auto var0 = manager.variable(0);
    auto var1 = manager.variable(1);
    auto or_result = manager.apply<OR>(var0, var1);

    std::vector<std::string> var_names = {"p", "q"};

    std::ostringstream out;
    write_teddy_to_mermaid(manager, or_result, var_names, out, "ORGraph");

    std::string result = out.str();
    REQUIRE_THAT(result, ContainsSubstring("flowchart TD"));
}

TEST_CASE("TeddyGraph - Node collection topological order", "[teddy_graph][analysis]") {
    using namespace teddy::ops;
    teddy::bdd_manager manager(2, 1000);
    auto var0 = manager.variable(0);
    auto var1 = manager.variable(1);
    auto and_result = manager.apply<AND>(var0, var1);

    std::vector<std::string> var_names = {"a", "b"};

    auto nodes = collect_teddy_nodes_topological(and_result, var_names);

    // Should have at least terminal nodes (0, 1) plus variable nodes
    REQUIRE(nodes.size() >= 2);
}

TEST_CASE("TeddyGraph - Node table text format", "[teddy_graph][analysis]") {
    using namespace teddy::ops;
    teddy::bdd_manager manager(2, 1000);
    auto var0 = manager.variable(0);
    auto var1 = manager.variable(1);
    auto and_result = manager.apply<AND>(var0, var1);

    std::vector<std::string> var_names = {"a", "b"};

    std::ostringstream out;
    write_teddy_nodes_to_stream(manager, and_result, var_names, out, true);

    std::string result = out.str();
    REQUIRE_THAT(result, ContainsSubstring("BDD Node Table"));
    REQUIRE_THAT(result, ContainsSubstring("Index"));
    REQUIRE_THAT(result, ContainsSubstring("Variable"));
}

TEST_CASE("TeddyGraph - Node table markdown format", "[teddy_graph][analysis]") {
    using namespace teddy::ops;
    teddy::bdd_manager manager(2, 1000);
    auto var0 = manager.variable(0);
    auto var1 = manager.variable(1);
    auto or_result = manager.apply<OR>(var0, var1);

    std::vector<std::string> var_names = {"x", "y"};

    std::ostringstream out;
    write_teddy_nodes_to_markdown(manager, or_result, var_names, out);

    std::string result = out.str();
    REQUIRE_THAT(result, ContainsSubstring("|"));  // Markdown table separator
    REQUIRE_THAT(result, ContainsSubstring("Index"));
}

TEST_CASE("TeddyGraph - BDD constants", "[teddy_graph][constants]") {
    teddy::bdd_manager manager(1, 1000);

    auto true_bdd = manager.constant(1);
    auto false_bdd = manager.constant(0);

    REQUIRE(true_bdd.unsafe_get_root() != nullptr);
    REQUIRE(false_bdd.unsafe_get_root() != nullptr);
    REQUIRE(true_bdd.unsafe_get_root() != false_bdd.unsafe_get_root());
}

TEST_CASE("TeddyGraph - BDD constant operations", "[teddy_graph][constants]") {
    using namespace teddy::ops;
    teddy::bdd_manager manager(1, 1000);
    auto var0 = manager.variable(0);
    auto true_bdd = manager.constant(1);
    auto false_bdd = manager.constant(0);

    SECTION("AND with true") {
        auto result = manager.apply<AND>(var0, true_bdd);
        REQUIRE(result.unsafe_get_root() != nullptr);
    }

    SECTION("AND with false") {
        auto result = manager.apply<AND>(var0, false_bdd);
        REQUIRE(result.unsafe_get_root() != nullptr);
    }

    SECTION("OR with true") {
        auto result = manager.apply<OR>(var0, true_bdd);
        REQUIRE(result.unsafe_get_root() != nullptr);
    }

    SECTION("OR with false") {
        auto result = manager.apply<OR>(var0, false_bdd);
        REQUIRE(result.unsafe_get_root() != nullptr);
    }
}
