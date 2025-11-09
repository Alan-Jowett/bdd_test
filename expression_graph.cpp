// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file expression_graph.cpp
 * @brief Expression tree graph generation implementation
 *
 * Implementation of expression tree DOT generation functions and iterator class.
 * Provides the implementation for visualizing expression trees using the
 * templated DOT generation system.
 *
 * @author Alan Jowett
 * @date 2025
 * @copyright Copyright (c) 2025 Alan Jowett. Licensed under the MIT License
 */

#include "expression_graph.hpp"

#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "dot_graph_generator.hpp"

// ============================================================================
// Expression Iterator Class (Implementation Detail)
// ============================================================================

/**
 * @brief Iterator for traversing expression trees with DOT graph properties
 *
 * This iterator traverses an expression tree in depth-first order and provides
 * individual property methods for generating DOT graph attributes. It's designed
 * to be compatible with the templated DOT generation system.
 */
class expression_iterator {
   private:
    const my_expression* current_expr_;  ///< Current expression node
    mutable std::unordered_map<const my_expression*, std::string> id_cache_;  ///< Node ID cache
    mutable size_t next_id_;                                                  ///< Next available ID

    /**
     * @brief Generates unique ID for expression nodes
     * @param expr Expression node to get ID for
     * @return Unique string identifier
     */
    std::string get_node_id(const my_expression* expr) const {
        if (id_cache_.find(expr) == id_cache_.end()) {
            id_cache_[expr] = "node" + std::to_string(next_id_++);
        }
        return id_cache_[expr];
    }

   public:
    /**
     * @brief Constructs iterator from expression
     * @param expr Expression to iterate over (can be nullptr for end iterator)
     */
    explicit expression_iterator(const my_expression& expr) : current_expr_(&expr), next_id_(0) {}

    /**
     * @brief Default constructor for end iterator
     */
    expression_iterator() : current_expr_(nullptr), next_id_(0) {}

    /**
     * @brief Copy constructor
     */
    expression_iterator(const expression_iterator& other)
        : current_expr_(other.current_expr_),
          id_cache_(other.id_cache_),
          next_id_(other.next_id_) {}

    /**
     * @brief Move constructor
     */
    expression_iterator(expression_iterator&& other) noexcept
        : current_expr_(other.current_expr_),
          id_cache_(std::move(other.id_cache_)),
          next_id_(other.next_id_) {
        other.current_expr_ = nullptr;
    }

    /**
     * @brief Copy assignment
     */
    expression_iterator& operator=(const expression_iterator& other) {
        if (this != &other) {
            current_expr_ = other.current_expr_;
            id_cache_ = other.id_cache_;
            next_id_ = other.next_id_;
        }
        return *this;
    }

    /**
     * @brief Move assignment
     */
    expression_iterator& operator=(expression_iterator&& other) noexcept {
        if (this != &other) {
            current_expr_ = other.current_expr_;
            id_cache_ = std::move(other.id_cache_);
            next_id_ = other.next_id_;
            other.current_expr_ = nullptr;
        }
        return *this;
    }

    /**
     * @brief Checks if iterator is valid (not end)
     */
    bool is_valid() const {
        return current_expr_ != nullptr;
    }

    /**
     * @brief Equality comparison
     */
    bool operator==(const expression_iterator& other) const {
        return current_expr_ == other.current_expr_;
    }

    /**
     * @brief Inequality comparison
     */
    bool operator!=(const expression_iterator& other) const {
        return !(*this == other);
    }

    // ========================================================================
    // DOT Graph Property Methods (required by template system)
    // ========================================================================

    /**
     * @brief Gets node address for unique identification
     */
    const void* get_node_address() const {
        return static_cast<const void*>(current_expr_);
    }

    /**
     * @brief Gets node label for DOT display
     */
    std::string get_label() const {
        if (!current_expr_)
            return "";

        return std::visit(
            [](const auto& variant_expr) -> std::string {
                using T = std::decay_t<decltype(variant_expr)>;

                if constexpr (std::is_same_v<T, my_variable>) {
                    return variant_expr.variable_name;
                } else if constexpr (std::is_same_v<T, my_and>) {
                    return "AND";
                } else if constexpr (std::is_same_v<T, my_or>) {
                    return "OR";
                } else if constexpr (std::is_same_v<T, my_not>) {
                    return "NOT";
                } else if constexpr (std::is_same_v<T, my_xor>) {
                    return "XOR";
                }
                return "";
            },
            *current_expr_);
    }

