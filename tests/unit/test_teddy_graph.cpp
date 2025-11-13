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

    // Validate proper DOT graph structure
    REQUIRE_THAT(result, ContainsSubstring("digraph TestGraph"));
    REQUIRE_THAT(result, ContainsSubstring("{"));
    REQUIRE_THAT(result, ContainsSubstring("}"));

    // Validate variable name appears in graph
    REQUIRE_THAT(result, ContainsSubstring("x"));

    // Validate terminal nodes (0 and 1) exist in BDD
    REQUIRE_THAT(result, ContainsSubstring("0"));
    REQUIRE_THAT(result, ContainsSubstring("1"));

    // Validate edges are present (-> indicates directed edge in DOT)
    REQUIRE_THAT(result, ContainsSubstring("->"));

    // Verify graph has proper opening and closing
    REQUIRE((result.find("digraph TestGraph") != std::string::npos));
    REQUIRE((result.find("{") < result.find("}")));
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

    // Validate proper DOT graph structure
    REQUIRE_THAT(result, ContainsSubstring("digraph ANDGraph"));
    REQUIRE_THAT(result, ContainsSubstring("{"));
    REQUIRE_THAT(result, ContainsSubstring("}"));

    // Validate both variable names appear
    REQUIRE_THAT(result, ContainsSubstring("a"));
    REQUIRE_THAT(result, ContainsSubstring("b"));

    // Validate terminal nodes exist
    REQUIRE_THAT(result, ContainsSubstring("0"));
    REQUIRE_THAT(result, ContainsSubstring("1"));

    // Validate directed edges exist (multiple edges in BDD)
    size_t edge_count = 0;
    size_t pos = 0;
    while ((pos = result.find("->", pos)) != std::string::npos) {
        edge_count++;
        pos += 2;
    }
    REQUIRE(edge_count >= 2);  // At least 2 edges for an AND operation

    // Verify proper structure
    REQUIRE((result.find("{") < result.find("}")));
}

TEST_CASE("TeddyGraph - Mermaid generation for single variable", "[teddy_graph][mermaid]") {
    teddy::bdd_manager manager(1, 1000);
    auto var0 = manager.variable(0);
    std::vector<std::string> var_names = {"x"};

    std::ostringstream out;
    write_teddy_to_mermaid(manager, var0, var_names, out, "TestGraph");

    std::string result = out.str();

    // Validate Mermaid flowchart structure
    REQUIRE_THAT(result, ContainsSubstring("flowchart TD"));

    // Validate variable name appears
    REQUIRE_THAT(result, ContainsSubstring("x"));

    // Validate terminal values (0 and 1) for BDD
    REQUIRE_THAT(result, ContainsSubstring("0"));
    REQUIRE_THAT(result, ContainsSubstring("1"));

    // Validate Mermaid edges (-->) exist
    REQUIRE_THAT(result, ContainsSubstring("-->"));

    // Validate node definitions with brackets (Mermaid syntax)
    REQUIRE((result.find("[") != std::string::npos || result.find("((") != std::string::npos));

    // Verify flowchart declaration comes first
    REQUIRE((result.find("flowchart TD") < result.find("-->")));
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

    // Validate Mermaid structure
    REQUIRE_THAT(result, ContainsSubstring("flowchart TD"));

    // Validate both variables appear
    REQUIRE_THAT(result, ContainsSubstring("p"));
    REQUIRE_THAT(result, ContainsSubstring("q"));

    // Validate edges exist (multiple for OR operation)
    size_t edge_count = 0;
    size_t pos = 0;
    while ((pos = result.find("-->", pos)) != std::string::npos) {
        edge_count++;
        pos += 3;
    }
    REQUIRE(edge_count >= 2);  // At least 2 edges for OR operation

    // Verify flowchart declaration is at the start
    // Flowchart declaration exists (may have YAML front matter before it)
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

    // Validate table structure with headers
    REQUIRE_THAT(result, ContainsSubstring("BDD Node Table"));
    REQUIRE_THAT(result, ContainsSubstring("Index"));
    REQUIRE_THAT(result, ContainsSubstring("Variable"));

    // Validate column headers appear
    REQUIRE_THAT(result, ContainsSubstring("False Child"));
    REQUIRE_THAT(result, ContainsSubstring("True Child"));

    // Validate separator line exists (typical in text tables)
    REQUIRE_THAT(result, ContainsSubstring("|"));

    // Validate variable names appear in table rows
    REQUIRE_THAT(result, ContainsSubstring("a"));
    REQUIRE_THAT(result, ContainsSubstring("b"));

    // Validate terminal nodes in table
    REQUIRE_THAT(result, ContainsSubstring("Terminal"));

    // Verify header and data are both present
    REQUIRE_THAT(result, ContainsSubstring("Index"));
    REQUIRE_THAT(result, ContainsSubstring("Variable"));
    REQUIRE_THAT(result, ContainsSubstring("a"));
    REQUIRE_THAT(result, ContainsSubstring("b"));
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

    // Validate markdown table structure
    REQUIRE_THAT(result, ContainsSubstring("|"));  // Markdown table separator
    REQUIRE_THAT(result, ContainsSubstring("Index"));
    REQUIRE_THAT(result, ContainsSubstring("Variable"));

    // Validate markdown table alignment row (---) exists
    size_t dash_count = 0;
    for (char c : result) {
        if (c == '-')
            dash_count++;
    }
    REQUIRE(dash_count >= 3);  // Markdown tables need separator dashes

    // Validate variables appear in markdown table
    REQUIRE_THAT(result, ContainsSubstring("x"));
    REQUIRE_THAT(result, ContainsSubstring("y"));

    // Verify header row comes before data
    REQUIRE((result.find("Index") < result.find("x")));

    // Count rows (each row should have | characters)
    size_t row_count = 0;
    size_t pos = 0;
    while ((pos = result.find("\n", pos)) != std::string::npos) {
        row_count++;
        pos++;
    }
    REQUIRE(row_count >= 3);  // Header + separator + at least 1 data row
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
