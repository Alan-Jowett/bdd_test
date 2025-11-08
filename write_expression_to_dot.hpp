// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file write_expression_to_dot_v2.hpp
 * @brief Header for the new templated DOT generation function
 *
 * This header provides the write_expression_to_dot function which uses
 * the expression_iterator and dot_graph_generator.hpp system for generating DOT
 * format graphs from expression trees.
 *
 * @author Alan Jowett
 * @date 2025
 * @copyright Copyright (c) 2025 Alan Jowett. Licensed under the MIT License
 */

#pragma once

#include "expression_iterator.hpp"
#include "dot_graph_generator.hpp"
#include <iostream>
#include <string>

/**
 * @brief Writes expression tree as DOT graph using the iterator and template system
 *
 * This is version 2 of write_expression_to_dot that uses the new expression_iterator
 * with the dot_graph_generator.hpp system for generating DOT format graphs. This version
 * produces identical output to the original function but uses the templated approach.
 *
 * Key improvements over the original:
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
 * write_expression_to_dot_v2(*expr, file, "MyExpression");
 * @endcode
 *
 * @note Requires expression_iterator.hpp and dot_graph_generator.hpp
 * @see write_expression_to_dot for the original implementation
 */
inline void write_expression_to_dot(const my_expression& expr, std::ostream& out, 
                                    const std::string& graph_name = "ExpressionTree") {
    // Reset the node ID counter to match original function behavior
    expression_iterator::reset_node_id_counter(0);
    
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