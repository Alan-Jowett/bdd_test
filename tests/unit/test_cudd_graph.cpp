// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file test_cudd_graph.cpp
 * @brief Unit tests for the CUDD BDD graph implementation
 *
 * This file contains unit tests for the CuddGraph class,
 * testing BDD construction, optimization, and analysis functionality.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <cudd/cuddObj.hh>
#include <sstream>

#include "cudd_graph.hpp"

using Catch::Matchers::ContainsSubstring;

TEST_CASE("CuddGraph - Basic CUDD manager creation", "[cudd_graph][basic]") {
    Cudd manager(2);  // 2 variables
    REQUIRE(manager.ReadSize() == 2);
}

TEST_CASE("CuddGraph - Create single variable BDD", "[cudd_graph][basic]") {
    Cudd manager(1);
    BDD var0 = manager.bddVar(0);

    REQUIRE(!var0.IsZero());
    REQUIRE(!var0.IsOne());
}

TEST_CASE("CuddGraph - AND operation", "[cudd_graph][operations]") {
    Cudd manager(2);
    BDD var0 = manager.bddVar(0);
    BDD var1 = manager.bddVar(1);

    BDD result = var0 * var1;  // AND operation

    REQUIRE(!result.IsZero());
}

TEST_CASE("CuddGraph - OR operation", "[cudd_graph][operations]") {
    Cudd manager(2);
    BDD var0 = manager.bddVar(0);
    BDD var1 = manager.bddVar(1);

    BDD result = var0 + var1;  // OR operation

    REQUIRE(!result.IsZero());
}

TEST_CASE("CuddGraph - XOR operation", "[cudd_graph][operations]") {
    Cudd manager(2);
    BDD var0 = manager.bddVar(0);
    BDD var1 = manager.bddVar(1);

    BDD result = var0.Xor(var1);  // XOR operation

    REQUIRE(!result.IsZero());
}

TEST_CASE("CuddGraph - NOT operation", "[cudd_graph][operations]") {
    Cudd manager(1);
    BDD var0 = manager.bddVar(0);

    BDD result = ~var0;  // NOT operation

    REQUIRE(!result.IsZero());
}

TEST_CASE("CuddGraph - Complex nested operations", "[cudd_graph][operations]") {
    Cudd manager(3);
    BDD var0 = manager.bddVar(0);
    BDD var1 = manager.bddVar(1);
    BDD var2 = manager.bddVar(2);

    // (var0 AND var1) OR var2
    BDD and_result = var0 * var1;
    BDD final_result = and_result + var2;

    REQUIRE(!final_result.IsZero());
}

TEST_CASE("CuddGraph - BDD constants", "[cudd_graph][constants]") {
    Cudd manager(1);

    BDD true_bdd = manager.bddOne();
    BDD false_bdd = manager.bddZero();

    REQUIRE(true_bdd.IsOne());
    REQUIRE(false_bdd.IsZero());
    REQUIRE(true_bdd != false_bdd);
}

TEST_CASE("CuddGraph - BDD constant operations", "[cudd_graph][constants]") {
    Cudd manager(1);
    BDD var0 = manager.bddVar(0);
    BDD true_bdd = manager.bddOne();
    BDD false_bdd = manager.bddZero();

    SECTION("AND with true") {
        BDD result = var0 * true_bdd;
        REQUIRE(result == var0);
    }

    SECTION("AND with false") {
        BDD result = var0 * false_bdd;
        REQUIRE(result.IsZero());
    }

    SECTION("OR with true") {
        BDD result = var0 + true_bdd;
        REQUIRE(result.IsOne());
    }

    SECTION("OR with false") {
        BDD result = var0 + false_bdd;
        REQUIRE(result == var0);
    }
}

TEST_CASE("CuddGraph - DOT generation for single variable", "[cudd_graph][dot]") {
    Cudd manager(1);
    BDD var0 = manager.bddVar(0);
    std::vector<std::string> var_names = {"x"};

    std::ostringstream out;
    write_cudd_to_dot(manager, var0, var_names, out, "TestGraph");

    std::string result = out.str();
    REQUIRE_THAT(result, ContainsSubstring("digraph TestGraph"));
    REQUIRE_THAT(result, ContainsSubstring("x"));
}

