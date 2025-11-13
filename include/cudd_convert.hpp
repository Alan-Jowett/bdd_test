// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file cudd_convert.hpp
 * @brief CUDD BDD library integration and conversion utilities
 *
 * This file provides conversion functions and utilities for working with the CUDD
 * Binary Decision Diagram library. It includes functions for converting between
 * different BDD representations and managing CUDD-specific data structures.
 *
 * @author Alan Jowett
 * @date 2025
 * @copyright Copyright (c) 2025 Alan Jowett. Licensed under the MIT License
 */

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <libteddy/core.hpp>
#include <memory>
#include <ranges>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

// CUDD headers for BDD comparison
#include <cudd/cudd.h>

#include <cudd/cuddObj.hh>

#include "cudd_graph.hpp"
#include "dag_walker.hpp"
#include "expression_adapter.hpp"
#include "expression_graph.hpp"
#include "expression_parser.hpp"
#include "node_table_generator.hpp"
#include "teddy_graph.hpp"

/**
 * @brief Convert expression to CUDD BDD format for comparison
 *
 * This function converts a parsed expression to CUDD BDD format using the CUDD library.
 * CUDD (Colorado University Decision Diagram) is a mature BDD library that provides
 * efficient BDD operations and automatic variable reordering. This function is used
 * for comparing BDD implementations.
 *
 * @param expr The expression to convert
 * @param variable_names The set of variable names used in the expression
 * @return Pair containing the CUDD manager and the root BDD
 */
inline std::pair<std::unique_ptr<Cudd>, BDD> convert_to_cudd_bdd(
    const my_expression& expr, const std::unordered_set<std::string>& variable_names) {
    // Create CUDD manager
    auto cudd_mgr = std::make_unique<Cudd>();

    // Create variable mapping
    std::vector<std::string> sorted_vars(variable_names.begin(), variable_names.end());
    std::ranges::sort(sorted_vars);

    std::cout << "CUDD variable ordering: ";
    for (size_t i = 0; i < sorted_vars.size(); ++i) {
        std::cout << sorted_vars[i] << "=" << i;
        if (i < sorted_vars.size() - 1)
            std::cout << ", ";
    }
    std::cout << std::endl;

    std::unordered_map<std::string, int> var_map;
    for (size_t i = 0; i < sorted_vars.size(); ++i) {
        var_map[sorted_vars[i]] = static_cast<int>(i);
    }

    // Recursive lambda to convert expression to CUDD BDD
    std::function<BDD(const my_expression&)> convert_recursive =
        [&](const my_expression& e) -> BDD {
        return std::visit(
            [&](const auto& content) -> BDD {
                using T = std::decay_t<decltype(content)>;

                if constexpr (std::is_same_v<T, my_variable>) {
                    auto it = var_map.find(content.variable_name);
                    if (it == var_map.end()) {
                        throw std::runtime_error("Variable not found in variable map: "
                                                 + content.variable_name);
                    }
                    return cudd_mgr->bddVar(it->second);
                } else if constexpr (std::is_same_v<T, my_not>) {
                    BDD expr_bdd = convert_recursive(*content.expr);
                    BDD one = cudd_mgr->bddOne();
                    return expr_bdd ^ one;
                } else if constexpr (std::is_same_v<T, my_and>) {
                    BDD left = convert_recursive(*content.left);
                    BDD right = convert_recursive(*content.right);
                    return left & right;
                } else if constexpr (std::is_same_v<T, my_or>) {
                    BDD left = convert_recursive(*content.left);
                    BDD right = convert_recursive(*content.right);
                    return left | right;
                } else if constexpr (std::is_same_v<T, my_xor>) {
                    BDD left = convert_recursive(*content.left);
                    BDD right = convert_recursive(*content.right);
                    return left ^ right;
                }

                // This should never be reached
                throw std::runtime_error("Unknown expression type");
            },
            e);
    };

    // Convert the expression
    BDD result = convert_recursive(expr);

    return std::make_pair(std::move(cudd_mgr), result);
}