    /**
     * @brief Gets node shape for DOT display
     */
    std::string get_shape() const {
        if (!current_expr_)
            return "ellipse";

        return std::visit(
            [](const auto& variant_expr) -> std::string {
                using T = std::decay_t<decltype(variant_expr)>;

                if constexpr (std::is_same_v<T, my_variable>) {
                    return "ellipse";
                } else {
                    return "box";
                }
            },
            *current_expr_);
    }

    /**
     * @brief Gets fill color for DOT display
     */
    std::string get_fillcolor() const {
        if (!current_expr_)
            return "white";

        return std::visit(
            [](const auto& variant_expr) -> std::string {
                using T = std::decay_t<decltype(variant_expr)>;

                if constexpr (std::is_same_v<T, my_variable>) {
                    return "lightblue";
                } else if constexpr (std::is_same_v<T, my_and>) {
                    return "lightgreen";
                } else if constexpr (std::is_same_v<T, my_or>) {
                    return "lightcoral";
                } else if constexpr (std::is_same_v<T, my_not>) {
                    return "yellow";
                } else if constexpr (std::is_same_v<T, my_xor>) {
                    return "lightpink";
                }
                return "white";
            },
            *current_expr_);
    }

    /**
     * @brief Gets style for DOT display
     */
    std::string get_style() const {
        return "filled";
    }

    /**
     * @brief Gets child iterators for tree traversal
     */
    std::vector<expression_iterator> get_children() const {
        std::vector<expression_iterator> children;

        if (!current_expr_)
            return children;

        std::visit(
            [&](const auto& variant_expr) {
                using T = std::decay_t<decltype(variant_expr)>;

                if constexpr (std::is_same_v<T, my_and> || std::is_same_v<T, my_or>
                              || std::is_same_v<T, my_xor>) {
                    if (variant_expr.left) {
                        children.emplace_back(*variant_expr.left);
                    }
                    if (variant_expr.right) {
                        children.emplace_back(*variant_expr.right);
                    }
                } else if constexpr (std::is_same_v<T, my_not>) {
                    if (variant_expr.expr) {
                        children.emplace_back(*variant_expr.expr);
                    }
                }
                // Variables have no children
            },
            *current_expr_);

        return children;
    }

    /**
     * @brief Gets edge label for a specific child
     */
    std::string get_edge_label(const expression_iterator& child, size_t child_index) const {
        if (!current_expr_)
            return "";

        return std::visit(
            [child_index](const auto& variant_expr) -> std::string {
                using T = std::decay_t<decltype(variant_expr)>;

                if constexpr (std::is_same_v<T, my_and> || std::is_same_v<T, my_or>
                              || std::is_same_v<T, my_xor>) {
                    return child_index == 0 ? "L" : "R";
                } else {
                    return "";
                }
            },
            *current_expr_);
    }
};

// ============================================================================
// Expression DOT Generation Functions Implementation
// ============================================================================

void write_expression_to_dot(const my_expression& expr, std::ostream& out,
                             const std::string& graph_name) {
    // Create an iterator from the expression
    expression_iterator root_iter(expr);

    // Configure the DOT generation for expression trees to match original format
    dot_graph::DotConfig config(graph_name);
    config.rankdir = "TB";           // Top-to-bottom layout
    config.font_name = "Arial";      // Consistent font
    config.default_node_shape = "";  // Don't add default shape (overridden anyway)
    config.default_node_style = "";  // Don't add default style (overridden anyway)
    config.default_edge_style = "";  // Don't add default edge style
    config.show_node_ids = false;    // Don't show internal IDs

    // Generate the DOT graph using the template system
    dot_graph::generate_dot_graph(root_iter, out, config);
}

void write_complete_expression_dot(const my_expression& expr, std::ostream& out,
                                   const std::string& graph_name) {
    // The template system already provides complete DOT output
    write_expression_to_dot(expr, out, graph_name);
}