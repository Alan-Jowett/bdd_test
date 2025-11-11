// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file expression_adapter.hpp
 * @brief Adapter to make our expression types work with TeDDy's from_expression_tree
 *
 * This file contains an adapter class that wraps our custom expression types
 * (my_expression) to conform to TeDDy's expression_node concept, allowing us
 * to use the library's built-in from_expression_tree method.
 *
 * @author Alan Jowett
 * @date 2025
 * @copyright Copyright (c) 2025 Alan Jowett. Licensed under the MIT License
 */

#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>

#include "expression_types.hpp"

/**
 * @brief Adapter class to make our expression types compatible with TeDDy's expression_node concept
 *
 * This class wraps our my_expression variant and provides the interface required
 * by TeDDy's from_expression_tree method. The adapter maintains a reference to
 * a variable mapping to convert variable names to indices.
 */
class expression_adapter {
   public:
    /**
     * @brief Constructs an adapter for the given expression
     *
     * @param expr Reference to the expression to wrap
     * @param var_map Reference to variable name to index mapping
     */
    explicit expression_adapter(const my_expression& expr,
                                const std::unordered_map<std::string, int>& var_map)
        : expr_(expr), var_map_(var_map), left_adapter_(nullptr), right_adapter_(nullptr) {
        // Pre-create child adapters for operations
        initialize_child_adapters();
    }

    /**
     * @brief Copy constructor
     */
    expression_adapter(const expression_adapter& other)
        : expr_(other.expr_),
          var_map_(other.var_map_),
          left_adapter_(nullptr),
          right_adapter_(nullptr) {
        initialize_child_adapters();
    }

    /**
     * @brief Check if this node represents a variable
     * @return true if the wrapped expression is a my_variable
     */
    bool is_variable() const {
        return std::holds_alternative<my_variable>(expr_);
    }

    /**
     * @brief Check if this node represents a constant
     * @return false (our expression system doesn't have explicit constants)
     *
     * @note Our expression system doesn't have explicit constant nodes,
     *       so this always returns false. Constants are handled during evaluation.
     */
    bool is_constant() const {
        return false;  // Our expression system doesn't have explicit constants
    }

    /**
     * @brief Check if this node represents an operation
     * @return true if the wrapped expression is an operation (AND, OR, NOT, XOR)
     */
    bool is_operation() const {
        return std::holds_alternative<my_and>(expr_) || std::holds_alternative<my_or>(expr_)
               || std::holds_alternative<my_not>(expr_) || std::holds_alternative<my_xor>(expr_);
    }

    /**
     * @brief Get the variable index (for variable nodes)
     * @return The index of the variable in the BDD manager
     * @throws std::runtime_error if called on non-variable node or variable not found
     */
    int32_t get_index() const {
        if (!is_variable()) {
            throw std::runtime_error("get_index() called on non-variable node");
        }

        const auto& var = std::get<my_variable>(expr_);
        auto it = var_map_.find(var.variable_name);
        if (it == var_map_.end()) {
            throw std::runtime_error("Variable not found: " + var.variable_name);
        }
        return it->second;
    }

    /**
     * @brief Get the constant value (for constant nodes)
     * @return The constant value
     * @throws std::runtime_error since our expression system doesn't have constants
     */
    int32_t get_value() const {
        throw std::runtime_error(
            "get_value() called - our expression system doesn't have constants");
    }

    /**
     * @brief Evaluate the operation with given operand values
     * @param left_val Value of left operand
     * @param right_val Value of right operand
     * @return Result of the operation
     * @throws std::runtime_error if called on non-operation node
     */
    int32_t evaluate(int32_t left_val, int32_t right_val) const {
        return std::visit(
            [left_val, right_val](const auto& variant_expr) -> int32_t {
                using T = std::decay_t<decltype(variant_expr)>;

                if constexpr (std::is_same_v<T, my_and>) {
                    return (left_val != 0 && right_val != 0) ? 1 : 0;
                } else if constexpr (std::is_same_v<T, my_or>) {
                    return (left_val != 0 || right_val != 0) ? 1 : 0;
                } else if constexpr (std::is_same_v<T, my_xor>) {
                    return ((left_val != 0) != (right_val != 0)) ? 1 : 0;
                } else if constexpr (std::is_same_v<T, my_not>) {
                    // For NOT, we only use the left value (right is ignored)
                    return (left_val != 0) ? 0 : 1;
                } else {
                    throw std::runtime_error("evaluate() called on non-operation node");
                }
            },
            expr_);
    }

    /**
     * @brief Get the left child node (for operation nodes)
     * @return Reference to adapter wrapping the left child expression
     * @throws std::runtime_error if called on non-operation node or node has no left child
     */
    const expression_adapter& get_left() const {
        if (!left_adapter_) {
            throw std::runtime_error(
                "get_left() called on non-operation node or left child not available");
        }
        return *left_adapter_;
    }

    /**
     * @brief Get the right child node (for binary operation nodes)
     * @return Reference to adapter wrapping the right child expression
     * @throws std::runtime_error if called on non-binary-operation node or node has no right child
     */
    const expression_adapter& get_right() const {
        if (!right_adapter_) {
            throw std::runtime_error(
                "get_right() called on non-operation node or right child not available");
        }
        return *right_adapter_;
    }

   private:
    /**
     * @brief Initialize child adapters based on the expression type
     */
    void initialize_child_adapters() {
        std::visit(
            [this](const auto& variant_expr) {
                using T = std::decay_t<decltype(variant_expr)>;

                if constexpr (std::is_same_v<T, my_and> || std::is_same_v<T, my_or>
                              || std::is_same_v<T, my_xor>) {
                    if (variant_expr.left) {
                        left_adapter_ =
                            std::make_unique<expression_adapter>(*variant_expr.left, var_map_);
                    }
                    if (variant_expr.right) {
                        right_adapter_ =
                            std::make_unique<expression_adapter>(*variant_expr.right, var_map_);
                    }
                } else if constexpr (std::is_same_v<T, my_not>) {
                    if (variant_expr.expr) {
                        left_adapter_ =
                            std::make_unique<expression_adapter>(*variant_expr.expr, var_map_);
                        // For NOT operations, TeDDy's from_expression_tree expects binary
                        // operations. Create a dummy right adapter with the same expression to
                        // satisfy this requirement.
                        right_adapter_ =
                            std::make_unique<expression_adapter>(*variant_expr.expr, var_map_);
                    }
                }
            },
            expr_);
    }

    const my_expression& expr_;                            ///< Reference to the wrapped expression
    const std::unordered_map<std::string, int>& var_map_;  ///< Reference to variable mapping
    std::unique_ptr<expression_adapter> left_adapter_;     ///< Left child adapter
    std::unique_ptr<expression_adapter> right_adapter_;    ///< Right child adapter
};