TEST_CASE("CuddGraph - DOT generation for AND operation", "[cudd_graph][dot]") {
    Cudd manager(2);
    BDD var0 = manager.bddVar(0);
    BDD var1 = manager.bddVar(1);
    BDD and_result = var0 * var1;

    std::vector<std::string> var_names = {"a", "b"};

    std::ostringstream out;
    write_cudd_to_dot(manager, and_result, var_names, out, "ANDGraph");

    std::string result = out.str();
    REQUIRE_THAT(result, ContainsSubstring("digraph ANDGraph"));
    REQUIRE_THAT(result, ContainsSubstring("a"));
    REQUIRE_THAT(result, ContainsSubstring("b"));
}

TEST_CASE("CuddGraph - Mermaid generation for single variable", "[cudd_graph][mermaid]") {
    Cudd manager(1);
    BDD var0 = manager.bddVar(0);
    std::vector<std::string> var_names = {"x"};

    std::ostringstream out;
    write_cudd_to_mermaid(manager, var0, var_names, out, "TestGraph");

    std::string result = out.str();
    REQUIRE_THAT(result, ContainsSubstring("flowchart TD"));
    REQUIRE_THAT(result, ContainsSubstring("x"));
}

TEST_CASE("CuddGraph - Mermaid generation for OR operation", "[cudd_graph][mermaid]") {
    Cudd manager(2);
    BDD var0 = manager.bddVar(0);
    BDD var1 = manager.bddVar(1);
    BDD or_result = var0 + var1;

    std::vector<std::string> var_names = {"p", "q"};

    std::ostringstream out;
    write_cudd_to_mermaid(manager, or_result, var_names, out, "ORGraph");

    std::string result = out.str();
    REQUIRE_THAT(result, ContainsSubstring("flowchart TD"));
}

TEST_CASE("CuddGraph - Node collection topological order", "[cudd_graph][analysis]") {
    Cudd manager(2);
    BDD var0 = manager.bddVar(0);
    BDD var1 = manager.bddVar(1);
    BDD and_result = var0 * var1;

    std::vector<std::string> var_names = {"a", "b"};

    auto nodes = collect_cudd_nodes_topological(manager, and_result, var_names);

    // Should have at least terminal nodes plus variable nodes
    REQUIRE(nodes.size() >= 2);
}

TEST_CASE("CuddGraph - Node table text format", "[cudd_graph][analysis]") {
    Cudd manager(2);
    BDD var0 = manager.bddVar(0);
    BDD var1 = manager.bddVar(1);
    BDD and_result = var0 * var1;

    std::vector<std::string> var_names = {"a", "b"};

    std::ostringstream out;
    write_cudd_nodes_to_stream(manager, and_result, var_names, out, true);

    std::string result = out.str();
    REQUIRE_THAT(result, ContainsSubstring("BDD Node Table"));
    REQUIRE_THAT(result, ContainsSubstring("Index"));
    REQUIRE_THAT(result, ContainsSubstring("Variable"));
}

TEST_CASE("CuddGraph - Node table markdown format", "[cudd_graph][analysis]") {
    Cudd manager(2);
    BDD var0 = manager.bddVar(0);
    BDD var1 = manager.bddVar(1);
    BDD or_result = var0 + var1;

    std::vector<std::string> var_names = {"x", "y"};

    std::ostringstream out;
    write_cudd_nodes_to_markdown(manager, or_result, var_names, out);

    std::string result = out.str();
    REQUIRE_THAT(result, ContainsSubstring("|"));  // Markdown table separator
    REQUIRE_THAT(result, ContainsSubstring("Index"));
}

TEST_CASE("CuddGraph - Node count", "[cudd_graph][analysis]") {
    Cudd manager(2);
    BDD var0 = manager.bddVar(0);
    BDD var1 = manager.bddVar(1);

    SECTION("Single variable") {
        int count = var0.nodeCount();
        REQUIRE(count >= 1);  // At least the variable node
    }

    SECTION("AND operation") {
        BDD and_result = var0 * var1;
        int count = and_result.nodeCount();
        REQUIRE(count >= 2);  // At least two variable nodes
    }
}
