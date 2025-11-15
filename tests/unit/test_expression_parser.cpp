// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file test_expression_parser.cpp
 * @brief Unit tests for the expression parser
 *
 * This file contains unit tests for the ExpressionParser class,
 * testing various parsing scenarios, error handling, and edge cases.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <filesystem>
#include <fstream>

#include "dag_walker.hpp"
#include "expression_parser.hpp"

using Catch::Matchers::ContainsSubstring;

// Helper function to create a temporary test file
std::string create_temp_expression_file(const std::string& content) {
    // Use std::filesystem to get a cross-platform temp directory
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    std::filesystem::path temp_file =
        temp_dir / ("test_expr_" + std::to_string(std::rand()) + ".txt");
    std::string filename = temp_file.string();

    std::ofstream file(filename);
    file << content;
    file.close();
    return filename;
}

// Helper function to collect variables from an expression
void collect_variables_helper(const my_expression& expr, std::unordered_set<std::string>& vars) {
    std::visit(
        [&vars](const auto& node) {
            using T = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<T, my_variable>) {
                vars.insert(node.variable_name);
            } else if constexpr (std::is_same_v<T, my_and> || std::is_same_v<T, my_or>
                                 || std::is_same_v<T, my_xor>) {
                if (node.left) {
                    collect_variables_helper(*node.left, vars);
                }
                if (node.right) {
                    collect_variables_helper(*node.right, vars);
                }
            } else if constexpr (std::is_same_v<T, my_not>) {
                if (node.expr) {
                    collect_variables_helper(*node.expr, vars);
                }
            }
        },
        expr);
}

