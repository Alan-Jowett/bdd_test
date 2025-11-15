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
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "dag_walker.hpp"
#include "dot_graph_generator.hpp"
#include "expression_iterator.hpp"
#include "node_id_allocator.hpp"

// ============================================================================
// Expression DOT Generation Functions Implementation
// ============================================================================

void write_expression_to_dot(const my_expression& expr, std::ostream& out,
                             const std::string& graph_name) {
    // Create an iterator from the expression
    expression_iterator root_iter(expr);

    // Configure the DOT generation for expression trees
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

    dag_walker::walk_dag_preorder(
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

// ============================================================================
// Mermaid Graph Generation
// ============================================================================

void write_expression_to_mermaid(const my_expression& expr, std::ostream& out,
                                 const std::string& graph_title) {
    out << "---\n";
    out << "title: " << graph_title << "\n";
    out << "---\n";
    out << "flowchart TD\n";

    // Simple recursive approach for Mermaid generation
    std::vector<std::string> node_definitions;
    std::vector<std::string> edges;
    std::vector<std::string> class_assignments;

    // Use node_id_allocator to generate stable node IDs
    graph_common::node_id_allocator id_alloc("N", 1);

    auto get_node_id = [&](const my_expression* expr_ptr) -> std::string {
        const void* ptr = static_cast<const void*>(expr_ptr);
        return id_alloc.get_id(ptr);
    };

    // Recursive function to process expressions
    std::function<void(const my_expression*)> process_expr = [&](const my_expression* expr_ptr) {
        if (!expr_ptr)
            return;

        std::string node_id = get_node_id(expr_ptr);
        std::string css_class;

        // Generate node definition based on expression type
        std::string node_def = std::visit(
            [&](const auto& variant_expr) -> std::string {
                using T = std::decay_t<decltype(variant_expr)>;
                if constexpr (std::is_same_v<T, my_variable>) {
                    // Variables as circles: N1(("var_name"))
                    css_class = "variable";
                    return node_id + "((\"" + variant_expr.variable_name + "\"))";
                } else if constexpr (std::is_same_v<T, my_and>) {
                    // Operators as rectangles: N1["AND"]
                    css_class = "andOp";
                    return node_id + "[\"AND\"]";
                } else if constexpr (std::is_same_v<T, my_or>) {
                    css_class = "orOp";
                    return node_id + "[\"OR\"]";
                } else if constexpr (std::is_same_v<T, my_not>) {
                    css_class = "notOp";
                    return node_id + "[\"NOT\"]";
                } else if constexpr (std::is_same_v<T, my_xor>) {
                    css_class = "xorOp";
                    return node_id + "[\"XOR\"]";
                }
                return "";
            },
            *expr_ptr);

        node_definitions.push_back("    " + node_def);

        // Add CSS class assignment for this node
        if (!css_class.empty()) {
            class_assignments.push_back("    class " + node_id + " " + css_class);
        }

        // Process children and create edges
        std::visit(
            [&](const auto& variant_expr) {
                using T = std::decay_t<decltype(variant_expr)>;
                if constexpr (std::is_same_v<T, my_and> || std::is_same_v<T, my_or>
                              || std::is_same_v<T, my_xor>) {
                    // Binary operators have two children
                    if (variant_expr.left) {
                        std::string left_id = get_node_id(variant_expr.left.get());
                        edges.push_back("    " + node_id + " --> " + left_id);
                        process_expr(variant_expr.left.get());
                    }
                    if (variant_expr.right) {
                        std::string right_id = get_node_id(variant_expr.right.get());
                        edges.push_back("    " + node_id + " --> " + right_id);
                        process_expr(variant_expr.right.get());
                    }
                } else if constexpr (std::is_same_v<T, my_not>) {
                    // Unary operator has one child
                    if (variant_expr.expr) {
                        std::string child_id = get_node_id(variant_expr.expr.get());
                        edges.push_back("    " + node_id + " --> " + child_id);
                        process_expr(variant_expr.expr.get());
                    }
                }
                // Variables have no children
            },
            *expr_ptr);
    };

    // Start processing from the root
    process_expr(&expr);

    // Output nodes first, then edges, then class assignments
    for (const auto& node_def : node_definitions) {
        out << node_def << "\n";
    }
    if (!node_definitions.empty() && !edges.empty()) {
        out << "\n";
    }
    for (const auto& edge : edges) {
        out << edge << "\n";
    }

    // Add CSS class assignments
    if (!class_assignments.empty()) {
        out << "\n";
        for (const auto& class_assign : class_assignments) {
            out << class_assign << "\n";
        }
    }

    // Add CSS class definitions for colors (matching DOT color scheme)
    out << "\n";
    out << "    classDef variable fill:" << expression_constants::variable_color()
        << ",stroke:#333,stroke-width:2px,color:#000\n";
    out << "    classDef andOp fill:" << expression_constants::and_color()
        << ",stroke:#333,stroke-width:2px,color:#000\n";
    out << "    classDef orOp fill:" << expression_constants::or_color()
        << ",stroke:#333,stroke-width:2px,color:#000\n";
    out << "    classDef notOp fill:" << expression_constants::not_color()
        << ",stroke:#333,stroke-width:2px,color:#000\n";
    out << "    classDef xorOp fill:" << expression_constants::xor_color()
        << ",stroke:#333,stroke-width:2px,color:#000\n";
}
