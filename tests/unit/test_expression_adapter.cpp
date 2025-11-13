// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file test_expression_adapter.cpp
 * @brief Unit tests for the expression adapter
 *
 * This file contains unit tests for the expression_adapter class,
 * testing the adapter interface that makes our expressions compatible with TeDDy.
 */

#include <catch2/catch_test_macros.hpp>
#include <unordered_map>

#include "expression_adapter.hpp"
#include "expression_types.hpp"

TEST_CASE("ExpressionAdapter - Variable node", "[expression_adapter][basic]") {
    std::unordered_map<std::string, int> var_map = {{"x", 0}};
    my_expression expr = my_variable{"x"};

    expression_adapter adapter(expr, var_map);

    REQUIRE(adapter.is_variable());
    REQUIRE_FALSE(adapter.is_constant());
    REQUIRE_FALSE(adapter.is_operation());
    REQUIRE(adapter.get_index() == 0);
}

TEST_CASE("ExpressionAdapter - Multiple variables", "[expression_adapter][basic]") {
    std::unordered_map<std::string, int> var_map = {{"a", 0}, {"b", 1}, {"c", 2}};

    SECTION("Variable a") {
        my_expression expr = my_variable{"a"};
        expression_adapter adapter(expr, var_map);
        REQUIRE(adapter.get_index() == 0);
    }

    SECTION("Variable b") {
        my_expression expr = my_variable{"b"};
        expression_adapter adapter(expr, var_map);
        REQUIRE(adapter.get_index() == 1);
    }

    SECTION("Variable c") {
        my_expression expr = my_variable{"c"};
        expression_adapter adapter(expr, var_map);
        REQUIRE(adapter.get_index() == 2);
    }
}

TEST_CASE("ExpressionAdapter - AND operation", "[expression_adapter][operations]") {
    std::unordered_map<std::string, int> var_map = {{"a", 0}, {"b", 1}};
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};

    expression_adapter adapter(expr, var_map);

    REQUIRE_FALSE(adapter.is_variable());
    REQUIRE_FALSE(adapter.is_constant());
    REQUIRE(adapter.is_operation());

    // Test the evaluate method for AND operation
    REQUIRE(adapter.evaluate(1, 1) == 1);  // true AND true = true
    REQUIRE(adapter.evaluate(1, 0) == 0);  // true AND false = false
    REQUIRE(adapter.evaluate(0, 1) == 0);  // false AND true = false
    REQUIRE(adapter.evaluate(0, 0) == 0);  // false AND false = false
}

TEST_CASE("ExpressionAdapter - OR operation", "[expression_adapter][operations]") {
    std::unordered_map<std::string, int> var_map = {{"x", 0}, {"y", 1}};
    my_expression expr = my_or{std::make_unique<my_expression>(my_variable{"x"}),
                               std::make_unique<my_expression>(my_variable{"y"})};

    expression_adapter adapter(expr, var_map);

    REQUIRE(adapter.is_operation());

    // Test the evaluate method for OR operation
    REQUIRE(adapter.evaluate(1, 1) == 1);  // true OR true = true
    REQUIRE(adapter.evaluate(1, 0) == 1);  // true OR false = true
    REQUIRE(adapter.evaluate(0, 1) == 1);  // false OR true = true
    REQUIRE(adapter.evaluate(0, 0) == 0);  // false OR false = false
}

TEST_CASE("ExpressionAdapter - XOR operation", "[expression_adapter][operations]") {
    std::unordered_map<std::string, int> var_map = {{"p", 0}, {"q", 1}};
    my_expression expr = my_xor{std::make_unique<my_expression>(my_variable{"p"}),
                                std::make_unique<my_expression>(my_variable{"q"})};

    expression_adapter adapter(expr, var_map);

    REQUIRE(adapter.is_operation());

    // Test the evaluate method for XOR operation
    REQUIRE(adapter.evaluate(1, 1) == 0);  // true XOR true = false
    REQUIRE(adapter.evaluate(1, 0) == 1);  // true XOR false = true
    REQUIRE(adapter.evaluate(0, 1) == 1);  // false XOR true = true
    REQUIRE(adapter.evaluate(0, 0) == 0);  // false XOR false = false
}