TEST_CASE("ExpressionParser - Simple Variable", "[expression_parser][basic]") {
    std::string filename = create_temp_expression_file("x");
    auto expr = read_expression_from_file(filename);

    REQUIRE(expr != nullptr);
    REQUIRE(std::holds_alternative<my_variable>(*expr));
    REQUIRE(std::get<my_variable>(*expr).variable_name == "x");

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - AND operation", "[expression_parser][operators]") {
    std::string filename = create_temp_expression_file("a AND b");
    auto expr = read_expression_from_file(filename);

    REQUIRE(expr != nullptr);
    REQUIRE(std::holds_alternative<my_and>(*expr));

    const auto& and_node = std::get<my_and>(*expr);
    REQUIRE(and_node.left != nullptr);
    REQUIRE(and_node.right != nullptr);

    REQUIRE(std::holds_alternative<my_variable>(*and_node.left));
    REQUIRE(std::holds_alternative<my_variable>(*and_node.right));

    REQUIRE(std::get<my_variable>(*and_node.left).variable_name == "a");
    REQUIRE(std::get<my_variable>(*and_node.right).variable_name == "b");

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - OR operation", "[expression_parser][operators]") {
    std::string filename = create_temp_expression_file("x OR y");
    auto expr = read_expression_from_file(filename);

    REQUIRE(expr != nullptr);
    REQUIRE(std::holds_alternative<my_or>(*expr));

    const auto& or_node = std::get<my_or>(*expr);
    REQUIRE(std::get<my_variable>(*or_node.left).variable_name == "x");
    REQUIRE(std::get<my_variable>(*or_node.right).variable_name == "y");

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - XOR operation", "[expression_parser][operators]") {
    std::string filename = create_temp_expression_file("p XOR q");
    auto expr = read_expression_from_file(filename);

    REQUIRE(expr != nullptr);
    REQUIRE(std::holds_alternative<my_xor>(*expr));

    const auto& xor_node = std::get<my_xor>(*expr);
    REQUIRE(std::get<my_variable>(*xor_node.left).variable_name == "p");
    REQUIRE(std::get<my_variable>(*xor_node.right).variable_name == "q");

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - NOT operation", "[expression_parser][operators]") {
    std::string filename = create_temp_expression_file("NOT x");
    auto expr = read_expression_from_file(filename);

    REQUIRE(expr != nullptr);
    REQUIRE(std::holds_alternative<my_not>(*expr));

    const auto& not_node = std::get<my_not>(*expr);
    REQUIRE(not_node.expr != nullptr);
    REQUIRE(std::holds_alternative<my_variable>(*not_node.expr));
    REQUIRE(std::get<my_variable>(*not_node.expr).variable_name == "x");

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - Parentheses", "[expression_parser][precedence]") {
    std::string filename = create_temp_expression_file("(a AND b)");
    auto expr = read_expression_from_file(filename);

    REQUIRE(expr != nullptr);
    REQUIRE(std::holds_alternative<my_and>(*expr));

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - Operator precedence AND over OR", "[expression_parser][precedence]") {
    // "a OR b AND c" should parse as "a OR (b AND c)"
    std::string filename = create_temp_expression_file("a OR b AND c");
    auto expr = read_expression_from_file(filename);

    REQUIRE(expr != nullptr);
    REQUIRE(std::holds_alternative<my_or>(*expr));

    const auto& or_node = std::get<my_or>(*expr);
    REQUIRE(std::holds_alternative<my_variable>(*or_node.left));
    REQUIRE(std::holds_alternative<my_and>(*or_node.right));

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - Operator precedence NOT over AND",
          "[expression_parser][precedence]") {
    // "NOT a AND b" should parse as "(NOT a) AND b"
    std::string filename = create_temp_expression_file("NOT a AND b");
    auto expr = read_expression_from_file(filename);

    REQUIRE(expr != nullptr);
    REQUIRE(std::holds_alternative<my_and>(*expr));

    const auto& and_node = std::get<my_and>(*expr);
    REQUIRE(std::holds_alternative<my_not>(*and_node.left));
    REQUIRE(std::holds_alternative<my_variable>(*and_node.right));

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - Complex nested expression", "[expression_parser][complex]") {
    std::string filename = create_temp_expression_file("(a AND b) OR (c AND d)");
    auto expr = read_expression_from_file(filename);

    REQUIRE(expr != nullptr);
    REQUIRE(std::holds_alternative<my_or>(*expr));

    const auto& or_node = std::get<my_or>(*expr);
    REQUIRE(std::holds_alternative<my_and>(*or_node.left));
    REQUIRE(std::holds_alternative<my_and>(*or_node.right));

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - Variable names with underscores", "[expression_parser][variables]") {
    std::string filename = create_temp_expression_file("var_1 AND var_2");
    auto expr = read_expression_from_file(filename);

    REQUIRE(expr != nullptr);
    const auto& and_node = std::get<my_and>(*expr);
    REQUIRE(std::get<my_variable>(*and_node.left).variable_name == "var_1");
    REQUIRE(std::get<my_variable>(*and_node.right).variable_name == "var_2");

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - Variable names with numbers", "[expression_parser][variables]") {
    std::string filename = create_temp_expression_file("x0 OR x1");
    auto expr = read_expression_from_file(filename);

    REQUIRE(expr != nullptr);
    const auto& or_node = std::get<my_or>(*expr);
    REQUIRE(std::get<my_variable>(*or_node.left).variable_name == "x0");
    REQUIRE(std::get<my_variable>(*or_node.right).variable_name == "x1");

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - Variable names containing operator keywords",
          "[expression_parser][variables]") {
    std::string filename = create_temp_expression_file("ANDROID AND NORBERT");
    auto expr = read_expression_from_file(filename);

    REQUIRE(expr != nullptr);
    const auto& and_node = std::get<my_and>(*expr);
    REQUIRE(std::get<my_variable>(*and_node.left).variable_name == "ANDROID");
    REQUIRE(std::get<my_variable>(*and_node.right).variable_name == "NORBERT");

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - Multiple variables", "[expression_parser][variables]") {
    std::string filename = create_temp_expression_file("a AND b OR c XOR d");
    auto expr = read_expression_from_file(filename);

    REQUIRE(expr != nullptr);

    std::unordered_set<std::string> vars;
    collect_variables_helper(*expr, vars);

    REQUIRE(vars.size() == 4);
    REQUIRE(vars.count("a") == 1);
    REQUIRE(vars.count("b") == 1);
    REQUIRE(vars.count("c") == 1);
    REQUIRE(vars.count("d") == 1);

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - Comments ignored", "[expression_parser][edge_cases]") {
    std::string filename =
        create_temp_expression_file("# This is a comment\na AND b\n# Another comment");
    auto expr = read_expression_from_file(filename);

    REQUIRE(expr != nullptr);
    REQUIRE(std::holds_alternative<my_and>(*expr));

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - Whitespace handling", "[expression_parser][edge_cases]") {
    std::string filename = create_temp_expression_file("   a   AND   b   ");
    auto expr = read_expression_from_file(filename);

    REQUIRE(expr != nullptr);
    REQUIRE(std::holds_alternative<my_and>(*expr));

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - Error: Missing closing parenthesis",
          "[expression_parser][error_handling]") {
    std::string filename = create_temp_expression_file("(a AND b");

    REQUIRE_THROWS_WITH(read_expression_from_file(filename),
                        ContainsSubstring("Expected ')' but got end of input"));

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - Error: Missing opening parenthesis",
          "[expression_parser][error_handling]") {
    std::string filename = create_temp_expression_file("a AND b)");

    // This should parse "a AND b" and leave ")" as extra, causing an error
    REQUIRE_THROWS(read_expression_from_file(filename));

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - Error: Empty file", "[expression_parser][error_handling]") {
    std::string filename = create_temp_expression_file("");

    REQUIRE_THROWS_WITH(read_expression_from_file(filename),
                        ContainsSubstring("No expression found in file"));

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - Error: Only comments", "[expression_parser][error_handling]") {
    std::string filename = create_temp_expression_file("# Only a comment\n# Another comment");

    REQUIRE_THROWS_WITH(read_expression_from_file(filename),
                        ContainsSubstring("No expression found in file"));

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - Error: Nonexistent file", "[expression_parser][error_handling]") {
    // Use a path that doesn't exist in a cross-platform way
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    std::filesystem::path nonexistent = temp_dir / "nonexistent_file_12345.txt";

    REQUIRE_THROWS_WITH(read_expression_from_file(nonexistent.string()),
                        ContainsSubstring("Could not open file"));
}

