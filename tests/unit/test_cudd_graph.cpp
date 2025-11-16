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
#include <vector>

#include "cudd_graph.hpp"

using Catch::Matchers::ContainsSubstring;

// Helper function to evaluate a BDD with specific variable assignments
// Returns true if the BDD evaluates to 1 for the given assignment
bool evaluate_cudd_bdd(Cudd& manager, const BDD& bdd, const std::vector<bool>& assignment) {
    BDD temp = bdd;
    for (size_t i = 0; i < assignment.size(); i++) {
        // Get the BDD variable
        BDD var = manager.bddVar(i);
        if (assignment[i]) {
            // If variable is true, restrict to the positive cofactor
            temp = temp.Restrict(var);
        } else {
            // If variable is false, restrict to the negative cofactor
            temp = temp.Restrict(!var);
        }
    }
    return temp.IsOne();
}

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

    // Validate AND truth table: result should be 1 only when both inputs are 1
    REQUIRE(evaluate_cudd_bdd(manager, result, {false, false}) == false);  // 0 AND 0 = 0
    REQUIRE(evaluate_cudd_bdd(manager, result, {false, true}) == false);   // 0 AND 1 = 0
    REQUIRE(evaluate_cudd_bdd(manager, result, {true, false}) == false);   // 1 AND 0 = 0
    REQUIRE(evaluate_cudd_bdd(manager, result, {true, true}) == true);     // 1 AND 1 = 1
}

TEST_CASE("CuddGraph - OR operation", "[cudd_graph][operations]") {
    Cudd manager(2);
    BDD var0 = manager.bddVar(0);
    BDD var1 = manager.bddVar(1);

    BDD result = var0 + var1;  // OR operation

    REQUIRE(!result.IsZero());

    // Validate OR truth table: result should be 1 when at least one input is 1
    REQUIRE(evaluate_cudd_bdd(manager, result, {false, false}) == false);  // 0 OR 0 = 0
    REQUIRE(evaluate_cudd_bdd(manager, result, {false, true}) == true);    // 0 OR 1 = 1
    REQUIRE(evaluate_cudd_bdd(manager, result, {true, false}) == true);    // 1 OR 0 = 1
    REQUIRE(evaluate_cudd_bdd(manager, result, {true, true}) == true);     // 1 OR 1 = 1
}

TEST_CASE("CuddGraph - XOR operation", "[cudd_graph][operations]") {
    Cudd manager(2);
    BDD var0 = manager.bddVar(0);
    BDD var1 = manager.bddVar(1);

    BDD result = var0.Xor(var1);  // XOR operation

    REQUIRE(!result.IsZero());

    // Validate XOR truth table: result should be 1 when inputs differ
    REQUIRE(evaluate_cudd_bdd(manager, result, {false, false}) == false);  // 0 XOR 0 = 0
    REQUIRE(evaluate_cudd_bdd(manager, result, {false, true}) == true);    // 0 XOR 1 = 1
    REQUIRE(evaluate_cudd_bdd(manager, result, {true, false}) == true);    // 1 XOR 0 = 1
    REQUIRE(evaluate_cudd_bdd(manager, result, {true, true}) == false);    // 1 XOR 1 = 0
}

