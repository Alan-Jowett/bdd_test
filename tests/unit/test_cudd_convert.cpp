// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file test_cudd_convert.cpp
 * @brief Unit tests for `convert_to_cudd_bdd` in `cudd_convert.hpp`.
 */

#include <catch2/catch_test_macros.hpp>
#include <cudd/cuddObj.hh>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "cudd_convert.hpp"

// Local helper: evaluate a BDD with a concrete boolean assignment
static bool evaluate_cudd_bdd(Cudd& manager, const BDD& bdd, const std::vector<bool>& assignment) {
    BDD temp = bdd;
    for (size_t i = 0; i < assignment.size(); ++i) {
        BDD var = manager.bddVar(i);
        if (assignment[i])
            temp = temp.Restrict(var);
        else
            temp = temp.Restrict(!var);
    }
    return temp.IsOne();
}

TEST_CASE("CuddConvert - convert simple AND expression", "[cudd_convert]") {
    // Expression: a AND b
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};

    std::unordered_set<std::string> var_names = {"a", "b"};

    auto pair_mgr_bdd = convert_to_cudd_bdd(expr, var_names);

    REQUIRE(pair_mgr_bdd.first != nullptr);
    Cudd& mgr = *pair_mgr_bdd.first;
    BDD result = pair_mgr_bdd.second;

    // Manager should have allocated variables for 'a' and 'b'
    REQUIRE(mgr.ReadSize() >= 2);

    // BDD for AND should not be constant zero
    REQUIRE(!result.IsZero());

    // Validate AND truth table
    REQUIRE(evaluate_cudd_bdd(mgr, result, {false, false}) == false);
    REQUIRE(evaluate_cudd_bdd(mgr, result, {false, true}) == false);
    REQUIRE(evaluate_cudd_bdd(mgr, result, {true, false}) == false);
    REQUIRE(evaluate_cudd_bdd(mgr, result, {true, true}) == true);
}

TEST_CASE("CuddConvert - single variable and NOT", "[cudd_convert]") {
    // Expression: NOT a
    my_expression not_a = my_not{std::make_unique<my_expression>(my_variable{"a"})};

    std::unordered_set<std::string> var_names = {"a"};
    auto p = convert_to_cudd_bdd(not_a, var_names);
    REQUIRE(p.first != nullptr);
    Cudd& mgr = *p.first;
    BDD b = p.second;

    REQUIRE(mgr.ReadSize() >= 1);
    REQUIRE(!b.IsZero());
    REQUIRE(evaluate_cudd_bdd(mgr, b, {false}) == true);
    REQUIRE(evaluate_cudd_bdd(mgr, b, {true}) == false);
}

TEST_CASE("CuddConvert - XOR chain", "[cudd_convert]") {
    // Expression: a XOR b XOR c (parity of three)
    my_expression expr = my_xor{
        std::make_unique<my_expression>(my_variable{"a"}),
        std::make_unique<my_expression>(my_xor{std::make_unique<my_expression>(my_variable{"b"}),
                                               std::make_unique<my_expression>(my_variable{"c"})})};

    std::unordered_set<std::string> var_names = {"a", "b", "c"};
    auto p = convert_to_cudd_bdd(expr, var_names);
    REQUIRE(p.first != nullptr);
    Cudd& mgr = *p.first;
    BDD b = p.second;

    // Check parity truth table (only odd number of true -> true)
    REQUIRE(evaluate_cudd_bdd(mgr, b, {false, false, false}) == false);
    REQUIRE(evaluate_cudd_bdd(mgr, b, {true, false, false}) == true);
    REQUIRE(evaluate_cudd_bdd(mgr, b, {true, true, false}) == false);
    REQUIRE(evaluate_cudd_bdd(mgr, b, {true, true, true}) == true);
}

TEST_CASE("CuddConvert - nested OR and NOT", "[cudd_convert]") {
    // Expression: NOT (a OR (NOT b)) == (NOT a) AND b
    my_expression expr = my_not{std::make_unique<my_expression>(
        my_or{std::make_unique<my_expression>(my_variable{"a"}),
              std::make_unique<my_expression>(
                  my_not{std::make_unique<my_expression>(my_variable{"b"})})})};

    std::unordered_set<std::string> var_names = {"a", "b"};
    auto p = convert_to_cudd_bdd(expr, var_names);
    REQUIRE(p.first != nullptr);
    Cudd& mgr = *p.first;
    BDD b = p.second;

    // Equivalent to (NOT a) AND b
    REQUIRE(evaluate_cudd_bdd(mgr, b, {false, false}) == false);
    REQUIRE(evaluate_cudd_bdd(mgr, b, {false, true}) == true);
    REQUIRE(evaluate_cudd_bdd(mgr, b, {true, true}) == false);
}

TEST_CASE("CuddConvert - repeated variable usage", "[cudd_convert]") {
    // Expression: a AND (a OR b) == a
    my_expression expr = my_and{
        std::make_unique<my_expression>(my_variable{"a"}),
        std::make_unique<my_expression>(my_or{std::make_unique<my_expression>(my_variable{"a"}),
                                              std::make_unique<my_expression>(my_variable{"b"})})};

    std::unordered_set<std::string> var_names = {"a", "b"};
    auto p = convert_to_cudd_bdd(expr, var_names);
    REQUIRE(p.first != nullptr);
    Cudd& mgr = *p.first;
    BDD b = p.second;

    // Equivalent to a
    REQUIRE(evaluate_cudd_bdd(mgr, b, {false, false}) == false);
    REQUIRE(evaluate_cudd_bdd(mgr, b, {true, false}) == true);
    REQUIRE(evaluate_cudd_bdd(mgr, b, {true, true}) == true);
}
