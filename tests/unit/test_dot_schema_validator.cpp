// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file test_dot_schema_validator.cpp
 * @brief DOT schema validator used to validate generated .dot outputs
 *
 * This test provides a small but robust DOT syntax validator (parser) that
 * verifies typical Graphviz DOT output produced by the project's generators.
 * The validator checks for:
 * - valid header (digraph/graph)
 * - balanced braces
 * - statements ending with semicolons
 * - well-formed attribute lists ([k=v,...])
 * - matched quoted strings
 */

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <cctype>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "cudd_graph.hpp"
#include "dot_graph_generator.hpp"
#include "expression_graph.hpp"

using std::string;

struct DotValidator {
    string input;
    size_t pos = 0;
    std::vector<string> errors;

    explicit DotValidator(string s) : input(std::move(s)), pos(0) {}

    bool eof() const {
        return pos >= input.size();
    }
    char peek() const {
        return eof() ? '\0' : input[pos];
    }
    char get() {
        return eof() ? '\0' : input[pos++];
    }

    void skip_ws() {
        while (!eof() && std::isspace(static_cast<unsigned char>(peek())))
            pos++;
    }

    std::optional<string> parse_quoted() {
        if (peek() != '"')
            return std::nullopt;
        get();  // consume '"'
        string buf;
        while (!eof()) {
            char c = get();
            if (c == '\\') {
                if (eof()) {
                    errors.push_back("Trailing backslash in quoted string");
                    return std::nullopt;
                }
                char esc = get();
                buf.push_back(esc);
                continue;
            }
            if (c == '"')
                return buf;
            buf.push_back(c);
        }
        errors.push_back("Unterminated quoted string");
        return std::nullopt;
    }

    std::optional<string> parse_id() {
        skip_ws();
        if (peek() == '"') {
            auto q = parse_quoted();
            if (!q)
                return std::nullopt;
            return *q;
        }
        // Identifier: letters, digits, underscore, dot, colon
        if (!std::isalpha(static_cast<unsigned char>(peek())) && peek() != '_')
            return std::nullopt;
        string id;
        while (!eof()) {
            char c = peek();
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '.' || c == ':') {
                id.push_back(get());
            } else
                break;
        }
        return id;
    }

    bool expect_char(char expected) {
        skip_ws();
        if (peek() == expected) {
            get();
            return true;
        }
        errors.push_back(std::string("Expected character: ") + expected);
        return false;
    }

    bool parse_attr_list() {
        skip_ws();
        if (peek() != '[')
            return true;  // optional
        get();            // consume '['
        while (!eof()) {
            skip_ws();
            // key
            auto key = parse_id();
            if (!key) {
                errors.push_back("Invalid attribute key");
                return false;
            }
            skip_ws();
            if (!expect_char('='))
                return false;
            skip_ws();
            // value can be id or quoted
            if (peek() == '"') {
                auto v = parse_quoted();
                if (!v)
                    return false;
            } else {
                auto v2 = parse_id();
                if (!v2) {
                    errors.push_back("Invalid attribute value");
                    return false;
                }
            }
            skip_ws();
            if (peek() == ']') {
                get();
                return true;
            }
            if (peek() == ',' || peek() == ';') {
                get();
                continue;
            }
            // any other char is error
            errors.push_back("Invalid character in attribute list");
            return false;
        }
        errors.push_back("Unterminated attribute list");
        return false;
    }

    bool parse_statement() {
        skip_ws();
        if (eof())
            return false;
        // statements end with semicolon ;
        // we will parse until ';' is found, validating fragments
        // possible forms: id ;  id -> id [attrs] ; id [attrs] ;
        auto id = parse_id();
        if (!id) {
            errors.push_back("Invalid or missing identifier at statement start");
            return false;
        }
        skip_ws();
        if (peek() == '-') {
            // expect ->
            get();
            if (peek() != '>') {
                errors.push_back("Expected '>' after '-' in edge");
                return false;
            }
            get();
            auto rhs = parse_id();
            if (!rhs) {
                errors.push_back("Invalid rhs of edge");
                return false;
            }
            // optional attrs
            if (!parse_attr_list())
                return false;
        } else {
            // node or attribute list
            if (!parse_attr_list())
                return false;
        }
        skip_ws();
        if (peek() != ';') {
            errors.push_back("Missing terminating semicolon");
            return false;
        }
        get();
        return true;
    }

    bool validate() {
        skip_ws();
        // optional 'strict'
        auto start_id = parse_id();
        if (!start_id) {
            errors.push_back("Missing graph keyword");
            return false;
        }
        string s = *start_id;
        if (s == "strict") {
            auto next = parse_id();
            if (!next) {
                errors.push_back("Missing graph keyword after strict");
                return false;
            }
            s = *next;
        }
        if (s != "digraph" && s != "graph") {
            errors.push_back("Expected 'digraph' or 'graph'");
            return false;
        }
        // optional name
        skip_ws();
        if (peek() == '"' || std::isalpha(static_cast<unsigned char>(peek())) || peek() == '_') {
            auto name = parse_id();
            if (!name) {
                errors.push_back("Invalid graph name");
                return false;
            }
        }
        skip_ws();
        if (!expect_char('{'))
            return false;
        // parse statements until matching '}'
        while (!eof()) {
            skip_ws();
            if (peek() == '}') {
                get();
                return errors.empty();
            }
            // allow comments starting with // or /* */? we ignore for now
            if (!parse_statement())
                return false;
        }
        errors.push_back("Unterminated graph (missing '}')");
        return false;
    }
};

static void validate_dot_output(const std::string& dot) {
    DotValidator v(dot);
    bool ok = v.validate();
    if (!ok) {
        for (auto& e : v.errors) {
            FAIL("DOT validation error: " + e);
        }
    }
    REQUIRE(ok);
}

TEST_CASE("DOT Schema Validator - expression_graph output", "[dot_schema]") {
    my_expression expr = my_or{
        std::make_unique<my_expression>(my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                               std::make_unique<my_expression>(my_variable{"b"})}),
        std::make_unique<my_expression>(my_variable{"c"})};

    std::ostringstream out;
    write_expression_to_dot(expr, out, "ValidatorExprGraph");
    validate_dot_output(out.str());
}

TEST_CASE("DOT Schema Validator - dot_graph::generate_dot_graph output", "[dot_schema]") {
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"x"}),
                                std::make_unique<my_expression>(my_variable{"y"})};
    expression_iterator root(expr);
    std::ostringstream out;
    dot_graph::DotConfig cfg;
    cfg.graph_name = "ValidatorDotGraph";
    dot_graph::generate_dot_graph(root, out, cfg);
    validate_dot_output(out.str());
}

TEST_CASE("DOT Schema Validator - CUDD dot output", "[dot_schema]") {
    Cudd mgr(1);
    BDD var0 = mgr.bddVar(0);
    std::vector<std::string> var_names = {"v0"};
    std::ostringstream out;
    write_cudd_to_dot(mgr, var0, var_names, out, "ValidatorCuddGraph");
    validate_dot_output(out.str());
}