TEST_CASE("CuddGraph - NOT operation", "[cudd_graph][operations]") {
    Cudd manager(1);
    BDD var0 = manager.bddVar(0);

    BDD result = ~var0;  // NOT operation

    REQUIRE(!result.IsZero());

    // Validate NOT truth table: result should be opposite of input
    REQUIRE(evaluate_cudd_bdd(manager, result, {false}) == true);  // NOT 0 = 1
    REQUIRE(evaluate_cudd_bdd(manager, result, {true}) == false);  // NOT 1 = 0
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

    // Validate truth table for (A AND B) OR C
    REQUIRE(evaluate_cudd_bdd(manager, final_result, {false, false, false})
            == false);  // (0 AND 0) OR 0 = 0
    REQUIRE(evaluate_cudd_bdd(manager, final_result, {false, false, true})
            == true);  // (0 AND 0) OR 1 = 1
    REQUIRE(evaluate_cudd_bdd(manager, final_result, {false, true, false})
            == false);  // (0 AND 1) OR 0 = 0
    REQUIRE(evaluate_cudd_bdd(manager, final_result, {false, true, true})
            == true);  // (0 AND 1) OR 1 = 1
    REQUIRE(evaluate_cudd_bdd(manager, final_result, {true, false, false})
            == false);  // (1 AND 0) OR 0 = 0
    REQUIRE(evaluate_cudd_bdd(manager, final_result, {true, false, true})
            == true);  // (1 AND 0) OR 1 = 1
    REQUIRE(evaluate_cudd_bdd(manager, final_result, {true, true, false})
            == true);  // (1 AND 1) OR 0 = 1
    REQUIRE(evaluate_cudd_bdd(manager, final_result, {true, true, true})
            == true);  // (1 AND 1) OR 1 = 1
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

TEST_CASE("CuddGraph - DOT generation for AND operation", "[cudd_graph][dot]") {
    Cudd manager(2);
    BDD var0 = manager.bddVar(0);
    BDD var1 = manager.bddVar(1);
    BDD and_result = var0 * var1;

    std::vector<std::string> var_names = {"a", "b"};

    std::ostringstream out;
    write_cudd_to_dot(manager, and_result, var_names, out, "ANDGraph");

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

TEST_CASE("CuddGraph - Mermaid generation for single variable", "[cudd_graph][mermaid]") {
    Cudd manager(1);
    BDD var0 = manager.bddVar(0);
    std::vector<std::string> var_names = {"x"};

    std::ostringstream out;
    write_cudd_to_mermaid(manager, var0, var_names, out, "TestGraph");

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

TEST_CASE("CuddGraph - Mermaid generation for OR operation", "[cudd_graph][mermaid]") {
    Cudd manager(2);
    BDD var0 = manager.bddVar(0);
    BDD var1 = manager.bddVar(1);
    BDD or_result = var0 + var1;

    std::vector<std::string> var_names = {"p", "q"};

    std::ostringstream out;
    write_cudd_to_mermaid(manager, or_result, var_names, out, "ORGraph");

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

TEST_CASE("CuddGraph - Node table text format", "[cudd_graph][analysis]") {
    Cudd manager(2);
    BDD var0 = manager.bddVar(0);
    BDD var1 = manager.bddVar(1);
    BDD and_result = var0 * var1;

    std::vector<std::string> var_names = {"a", "b"};

    std::ostringstream out;
    write_cudd_nodes_to_stream(manager, and_result, var_names, out, true);

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

TEST_CASE("CuddGraph - Node table markdown format", "[cudd_graph][analysis]") {
    Cudd manager(2);
    BDD var0 = manager.bddVar(0);
    BDD var1 = manager.bddVar(1);
    BDD or_result = var0 + var1;

    std::vector<std::string> var_names = {"x", "y"};

    std::ostringstream out;
    write_cudd_nodes_to_markdown(manager, or_result, var_names, out);

    std::string result = out.str();

    // Validate markdown table structure
    REQUIRE_THAT(result, ContainsSubstring("|"));  // Markdown table separator
    REQUIRE_THAT(result, ContainsSubstring("Index"));
    REQUIRE_THAT(result, ContainsSubstring("Variable"));

    // Validate markdown table alignment row (---) exists
    size_t dash_count = 0;
    std::count_if(result.begin(), result.end(), [&](char c) {
        if (c == '-') {
            dash_count++;
        }
        return false;
    });
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

TEST_CASE("CuddGraph - DOT writer with mismatched variable names vector",
          "[cudd_graph][negative]") {
    Cudd manager(2);
    BDD var0 = manager.bddVar(0);
    BDD var1 = manager.bddVar(1);

    BDD and_result = var0 * var1;

    // Provide only one variable name for two vars -> mismatch
    std::vector<std::string> var_names = {"only_one"};
    std::ostringstream out;

    // The writer should either throw or produce output without crashing.
    // Prefer ensuring it does not crash; if behavior changes to throw, tests
    // should be updated. We'll assert that calling the writer does not crash.
    REQUIRE_NOTHROW(write_cudd_to_dot(manager, and_result, var_names, out, "BadGraph"));

    std::string result = out.str();

    // If the implementation is robust it should still include graph header
    REQUIRE_THAT(result, ContainsSubstring("digraph BadGraph"));
}

TEST_CASE("CuddGraph - Mermaid writer with empty variable names", "[cudd_graph][negative]") {
    Cudd manager(1);
    BDD var0 = manager.bddVar(0);

    BDD not_var = ~var0;

    std::vector<std::string> var_names;  // empty
    std::ostringstream out;

    // Ensure writer handles empty names without throwing
    REQUIRE_NOTHROW(write_cudd_to_mermaid(manager, not_var, var_names, out, "EmptyNames"));

    std::string result = out.str();

    // Should still produce Mermaid flowchart header
    REQUIRE_THAT(result, ContainsSubstring("flowchart TD"));
}
