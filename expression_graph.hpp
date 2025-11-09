// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file expression_graph.hpp
 * @brief Expression tree iterator for DOT graph generation
 *
 * Provides an iterator interface for traversing expression trees and accessing
 * DOT graph properties compatible with the template DOT generation system.
 * The iterator performs depth-first traversal and provides individual property
 * methods for each DOT attribute.
 *
 * @author Alan Jowett
 * @date 2025
 * @copyright Copyright (c) 2025 Alan Jowett. Licensed under the MIT License
 */

#pragma once

#include "expression_types.hpp"
#include "dot_graph_generator.hpp"
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <iostream>

/**
 * @brief Iterator for traversing expression trees with DOT graph properties
 *
 * This iterator traverses an expression tree in depth-first order and provides
 * individual property methods for generating DOT graph attributes. It's designed
 * to be compatible with the templated DOT generation system.
 *
 * The iterator provides the following DOT properties:
 * - get_label(): Node label (variable name or operator)
 * - get_shape(): Node shape (ellipse or box)
 * - get_fillcolor(): Fill color based on node type
 * - get_style(): Style attribute (always "filled")
 * - get_node_address(): Node address for unique identification
 * - get_children(): Child iterators for traversal
 * - get_edge_label(): Label for edges to children
 *
 * Usage example:
 * @code
 * auto expr = parse_expression("(a AND b) OR c");
 * expression_iterator it(expr);
 * generate_dot_graph(it, std::cout, "ExpressionTree");
 * @endcode
 */
class expression_iterator {
public:
    /**
     * @brief Child information for tree traversal
     */
    struct child_info {
        const my_expression* node;  ///< Pointer to child expression
        std::string edge_label;     ///< Edge label ("L", "R", or empty)
        
        child_info(const my_expression* n, std::string label) 
            : node(n), edge_label(std::move(label)) {}
    };

private:
    const my_expression* current_node_;     ///< Current expression node

public:
    /**
     * @brief Constructs iterator from expression pointer
     * 
     * @param expr Expression to iterate over (can be nullptr for end iterator)
     */
    explicit expression_iterator(const my_expression_ptr& expr) 
        : current_node_(expr ? expr.get() : nullptr) {
    }

    /**
     * @brief Constructs iterator from expression reference
     * 
     * @param expr Expression to iterate over
     */
    explicit expression_iterator(const my_expression& expr)
        : current_node_(&expr) {
    }

    /**
     * @brief Default constructor creates end iterator
     */
    expression_iterator() 
        : current_node_(nullptr) {
    }

    /**
     * @brief Copy constructor
     */
    expression_iterator(const expression_iterator& other)
        : current_node_(other.current_node_) {
    }

    /**
     * @brief Move constructor
     */
    expression_iterator(expression_iterator&& other) noexcept
        : current_node_(other.current_node_) {
        other.current_node_ = nullptr;
    }

    /**
     * @brief Copy assignment
     */
    expression_iterator& operator=(const expression_iterator& other) {
        if (this != &other) {
            current_node_ = other.current_node_;
        }
        return *this;
    }

    /**
     * @brief Move assignment
     */
    expression_iterator& operator=(expression_iterator&& other) noexcept {
        if (this != &other) {
            current_node_ = other.current_node_;
            other.current_node_ = nullptr;
        }
        return *this;
    }

    /**
     * @brief Checks if iterator is valid (not end)
     */
    bool is_valid() const {
        return current_node_ != nullptr;
    }

