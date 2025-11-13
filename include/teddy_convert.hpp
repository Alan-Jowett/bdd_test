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
 * @brief Converts an expression tree to a Binary Decision Diagram (BDD)
 *
 * Performs recursive conversion of the custom expression tree into a BDD using
 * the TeDDy library. Uses dag_walker for efficient variable collection and
 * simple recursion for BDD construction since expressions form a tree structure.
 *
 * The conversion process:
 * 1. Uses dag_walker to collect all unique variable names efficiently
 * 2. Creates a sorted variable mapping for consistent BDD ordering
 * 3. Recursively converts each expression node to BDD operations
 *
 * @param expr The root expression to convert
 * @param mgr Reference to the BDD manager for creating BDD nodes
 * @return BDD diagram representing the logical function
 *
 * @throws std::runtime_error If a variable reference is not found during conversion
 *
 * Operator mappings:
 * - AND -> BDD AND operation
 * - OR -> BDD OR operation
 * - XOR -> BDD XOR operation
 * - NOT -> XOR with constant 1
 * - Variable -> BDD variable node
 */
teddy::bdd_manager::diagram_t convert_to_bdd(const my_expression& expr, teddy::bdd_manager& mgr) {
    using bdd_t = teddy::bdd_manager::diagram_t;
    using namespace teddy::ops;

    // First pass: collect all unique variable names using dag_walker
    std::unordered_set<std::string> variable_names;
    collect_variables_with_dag_walker(expr, variable_names);

    // Build variable map with sorted variable names for consistent ordering
    std::vector<std::string> sorted_vars(variable_names.begin(), variable_names.end());
    std::ranges::sort(sorted_vars);

    std::cout << "TeDDy variable ordering: ";
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

    // Helper function for recursive conversion (no memoization needed for tree structure)
    std::function<bdd_t(const my_expression&)> convert_recursive =
        [&](const my_expression& e) -> bdd_t {
        return std::visit(
            [&](const auto& variant_expr) -> bdd_t {
                using T = std::decay_t<decltype(variant_expr)>;

                if constexpr (std::is_same_v<T, my_variable>) {
                    auto it = var_map.find(variant_expr.variable_name);
                    if (it == var_map.end()) {
                        throw std::runtime_error("Variable not found: "
                                                 + variant_expr.variable_name);
                    }
                    return mgr.variable(it->second);
                } else if constexpr (std::is_same_v<T, my_and>) {
                    bdd_t left_bdd = convert_recursive(*variant_expr.left);
                    bdd_t right_bdd = convert_recursive(*variant_expr.right);
                    return mgr.apply<AND>(left_bdd, right_bdd);
                } else if constexpr (std::is_same_v<T, my_or>) {
                    bdd_t left_bdd = convert_recursive(*variant_expr.left);
                    bdd_t right_bdd = convert_recursive(*variant_expr.right);
                    return mgr.apply<OR>(left_bdd, right_bdd);
                } else if constexpr (std::is_same_v<T, my_not>) {
                    bdd_t expr_bdd = convert_recursive(*variant_expr.expr);
                    bdd_t one = mgr.constant(1);
                    return mgr.apply<XOR>(expr_bdd, one);
                } else if constexpr (std::is_same_v<T, my_xor>) {
                    bdd_t left_bdd = convert_recursive(*variant_expr.left);
                    bdd_t right_bdd = convert_recursive(*variant_expr.right);
                    return mgr.apply<XOR>(left_bdd, right_bdd);
                }
            },
            e);
    };

    return convert_recursive(expr);
}

/**
 * @brief Alternative conversion using TeDDy's built-in from_expression_tree method
 *
 * This function demonstrates using TeDDy's native from_expression_tree method
 * by wrapping our custom expression types with an adapter that conforms to
 * the expression_node concept.
 *
 * The conversion process:
 * 1. Collects all unique variable names using dag_walker
 * 2. Creates a sorted variable mapping for consistent BDD ordering
 * 3. Wraps the root expression with an adapter
 * 4. Uses TeDDy's from_expression_tree method for conversion
 *
 * @param expr The root expression to convert
 * @param mgr Reference to the BDD manager for creating BDD nodes
 * @return BDD diagram representing the logical function
 *
 * @throws std::runtime_error If a variable reference is not found during conversion
 *
 * @note This is an experimental alternative to compare performance and behavior
 *       with our custom recursive conversion approach.
 */
teddy::bdd_manager::diagram_t convert_to_bdd_with_teddy_adapter(const my_expression& expr,
                                                                teddy::bdd_manager& mgr) {
    // First pass: collect all unique variable names using dag_walker
    std::unordered_set<std::string> variable_names;
    collect_variables_with_dag_walker(expr, variable_names);

    // Build variable map with sorted variable names for consistent ordering
    std::vector<std::string> sorted_vars(variable_names.begin(), variable_names.end());
    std::ranges::sort(sorted_vars);

    std::cout << "TeDDy Adapter variable ordering: ";
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

    // Create adapter and use TeDDy's from_expression_tree
    expression_adapter adapter(expr, var_map);
    return mgr.from_expression_tree(adapter);
}
