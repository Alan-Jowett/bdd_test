// SPDX-License-Identifier: MIT
/**
 * @file tests/unit/test_teddy_convert.cpp
 * @brief Tests for converting expression ASTs to TeDDy BDDs
 *
 * Ensures conversion functions produce non-empty BDDs and validates basic
 * truth table semantics for converted diagrams.
 */

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

#include "teddy_convert.hpp"
#include "teddy_test_utils.hpp"

// Helpers that create unique_ptr<my_expression> to avoid copying non-copyable variants
static std::unique_ptr<my_expression> var_ptr(const std::string& name) {
    return std::make_unique<my_expression>(my_variable{name});
}

static std::unique_ptr<my_expression> and_ptr(std::unique_ptr<my_expression> left,
                                              std::unique_ptr<my_expression> right) {
    my_and node;
    node.left = std::move(left);
    node.right = std::move(right);
    return std::make_unique<my_expression>(std::move(node));
}

static std::unique_ptr<my_expression> or_ptr(std::unique_ptr<my_expression> left,
                                             std::unique_ptr<my_expression> right) {
    my_or node;
    node.left = std::move(left);
    node.right = std::move(right);
    return std::make_unique<my_expression>(std::move(node));
}

static std::unique_ptr<my_expression> not_ptr(std::unique_ptr<my_expression> expr) {
    my_not node;
    node.expr = std::move(expr);
    return std::make_unique<my_expression>(std::move(node));
}

// evaluate_teddy_bdd is provided by teddy_test_utils.hpp

TEST_CASE("teddy_convert: basic conversions produce non-empty BDDs", "[teddy_convert]") {
    teddy::bdd_manager mgr(3, 1000);

    // Expression: (a AND b) OR c
    auto root_ptr = or_ptr(and_ptr(var_ptr("a"), var_ptr("b")), var_ptr("c"));

    auto bdd = convert_to_bdd(*root_ptr, mgr);
    REQUIRE(bdd.unsafe_get_root() != nullptr);

    auto bdd2 = convert_to_bdd_with_teddy_adapter(*root_ptr, mgr);
    REQUIRE(bdd2.unsafe_get_root() != nullptr);

    // Verify truth table for (a AND b) OR c
    REQUIRE(evaluate_teddy_bdd(mgr, bdd, {false, false, false}) == false);
    REQUIRE(evaluate_teddy_bdd(mgr, bdd, {true, true, false}) == true);
    REQUIRE(evaluate_teddy_bdd(mgr, bdd, {false, false, true}) == true);

    // Same checks for adapter-produced BDD
    REQUIRE(evaluate_teddy_bdd(mgr, bdd2, {false, false, false}) == false);
    REQUIRE(evaluate_teddy_bdd(mgr, bdd2, {true, true, false}) == true);
    REQUIRE(evaluate_teddy_bdd(mgr, bdd2, {false, false, true}) == true);
}

TEST_CASE("teddy_convert - negative: empty expression and unknown variable",
          "[teddy_convert][negative]") {
    teddy::bdd_manager mgr(2, 1000);

    // Empty expression: uninitialized variant - simulate by creating a variable with empty name
    auto empty_var = var_ptr("");

    // Behavior: conversion may throw or produce a valid BDD; accept either but ensure no crash
    REQUIRE_NOTHROW([&]() {
        auto b = convert_to_bdd(*empty_var, mgr);
        (void)b;
    }());

    // Unknown variable name: create variable name that is unlikely to be mapped to an index
    auto unknown_var = var_ptr("__nonexistent_var__");
    // The adapter-based conversion may throw if it expects variable mapping; accept either outcome
    REQUIRE_NOTHROW([&]() {
        auto b2 = convert_to_bdd_with_teddy_adapter(*unknown_var, mgr);
        REQUIRE(b2.unsafe_get_root() != nullptr);
    }());
}