    /**
     * @brief Equality comparison
     */
    bool operator==(const expression_iterator& other) const {
        return current_node_ == other.current_node_;
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
     * @return Pointer address that can be used as a unique key for this node
     */
    const void* get_node_address() const {
        return static_cast<const void*>(current_node_);
    }

    /**
     * @brief Gets node label for DOT display
     * @return Label string (variable name or operator)
     */
    std::string get_label() const {
        if (!current_node_) return "";
        
        return std::visit([](const auto& variant_expr) -> std::string {
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
        }, *current_node_);
    }

    /**
     * @brief Gets node shape for DOT display
     * @return Shape string ("ellipse" for variables, "box" for operators)
     */
    std::string get_shape() const {
        if (!current_node_) return "ellipse";
        
        return std::visit([](const auto& variant_expr) -> std::string {
            using T = std::decay_t<decltype(variant_expr)>;
            
            if constexpr (std::is_same_v<T, my_variable>) {
                return "ellipse";
            } else {
                return "box";
            }
        }, *current_node_);
    }

    /**
     * @brief Gets fill color for DOT display
     * @return Color string based on node type
     */
    std::string get_fillcolor() const {
        if (!current_node_) return "white";
        
        return std::visit([](const auto& variant_expr) -> std::string {
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
        }, *current_node_);
    }

    /**
     * @brief Gets style for DOT display
     * @return Style string (always "filled")
     */
    std::string get_style() const {
        return "filled";
    }

    /**
     * @brief Gets font name for DOT display
     * @return Font name string
     */
    std::string get_fontname() const {
        return "Arial";
    }

    /**
     * @brief Gets child information for tree traversal
     * @return Vector of child_info containing child pointers and edge labels
     */
    std::vector<child_info> get_children_info() const {
        std::vector<child_info> children;
        
        if (!current_node_) return children;
        
        std::visit([&](const auto& variant_expr) {
            using T = std::decay_t<decltype(variant_expr)>;
            
            if constexpr (std::is_same_v<T, my_and> || 
                          std::is_same_v<T, my_or> || 
                          std::is_same_v<T, my_xor>) {
                if (variant_expr.left) {
                    children.emplace_back(variant_expr.left.get(), "L");
                }
                if (variant_expr.right) {
                    children.emplace_back(variant_expr.right.get(), "R");
                }
            } else if constexpr (std::is_same_v<T, my_not>) {
                if (variant_expr.expr) {
                    children.emplace_back(variant_expr.expr.get(), "");
                }
            }
            // Variables have no children
        }, *current_node_);
        
        return children;
    }

    /**
     * @brief Gets child iterators for tree traversal (compatible with template system)
     * @return Vector of expression_iterator objects for child nodes
     */
    std::vector<expression_iterator> get_children() const {
        std::vector<expression_iterator> children;
        auto child_infos = get_children_info();
        
        for (const auto& info : child_infos) {
            children.emplace_back(*info.node);
        }
        
        return children;
    }

    /**
     * @brief Gets edge label for connection to parent
     * @param child_index Index of this child (0 for left/first, 1 for right/second)
     * @return Edge label string
     */
    std::string get_edge_label(size_t child_index = 0) const {
        // This method is called on child iterators to get their edge labels
        // The actual edge label is determined by the parent's get_children_info() method
        // This method exists for compatibility but isn't used in the tree traversal
        return "";
    }

    /**
     * @brief Gets edge label for a specific child (dot_template compatible)
     * @param child Child iterator to get edge label for
     * @param child_index Index of the child (0 for left/first, 1 for right/second)
     * @return Edge label string for that child
     */
    std::string get_edge_label(const expression_iterator& child, size_t child_index) const {
        auto child_infos = get_children_info();
        if (child_index < child_infos.size()) {
            return child_infos[child_index].edge_label;
        }
        return "";
    }

    /**
     * @brief Gets edge label for a specific child by index
     * @param child_index Index of the child (0 for left/first, 1 for right/second)
     * @return Edge label string for that child
     */
    std::string get_child_edge_label(size_t child_index) const {
        auto child_infos = get_children_info();
        if (child_index < child_infos.size()) {
            return child_infos[child_index].edge_label;
        }
        return "";
    }

};

// ============================================================================
// DOT Generation Functions
// ============================================================================

/**
 * @brief Writes expression tree as DOT graph using the iterator and template system
 *
 * This function uses the expression_iterator with the dot_graph_generator.hpp system 
 * for generating DOT format graphs from expression trees. It produces identical output 
 * to the original function but uses the templated approach.
 *
 * Key improvements:
 * - Uses iterator pattern for consistent tree traversal
 * - Leverages templated DOT system for extensibility
 * - Individual property methods for maximum flexibility
 * - Automatic edge label detection and generation
 * - Configurable graph appearance through DotConfig
 *
 * Generated DOT format includes:
 * - Node properties: label, shape, style, fillcolor based on expression type
 * - Edge properties: labels for left ("L"), right ("R"), and NOT ("") children
 * - Graph directives: rankdir=TB, Arial font, proper styling
 *
 * @param expr The expression tree node to process
 * @param out Output stream for DOT content
 * @param graph_name Name for the generated DOT graph (default: "ExpressionTree")
 *
 * Example usage:
 * @code
 * auto expr = parse_expression("(a AND b) OR c");
 * std::ofstream file("output.dot");
 * write_expression_to_dot(*expr, file, "MyExpression");
 * @endcode
 *
 * @note Uses expression_iterator and dot_graph_generator.hpp
 */
inline void write_expression_to_dot(const my_expression& expr, std::ostream& out, 
                                    const std::string& graph_name = "ExpressionTree") {
    // Create an iterator from the expression
    expression_iterator root_iter(expr);
    
    // Configure the DOT generation for expression trees to match original format
    dot_graph::DotConfig config(graph_name);
    config.rankdir = "TB";                    // Top-to-bottom layout
    config.font_name = "Arial";               // Consistent font
    config.default_node_shape = "";          // Don't add default shape (overridden anyway)
    config.default_node_style = "";          // Don't add default style (overridden anyway)
    config.default_edge_style = "";          // Don't add default edge style
    config.show_node_ids = false;            // Don't show internal IDs
    
    // Generate the DOT graph using the template system
    dot_graph::generate_dot_graph(root_iter, out, config);
}

/**
 * @brief Convenience wrapper that includes DOT graph wrapper
 *
 * This version automatically wraps the output in a complete DOT digraph
 * structure, making it ready for direct use with Graphviz tools.
 *
 * @param expr The expression tree to convert
 * @param out Output stream for complete DOT content
 * @param graph_name Name for the DOT graph
 */
inline void write_complete_expression_dot(const my_expression& expr, std::ostream& out,
                                          const std::string& graph_name = "ExpressionTree") {
    // The template system already provides complete DOT output
    write_expression_to_dot(expr, out, graph_name);
}