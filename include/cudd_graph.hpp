// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file cudd_graph.hpp
 * @brief CUDD BDD graph generation using generic template systems
 *
 * Provides iterator interface and graph generation functions for visualizing
 * Binary Decision Diagrams using the generic dot_graph_generator.hpp and
 * mermaid_graph_generator.hpp templates. These functions produce high-quality
 * output for CUDD BDDs using the flexible template system.
 *
 * @author Alan Jowett
 * @date 2025
 * @copyright Copyright (c) 2025 Alan Jowett. Licensed under the MIT License
 */

#pragma once

#include <cudd/cudd.h>

#include <cudd/cuddObj.hh>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "dot_graph_generator.hpp"
#include "mermaid_graph_generator.hpp"
#include "node_table_generator.hpp"

// ============================================================================
// CUDD BDD Graph Generation Functions (Using Generic Templates)
// ============================================================================

/**
 * @brief Writes CUDD BDD as DOT graph using the generic template system
 *
 * This function generates DOT format graphs from CUDD Binary Decision Diagrams
 * using the generic dot_graph_generator.hpp template. It provides high-quality
 * visualization for CUDD BDDs using the flexible template system.
 *
 * Generated DOT format includes:
 * - Node properties: variable labels, square shapes for terminals, circles for variables
 * - Edge properties: dashed lines for false edges, solid lines for true edges
 * - Tooltips: variable indices and terminal values
 * - Proper DAG handling: no duplicate nodes, all edges preserved
 *
 * @param cudd_manager CUDD manager containing the BDD
 * @param bdd CUDD BDD to process
 * @param variable_names Vector of variable names for labeling
 * @param out Output stream for DOT content
 * @param graph_name Name for the generated DOT graph (default: "CUDD_BDD")
 */
void write_cudd_to_dot(const Cudd& cudd_manager, const BDD& bdd,
                       const std::vector<std::string>& variable_names, std::ostream& out,
                       const std::string& graph_name = "CUDD_BDD");

/**
 * @brief Writes CUDD BDD as Mermaid graph using the generic template system
 *
 * This function generates Mermaid format graphs from CUDD Binary Decision Diagrams
 * using the generic mermaid_graph_generator.hpp template. It provides high-quality
 * visualization for CUDD BDDs using the flexible template system.
 *
 * Generated Mermaid format includes:
 * - Flowchart syntax using TD (top-down) direction
 * - Node styling: circles for variables, squares for terminals
 * - Edge styling: solid lines for true edges, dashed lines for false edges
 * - Clean labels: variable names and terminal values (0/1)
 * - Proper DAG handling: shared nodes represented correctly
 * - CSS class styling for professional appearance
 *
 * @param cudd_manager CUDD manager containing the BDD
 * @param bdd CUDD BDD to process
 * @param variable_names Vector of variable names for labeling
 * @param out Output stream for Mermaid content
 * @param graph_title Title for the generated Mermaid graph (default: "BDD")
 */
void write_cudd_to_mermaid(const Cudd& cudd_manager, const BDD& bdd,
                           const std::vector<std::string>& variable_names, std::ostream& out,
                           const std::string& graph_title = "BDD");

/**
 * @brief Collect CUDD BDD nodes in topological order using dag_walker
 *
 * Generic implementation that uses the dag_walker system to collect CUDD BDD nodes
 * in post-order (topological) traversal order. This uses the generic template
 * system for consistency with the other functions.
 *
 * @param cudd_manager CUDD manager containing the BDD
 * @param bdd CUDD BDD to traverse
 * @param variable_names Vector of variable names for labeling
 * @return Vector of CUDD node pointers in topological order (children before parents)
 */
std::vector<DdNode*> collect_cudd_nodes_topological(const Cudd& cudd_manager, const BDD& bdd,
                                                    const std::vector<std::string>& variable_names);

/**
 * @brief Writes CUDD BDD node table to output stream using generic template
 *
 * This function generates a comprehensive table showing the structure of a CUDD BDD
 * using the generic node_table_generator.hpp template. It provides high-quality
 * table output for CUDD BDDs using the flexible template system.
 *
 * Generated table includes:
 * - Node indices in topological order (parents before children)
 * - Variable names and assignments
 * - Child relationship information
 * - Terminal node identification and values
 * - Optional headers and footers
 *
 * @param cudd_manager CUDD manager containing the BDD
 * @param bdd CUDD BDD to analyze
 * @param variable_names Vector of variable names for display
 * @param out Output stream to write the table to
 * @param include_headers Whether to include descriptive headers and footers (default: true)
 */
void write_cudd_nodes_to_stream(const Cudd& cudd_manager, const BDD& bdd,
                                const std::vector<std::string>& variable_names, std::ostream& out,
                                bool include_headers = true);

/**
 * @brief Writes CUDD BDD node table in Markdown format using generic template
 *
 * This function generates a Markdown table showing CUDD BDD nodes with their
 * relationships using the generic node_table_generator.hpp template. It provides
 * high-quality Markdown output for CUDD BDDs using the template system.
 *
 * Generated Markdown table includes:
 * - Proper Markdown table formatting with headers
 * - Node indices in topological order
 * - Variable names and relationships
 * - Child index information
 * - Terminal node identification
 *
 * @param cudd_manager CUDD manager containing the BDD
 * @param bdd CUDD BDD to analyze
 * @param variable_names Vector of variable names for display
 * @param out Output stream to write the Markdown table to
 */
void write_cudd_nodes_to_markdown(const Cudd& cudd_manager, const BDD& bdd,
                                  const std::vector<std::string>& variable_names,
                                  std::ostream& out);