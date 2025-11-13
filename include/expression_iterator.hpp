// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file expression_iterator.hpp
 * @brief Iterator interface for expression tree traversal and BDD operations
 *
 * This file provides iterator classes and utilities for traversing expression trees
 * and performing Binary Decision Diagram operations. It includes specialized iterators
 * for different expression node types and supports various traversal patterns.
 *
 * @author Alan Jowett
 * @date 2025
 * @copyright Copyright (c) 2025 Alan Jowett. Licensed under the MIT License
 */

#pragma once

#include <format>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

#include "expression_types.hpp"

// ============================================================================
// Expression Constants and Traits
// ============================================================================

namespace expression_constants {
// Operator labels (constexpr for compile-time evaluation)
inline constexpr const char* and_label() {
    return "AND";
}
inline constexpr const char* or_label() {
    return "OR";
}
inline constexpr const char* not_label() {
    return "NOT";
}
inline constexpr const char* xor_label() {
    return "XOR";
}

// Edge labels (constexpr for compile-time evaluation)
inline constexpr const char* left_edge() {
    return "L";
}
inline constexpr const char* right_edge() {
    return "R";
}

// Node shapes (constexpr for compile-time evaluation)
inline constexpr const char* variable_shape() {
    return "ellipse";
}
inline constexpr const char* operator_shape() {
    return "box";
}

// Fill colors (constexpr for compile-time evaluation)
inline constexpr const char* variable_color() {
    return "lightblue";
}
inline constexpr const char* and_color() {
    return "lightgreen";
}
inline constexpr const char* or_color() {
    return "lightcoral";
}
inline constexpr const char* not_color() {
    return "yellow";
}
inline constexpr const char* xor_color() {
    return "lightpink";
}
inline constexpr const char* default_color() {
    return "white";
}

// Style (constexpr for compile-time evaluation)
inline constexpr const char* filled_style() {
    return "filled";
}

}  // namespace expression_constants

namespace detail {
/**
 * @brief Expression type traits for eliminating code duplication
 */
template <typename T>
struct expression_traits {
    static constexpr const char* label = "";
    static constexpr const char* shape = expression_constants::operator_shape();
    static constexpr const char* fillcolor = expression_constants::default_color();
};

template <>
struct expression_traits<my_variable> {
    static constexpr const char* label = "";  // Will use actual variable name
    static constexpr const char* shape = expression_constants::variable_shape();
    static constexpr const char* fillcolor = expression_constants::variable_color();
};

template <>
struct expression_traits<my_and> {
    static constexpr const char* label = expression_constants::and_label();
    static constexpr const char* shape = expression_constants::operator_shape();
    static constexpr const char* fillcolor = expression_constants::and_color();
};

template <>
struct expression_traits<my_or> {
    static constexpr const char* label = expression_constants::or_label();
    static constexpr const char* shape = expression_constants::operator_shape();
    static constexpr const char* fillcolor = expression_constants::or_color();
};

template <>
struct expression_traits<my_not> {
    static constexpr const char* label = expression_constants::not_label();
    static constexpr const char* shape = expression_constants::operator_shape();
    static constexpr const char* fillcolor = expression_constants::not_color();
};

template <>
struct expression_traits<my_xor> {
    static constexpr const char* label = expression_constants::xor_label();
    static constexpr const char* shape = expression_constants::operator_shape();
    static constexpr const char* fillcolor = expression_constants::xor_color();
};
}  // namespace detail

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
                    // Variables use their actual name, not the trait label
                    return variant_expr.variable_name;
                } else {
                    // Operators use their trait-defined label
                    return detail::expression_traits<T>::label;
                }
            },
            *current_expr_);
    }

    /**
     * @brief Gets node shape for DOT display
     */
    std::string get_shape() const {
        if (!current_expr_)
            return expression_constants::variable_shape();

        return std::visit(
            [](const auto& variant_expr) -> std::string {
                using T = std::decay_t<decltype(variant_expr)>;
                return detail::expression_traits<T>::shape;
            },
            *current_expr_);
    }

    /**
     * @brief Gets fill color for DOT display
     */
    std::string get_fillcolor() const {
        if (!current_expr_)
            return expression_constants::default_color();

        return std::visit(
            [](const auto& variant_expr) -> std::string {
                using T = std::decay_t<decltype(variant_expr)>;
                return detail::expression_traits<T>::fillcolor;
            },
            *current_expr_);
    }

    /**
     * @brief Gets style for DOT display
     */
    std::string get_style() const {
        return expression_constants::filled_style();
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
                    return child_index == 0 ? expression_constants::left_edge()
                                            : expression_constants::right_edge();
                } else {
                    return "";
                }
            },
            *current_expr_);
    }
};
