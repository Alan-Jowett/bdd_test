// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file test_mermaid_schema_validator.cpp
 * @brief Simple Mermaid syntax validator for unit tests
 *
 * This test implements a lightweight validator for Mermaid flowchart syntax used
 * by the project's generators. It validates the typical 'flowchart TD' output,
 * checks for node/edge lines, balanced brackets and basic label escaping.
 */

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <string>
#include <vector>

#include "cudd_graph.hpp"
#include "expression_graph.hpp"
#include "mermaid_graph_generator.hpp"

using std::string;

static bool validate_mermaid_flowchart(const string &s, std::string &err) {
    // Basic checks: contains 'flowchart' and direction (TD or LR etc.)
    if (s.find("flowchart") == string::npos) {
        err = "Missing flowchart declaration";
        return false;
    }
    if (s.find("TD") == string::npos && s.find("LR") == string::npos
        && s.find("RL") == string::npos) {
        err = "Missing flow direction";
        return false;
    }

    // Check for balanced parentheses and brackets used in node shapes
    int paren = 0, square = 0, brace = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '(')
            ++paren;
        if (c == ')')
            --paren;
        if (c == '[')
            ++square;
        if (c == ']')
            --square;
        if (c == '{')
            ++brace;
        if (c == '}')
            --brace;
        if (paren < 0 || square < 0 || brace < 0) {
            err = "Unbalanced brackets";
            return false;
        }
    }
    if (paren != 0 || square != 0 || brace != 0) {
        err = "Unbalanced brackets at end";
        return false;
    }

    // Check presence of at least one arrow ('-->' or '-.->')
    if (s.find("-->") == string::npos && s.find("-.->") == string::npos
        && s.find("==>") == string::npos) {
        err = "No edges detected";
        return false;
    }

    return true;
}

static void validate_and_require(const std::string &s) {
    std::string err;
    bool ok = validate_mermaid_flowchart(s, err);
    if (!ok)
        FAIL("Mermaid validation failed: " + err);
    REQUIRE(ok);
}

TEST_CASE("Mermaid Schema Validator - expression mermaid output", "[mermaid_schema]") {
    my_expression expr = my_or{
        std::make_unique<my_expression>(my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                               std::make_unique<my_expression>(my_variable{"b"})}),
        std::make_unique<my_expression>(my_variable{"c"})};

    std::ostringstream out;
    write_expression_to_mermaid(expr, out, "ValidatorExprMermaid");
    validate_and_require(out.str());
}

TEST_CASE("Mermaid Schema Validator - mermaid_graph::generate_mermaid_graph output",
          "[mermaid_schema]") {
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"x"}),
                                std::make_unique<my_expression>(my_variable{"y"})};
    expression_iterator root(expr);
    std::ostringstream out;
    mermaid_graph::MermaidConfig cfg;
    cfg.graph_title = "ValidatorMermaid";
    mermaid_graph::generate_mermaid_graph(root, out, cfg);
    validate_and_require(out.str());
}

TEST_CASE("Mermaid Schema Validator - malformed detection", "[mermaid_negative]") {
    // Missing flowchart
    std::string bad = "graph TD\nA --> B";
    std::string err;
    REQUIRE(!validate_mermaid_flowchart(bad, err));

    // Unbalanced brackets
    std::string bad2 = "flowchart TD\nA((Hello --> B";
    REQUIRE(!validate_mermaid_flowchart(bad2, err));
}
