// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file expression_graph.hpp
 * @brief Expression tree graph generation for DOT visualization
 *
 * Provides functions for generating DOT format graphs from expression trees
 * using the templated DOT generation system. Handles expression-specific
 * requirements like operator nodes and variable labeling.
 *
 * @author Alan Jowett
 * @date 2025
 * @copyright Copyright (c) 2025 Alan Jowett. Licensed under the MIT License
 */

#pragma once

#include <iostream>
#include <string>
#include <unordered_set>

#include "expression_types.hpp"

// ============================================================================
// Expression DOT Generation Functions
// ============================================================================

/**
 * @brief Writes expression tree as DOT graph
 *
 * This function generates DOT format graphs from expression trees using an
 * internal iterator system with the enhanced dot_graph_generator.hpp.
 *
 * Generated DOT format includes:
 * - Node properties: operator symbols, variable names, parentheses handling
 * - Tree structure: proper parent-child relationships
 * - Visual styling: different shapes for operators vs variables
 * - Format compatibility: clean DOT output for visualization tools
 *
 * @param expr Expression tree to process
 * @param out Output stream for DOT content
 * @param graph_name Name for the generated DOT graph
 *
 * Example usage:
 * @code
 * write_expression_to_dot(my_expr, std::cout, "ExpressionTree");
 * @endcode
 */
void write_expression_to_dot(const my_expression& expr, std::ostream& out,
                             const std::string& graph_name);

/**
 * @brief Writes expression tree as Mermaid graph
 *
 * This function generates Mermaid format graphs from expression trees for
 * embedding in Markdown documents. Mermaid provides a simple syntax for
 * creating diagrams that can be rendered directly in GitHub, GitLab, and
 * many documentation tools.
 *
 * Generated Mermaid format includes:
 * - Flowchart syntax using TD (top-down) direction
 * - Node styling: rectangles for operators, circles for variables
 * - Clean labels: operator symbols and variable names
 * - Proper parent-child relationships
 *
 * @param expr Expression tree to process
 * @param out Output stream for Mermaid content
 * @param graph_title Title for the generated Mermaid graph
 *
 * Example usage:
 * @code
 * write_expression_to_mermaid(my_expr, std::cout, "Expression Tree");
 * @endcode
 *
 * Example output:
 * @code
 * flowchart TD
 *     N1["AND"]
 *     N2(("a"))
 *     N3(("b"))
 *     N1 --> N2
 *     N1 --> N3
 * @endcode
 */
void write_expression_to_mermaid(const my_expression& expr, std::ostream& out,
                                 const std::string& graph_title = "Expression Tree");

/**
 * @brief Collects variable names using dag_walker traversal
 *
 * Alternative implementation of variable collection that uses the
 * generic dag_walker system for traversal instead of manual recursion.
 *
 * @param expr Expression tree to traverse
 * @param variables Set to store unique variable names (output parameter)
 */
void collect_variables_with_dag_walker(const my_expression& expr,
                                       std::unordered_set<std::string>& variables);
