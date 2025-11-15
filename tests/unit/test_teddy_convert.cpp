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

// Local evaluator borrowed from test_teddy_graph to verify truth table
static bool evaluate_teddy_bdd(teddy::bdd_manager& manager, teddy::bdd_manager::diagram_t bdd,
                               const std::vector<bool>& assignment) {
    using namespace teddy::ops;

    teddy::bdd_manager::diagram_t cube = manager.constant(1);
    for (size_t i = 0; i < assignment.size(); ++i) {
        teddy::bdd_manager::diagram_t var = manager.variable(static_cast<int>(i));
        if (!assignment[i]) {
            teddy::bdd_manager::diagram_t one = manager.constant(1);
            var = manager.apply<XOR>(var, one);
        }
        cube = manager.apply<AND>(cube, var);
    }

    teddy::bdd_manager::diagram_t result = manager.apply<AND>(bdd, cube);
    return result.unsafe_get_root() == cube.unsafe_get_root();
}

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
