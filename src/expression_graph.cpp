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
#include "graph_render_helpers.hpp"
#include "mermaid_graph_generator.hpp"
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

    dag_walker::walk_dag_topological_order(
        root_iter, [&](const dag_walker::NodeInfo<expression_iterator>& node_info) {
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
        });
}

// ============================================================================
// Mermaid Graph Generation
// ============================================================================

void write_expression_to_mermaid(const my_expression& expr, std::ostream& out,
                                 const std::string& graph_title) {
    // Use the generic expression iterator and the Mermaid generator to
    // produce the same output as the previous custom implementation.
    expression_iterator root_iter(expr);
    mermaid_graph::MermaidConfig config;
    config.graph_title = graph_title;
    config.direction = "TD";  // default top-down
    config.default_node_shape = "circle";
    config.show_frontmatter = true;
    config.show_css_classes = true;
    config.default_css_class = "variable";
    config.node_id_prefix = "N";
    config.node_id_start = 1;
    config.show_node_ids = false;
    config.show_edge_labels = false;
    // Preserve Expression-specific class mapping and definitions to match reference output
    config.label_to_class_map = {
        {"OR", "orOp"}, {"AND", "andOp"}, {"NOT", "notOp"}, {"XOR", "xorOp"}};
    config.class_definitions = {
        {"variable", "fill:lightblue,stroke:#333,stroke-width:2px,color:#000"},
        {"andOp", "fill:lightgreen,stroke:#333,stroke-width:2px,color:#000"},
        {"orOp", "fill:lightcoral,stroke:#333,stroke-width:2px,color:#000"},
        {"notOp", "fill:yellow,stroke:#333,stroke-width:2px,color:#000"},
        {"xorOp", "fill:lightpink,stroke:#333,stroke-width:2px,color:#000"}};

    mermaid_graph::generate_mermaid_graph(root_iter, out, config);
}
