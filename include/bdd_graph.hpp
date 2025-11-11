// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file bdd_graph.hpp
 * @brief BDD (Binary Decision Diagram) graph generation for DOT visualization
 *
 * Provides iterator interface and DOT generation functions for visualizing
 * Binary Decision Diagrams using the templated DOT generation system.
 * Handles BDD-specific requirements like shared nodes and edge styling.
 *
 * @author Alan Jowett
 * @date 2025
 * @copyright Copyright (c) 2025 Alan Jowett. Licensed under the MIT License
 */

#pragma once

#include <iostream>
#include <libteddy/core.hpp>
#include <string>
#include <vector>

// ============================================================================
// BDD DOT Generation Functions
// ============================================================================

/**
 * @brief Writes BDD as DOT graph using the enhanced templated system
 *
 * This function generates DOT format graphs from Binary Decision Diagrams
 * using an internal iterator system with the enhanced dot_graph_generator.hpp.
 *
 * The enhanced DOT generator properly handles DAGs with shared nodes, preventing
 * duplicate node definitions while preserving all edge relationships. This makes
 * it suitable for production use with BDD structures.
 *
 * Generated DOT format includes:
 * - Node properties: variable labels, square shapes for terminals, circles for variables
 * - Edge properties: dashed lines for false edges, solid lines for true edges
 * - Tooltips: variable indices and terminal values
 * - Proper DAG handling: no duplicate nodes, all edges preserved
 * - Format compatibility: matches original format style (interleaved nodes/edges)
 *
 * Note: Node ID assignment may differ from the original manual implementation due to
 * different traversal algorithms, but the generated graph represents the same BDD
 * structure and is functionally equivalent.
 *
 * @param manager BDD manager (currently unused but kept for compatibility)
 * @param diagram BDD diagram to process
 * @param variable_names Vector of variable names for labeling
 * @param out Output stream for DOT content
 * @param graph_name Name for the generated DOT graph (default: "DD")
 *
 * Example usage:
 * @code
 * write_bdd_to_dot(manager, diagram, variable_names, std::cout);
 * @endcode
 */
void write_bdd_to_dot(const teddy::bdd_manager& manager, teddy::bdd_manager::diagram_t diagram,
                      const std::vector<std::string>& variable_names, std::ostream& out,
                      const std::string& graph_name = "DD");

/**
 * @brief Writes BDD as Mermaid graph
 *
 * This function generates Mermaid format graphs from Binary Decision Diagrams
 * for embedding in Markdown documents. Mermaid provides clean, readable
 * diagrams that render directly in GitHub, GitLab, and documentation tools.
 *
 * Generated Mermaid format includes:
 * - Flowchart syntax using TD (top-down) direction
 * - Node styling: circles for variables, squares for terminals
 * - Edge styling: solid lines for true edges, dashed lines for false edges
 * - Clean labels: variable names and terminal values (0/1)
 * - Proper DAG handling: shared nodes represented correctly
 *
 * @param manager BDD manager (currently unused but kept for compatibility)
 * @param diagram BDD diagram to process
 * @param variable_names Vector of variable names for labeling
 * @param out Output stream for Mermaid content
 * @param graph_title Title for the generated Mermaid graph
 *
 * Example usage:
 * @code
 * write_bdd_to_mermaid(manager, diagram, variable_names, std::cout, "BDD");
 * @endcode
 *
 * Example output:
 * @code
 * flowchart TD
 *     N0(("x0"))
 *     N1["0"]
 *     N2["1"]
 *     N0 -.-> N1
 *     N0 --> N2
 * @endcode
 */
void write_bdd_to_mermaid(const teddy::bdd_manager& manager, teddy::bdd_manager::diagram_t diagram,
                          const std::vector<std::string>& variable_names, std::ostream& out,
                          const std::string& graph_title = "BDD");

/**
 * @brief Collect BDD nodes in topological order using dag_walker
 *
 * Alternative implementation that uses the generic dag_walker system
 * to collect BDD nodes in post-order (topological) traversal order.
 * This replaces manual stack-based traversal with the generic walker.
 *
 * @param diagram BDD diagram to traverse
 * @param variable_names Vector of variable names for labeling
 * @return Vector of node pointers in topological order (children before parents)
 */
std::vector<teddy::bdd_manager::diagram_t::node_t*> collect_bdd_nodes_topological(
    teddy::bdd_manager::diagram_t diagram, const std::vector<std::string>& variable_names);
