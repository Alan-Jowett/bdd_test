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
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "dag_walker.hpp"
#include "dot_graph_generator.hpp"

// ============================================================================
// Expression Constants and Traits
// ============================================================================

namespace expression_constants {
// Operator labels (consteval for compile-time evaluation)
inline consteval const char* and_label() {
    return "AND";
}
inline consteval const char* or_label() {
    return "OR";
}
inline consteval const char* not_label() {
    return "NOT";
}
inline consteval const char* xor_label() {
    return "XOR";
}

// Edge labels (consteval for compile-time evaluation)
inline consteval const char* left_edge() {
    return "L";
}
inline consteval const char* right_edge() {
    return "R";
}

// Node shapes (consteval for compile-time evaluation)
inline consteval const char* variable_shape() {
    return "ellipse";
}
inline consteval const char* operator_shape() {
    return "box";
}

// Fill colors (consteval for compile-time evaluation)
inline consteval const char* variable_color() {
    return "lightblue";
}
inline consteval const char* and_color() {
    return "lightgreen";
}
inline consteval const char* or_color() {
    return "lightcoral";
}
inline consteval const char* not_color() {
    return "yellow";
}
inline consteval const char* xor_color() {
    return "lightpink";
}
inline consteval const char* default_color() {
    return "white";
}

// Style (consteval for compile-time evaluation)
inline consteval const char* filled_style() {
    return "filled";
}

// Legacy constants for backward compatibility
inline constexpr const char* AND_LABEL = and_label();
inline constexpr const char* OR_LABEL = or_label();
inline constexpr const char* NOT_LABEL = not_label();
inline constexpr const char* XOR_LABEL = xor_label();
inline constexpr const char* LEFT_EDGE = left_edge();
inline constexpr const char* RIGHT_EDGE = right_edge();
inline constexpr const char* VARIABLE_SHAPE = variable_shape();
inline constexpr const char* OPERATOR_SHAPE = operator_shape();
inline constexpr const char* VARIABLE_COLOR = variable_color();
inline constexpr const char* AND_COLOR = and_color();
inline constexpr const char* OR_COLOR = or_color();
inline constexpr const char* NOT_COLOR = not_color();
inline constexpr const char* XOR_COLOR = xor_color();
inline constexpr const char* DEFAULT_COLOR = default_color();
inline constexpr const char* FILLED_STYLE = filled_style();
}  // namespace expression_constants

namespace detail {
/**
 * @brief Expression type traits for eliminating code duplication
 */
template <typename T>
struct expression_traits {
    static constexpr const char* label = "";
    static constexpr const char* shape = expression_constants::OPERATOR_SHAPE;
    static constexpr const char* fillcolor = expression_constants::DEFAULT_COLOR;
};

template <>
struct expression_traits<my_variable> {
    static constexpr const char* label = "";  // Will use actual variable name
    static constexpr const char* shape = expression_constants::VARIABLE_SHAPE;
    static constexpr const char* fillcolor = expression_constants::VARIABLE_COLOR;
};

template <>
struct expression_traits<my_and> {
    static constexpr const char* label = expression_constants::AND_LABEL;
    static constexpr const char* shape = expression_constants::OPERATOR_SHAPE;
    static constexpr const char* fillcolor = expression_constants::AND_COLOR;
};

template <>
struct expression_traits<my_or> {
    static constexpr const char* label = expression_constants::OR_LABEL;
    static constexpr const char* shape = expression_constants::OPERATOR_SHAPE;
    static constexpr const char* fillcolor = expression_constants::OR_COLOR;
};

template <>
struct expression_traits<my_not> {
    static constexpr const char* label = expression_constants::NOT_LABEL;
    static constexpr const char* shape = expression_constants::OPERATOR_SHAPE;
    static constexpr const char* fillcolor = expression_constants::NOT_COLOR;
};

template <>
struct expression_traits<my_xor> {
    static constexpr const char* label = expression_constants::XOR_LABEL;
    static constexpr const char* shape = expression_constants::OPERATOR_SHAPE;
    static constexpr const char* fillcolor = expression_constants::XOR_COLOR;
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
            return expression_constants::VARIABLE_SHAPE;

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
            return expression_constants::DEFAULT_COLOR;

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
        return expression_constants::FILLED_STYLE;
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
                    return child_index == 0 ? expression_constants::LEFT_EDGE
                                            : expression_constants::RIGHT_EDGE;
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
    dot_graph::DotConfig config{
        .graph_name = graph_name,
        .rankdir = "TB",           // Top-to-bottom layout
        .font_name = "Arial",      // Consistent font
        .default_node_shape = "",  // Don't add default shape (overridden anyway)
        .default_node_style = "",  // Don't add default style (overridden anyway)
        .default_edge_style = "",  // Don't add default edge style
        .show_node_ids = false     // Don't show internal IDs
    };

    // Generate the DOT graph using the template system
    dot_graph::generate_dot_graph(root_iter, out, config);
}

// ============================================================================
// DAG Walker Based Variable Collection
// ============================================================================

void collect_variables_with_dag_walker(const my_expression& expr,
                                       std::unordered_set<std::string>& variables) {
    expression_iterator root_iter(expr);

    dag_walker::walk_dag(
        root_iter, [&](const dag_walker::NodeInfo<expression_iterator>& node_info) {
            if (!node_info.is_revisit) {
                // Type-safe variable detection using direct variant access
                const my_expression* current_expr =
                    reinterpret_cast<const my_expression*>(node_info.node.get_node_address());

                if (current_expr) {
                    std::visit(
                        [&](const auto& variant_expr) {
                            using T = std::decay_t<decltype(variant_expr)>;
                            if constexpr (std::is_same_v<T, my_variable>) {
                                variables.insert(variant_expr.variable_name);
                            }
                            // Operators are ignored - only variables are collected
                        },
                        *current_expr);
                }
            }
        });
}