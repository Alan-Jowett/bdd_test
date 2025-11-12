// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file cudd_graph.cpp
 * @brief CUDD BDD graph generation using generic template systems implementation
 *
 * Implementation of CUDD BDD graph generation functions that use the generic
 * dot_graph_generator.hpp and mermaid_graph_generator.hpp templates.
 * Provides high-quality visualization for CUDD BDDs.
 *
 * @author Alan Jowett
 * @date 2025
 * @copyright Copyright (c) 2025 Alan Jowett. Licensed under the MIT License
 */

#include "cudd_graph.hpp"

#include <iostream>
#include <ranges>

#include "cudd_iterator.hpp"
#include "dag_walker.hpp"
#include "dot_graph_generator.hpp"
#include "mermaid_graph_generator.hpp"

// ============================================================================
// Public Function Implementations
// ============================================================================

void write_cudd_to_dot(const Cudd& cudd_manager, const BDD& bdd,
                       const std::vector<std::string>& variable_names, std::ostream& out,
                       const std::string& graph_name) {
    // Create iterator for the root node
    cudd_iterator root_iter(cudd_manager, bdd.getNode(), &variable_names);

    // Configure the generic DOT generator for BDD-specific format
    dot_graph::DotConfig config;
    config.graph_name = graph_name;
    config.rankdir = "";              // No rankdir to match original format
    config.font_name = "";            // No font declarations to match original
    config.default_node_shape = "";   // Shapes specified per node
    config.default_node_style = "";   // No default style
    config.default_edge_style = "";   // Edge styles specified per edge
    config.show_node_ids = false;     // Don't show internal IDs
    config.use_bdd_format = true;     // Enable BDD-specific formatting
    config.collect_all_edges = true;  // Collect all edges including to revisited nodes

    // Use the generic DOT graph generator
    dot_graph::generate_dot_graph(root_iter, out, config);
}

void write_cudd_to_mermaid(const Cudd& cudd_manager, const BDD& bdd,
                           const std::vector<std::string>& variable_names, std::ostream& out,
                           const std::string& graph_title) {
    // Create iterator for the root node
    cudd_iterator root_iter(cudd_manager, bdd.getNode(), &variable_names);

    // Use the generic Mermaid generator
    mermaid_graph::generate_mermaid_graph(root_iter, out, graph_title);
}

std::vector<DdNode*> collect_cudd_nodes_topological(
    const Cudd& cudd_manager, const BDD& bdd, const std::vector<std::string>& variable_names) {
    // Create iterator for the root node
    cudd_iterator root_iter(cudd_manager, bdd.getNode(), &variable_names);

    // Collect nodes using DAG walker
    std::vector<cudd_iterator> nodes_in_order =
        dag_walker::collect_unique_nodes_topological(root_iter);

    // Convert iterator vector to node pointer vector
    std::vector<DdNode*> result;
    result.reserve(nodes_in_order.size());

    std::ranges::transform(nodes_in_order, std::back_inserter(result),
                           [](const auto& node_iter) { return node_iter.get_node(); });

    return result;
}

void write_cudd_nodes_to_stream(const Cudd& cudd_manager, const BDD& bdd,
                                const std::vector<std::string>& variable_names, std::ostream& out,
                                bool include_headers) {
    // Create iterator for the root node
    cudd_iterator root_iter(cudd_manager, bdd.getNode(), &variable_names);

    // Configure the table generator with CUDD-specific title
    node_table::TextTableConfig config(include_headers,
                                       "CUDD BDD Node Table (topological ordering)");

    // Use the generic text table generator
    node_table::generate_text_table(root_iter, out, config);
}

void write_cudd_nodes_to_markdown(const Cudd& cudd_manager, const BDD& bdd,
                                  const std::vector<std::string>& variable_names,
                                  std::ostream& out) {
    // Create iterator for the root node
    cudd_iterator root_iter(cudd_manager, bdd.getNode(), &variable_names);

    // Use the generic Markdown table generator
    node_table::generate_markdown_table(root_iter, out);
}