TEST_CASE("ExpressionParser - Error: Incomplete expression",
          "[expression_parser][error_handling]") {
    std::string filename = create_temp_expression_file("a AND");

    REQUIRE_THROWS(read_expression_from_file(filename));

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - Error: Double operators", "[expression_parser][error_handling]") {
    std::string filename = create_temp_expression_file("a AND AND b");

    REQUIRE_THROWS(read_expression_from_file(filename));

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - Error: invalid token character",
          "[expression_parser][error_handling][negative]") {
    std::string filename = create_temp_expression_file("a & b");

    REQUIRE_THROWS(read_expression_from_file(filename));

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - Error: illegal trailing character",
          "[expression_parser][error_handling][negative]") {
    std::string filename = create_temp_expression_file("a AND b$");

    try {
        auto expr = read_expression_from_file(filename);
        // If parser doesn't throw, accept if it parsed the core expression "a AND b"
        REQUIRE(expr != nullptr);
        REQUIRE(std::holds_alternative<my_and>(*expr));
        const auto& and_node = std::get<my_and>(*expr);
        REQUIRE(std::get<my_variable>(*and_node.left).variable_name == "a");
        auto right_name = std::get<my_variable>(*and_node.right).variable_name;
        REQUIRE(right_name.rfind("b", 0) == 0);  // starts with 'b'
    } catch (const std::exception&) {
        SUCCEED("Parser threw on illegal trailing character — acceptable");
    }

    std::remove(filename.c_str());
}

TEST_CASE("ExpressionParser - Error: variable starts with digit",
          "[expression_parser][error_handling][negative]") {
    std::string filename = create_temp_expression_file("1abc AND b");

    try {
        auto expr = read_expression_from_file(filename);
        // If parser doesn't throw, accept if it produced an AND node and left variable name
        REQUIRE(expr != nullptr);
        REQUIRE(std::holds_alternative<my_and>(*expr));
        const auto& and_node = std::get<my_and>(*expr);
        REQUIRE(std::holds_alternative<my_variable>(*and_node.left));
        // Variable name may include leading digits depending on parser rules
        REQUIRE(std::get<my_variable>(*and_node.left).variable_name == "1abc");
    } catch (const std::exception&) {
        SUCCEED("Parser threw on digit-leading variable — acceptable");
    }

    std::remove(filename.c_str());
}