TEST_CASE("ExpressionAdapter - NOT operation", "[expression_adapter][operations]") {
    std::unordered_map<std::string, int> var_map = {{"z", 0}};
    my_expression expr = my_not{std::make_unique<my_expression>(my_variable{"z"})};

    expression_adapter adapter(expr, var_map);

    REQUIRE(adapter.is_operation());

    // Test the evaluate method for NOT operation (right value is ignored)
    REQUIRE(adapter.evaluate(1, 0) == 0);  // NOT true = false
    REQUIRE(adapter.evaluate(0, 0) == 1);  // NOT false = true
}

TEST_CASE("ExpressionAdapter - Left child access", "[expression_adapter][children]") {
    std::unordered_map<std::string, int> var_map = {{"a", 0}, {"b", 1}};
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};

    expression_adapter adapter(expr, var_map);
    const auto& left_child = adapter.get_left();

    REQUIRE(left_child.is_variable());
    REQUIRE(left_child.get_index() == 0);
}

TEST_CASE("ExpressionAdapter - Right child access", "[expression_adapter][children]") {
    std::unordered_map<std::string, int> var_map = {{"a", 0}, {"b", 1}};
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};

    expression_adapter adapter(expr, var_map);
    const auto& right_child = adapter.get_right();

    REQUIRE(right_child.is_variable());
    REQUIRE(right_child.get_index() == 1);
}

TEST_CASE("ExpressionAdapter - NOT operation child access", "[expression_adapter][children]") {
    std::unordered_map<std::string, int> var_map = {{"x", 0}};
    my_expression expr = my_not{std::make_unique<my_expression>(my_variable{"x"})};

    expression_adapter adapter(expr, var_map);
    // For NOT, get_left() should work to access the child
    const auto& child = adapter.get_left();

    REQUIRE(child.is_variable());
    REQUIRE(child.get_index() == 0);
}

TEST_CASE("ExpressionAdapter - Copy constructor", "[expression_adapter][copy]") {
    std::unordered_map<std::string, int> var_map = {{"a", 0}, {"b", 1}};
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};

    expression_adapter adapter1(expr, var_map);
    expression_adapter adapter2(adapter1);

    REQUIRE(adapter2.is_operation());
    REQUIRE(adapter2.evaluate(1, 1) == 1);  // Verify it works correctly
}

TEST_CASE("ExpressionAdapter - Error: Variable not in map", "[expression_adapter][error]") {
    std::unordered_map<std::string, int> var_map = {{"a", 0}};
    my_expression expr = my_variable{"nonexistent"};

    expression_adapter adapter(expr, var_map);

    REQUIRE_THROWS(adapter.get_index());
}

TEST_CASE("ExpressionAdapter - Error: get_index on non-variable", "[expression_adapter][error]") {
    std::unordered_map<std::string, int> var_map = {{"a", 0}, {"b", 1}};
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};

    expression_adapter adapter(expr, var_map);

    REQUIRE_THROWS(adapter.get_index());
}

TEST_CASE("ExpressionAdapter - Nested expression", "[expression_adapter][complex]") {
    std::unordered_map<std::string, int> var_map = {{"a", 0}, {"b", 1}, {"c", 2}};

    // (a AND b) OR c
    my_expression expr = my_or{
        std::make_unique<my_expression>(my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                               std::make_unique<my_expression>(my_variable{"b"})}),
        std::make_unique<my_expression>(my_variable{"c"})};

    expression_adapter adapter(expr, var_map);

    REQUIRE(adapter.is_operation());

    const auto& left_child = adapter.get_left();
    REQUIRE(left_child.is_operation());

    const auto& right_child = adapter.get_right();
    REQUIRE(right_child.is_variable());
    REQUIRE(right_child.get_index() == 2);
}
