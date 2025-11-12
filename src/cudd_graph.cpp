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
#include <unordered_map>

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
    out << "---\n";
    out << "title: " << graph_title << "\n";
    out << "---\n";
    out << "flowchart TD\n";

    // Get all nodes using the same topological collection as TeDDy
    std::vector<DdNode*> nodes = collect_cudd_nodes_topological(cudd_manager, bdd, variable_names);

    // Create node ID mapping and class assignments
    std::unordered_map<DdNode*, std::string> node_to_id;
    std::vector<std::string> class_assignments;

    for (size_t i = 0; i < nodes.size(); ++i) {
        node_to_id[nodes[i]] = "N" + std::to_string(i);
    }

    // Generate node definitions
    for (DdNode* node : nodes) {
        std::string node_id = node_to_id[node];

        if (Cudd_IsConstant(node)) {
            // Terminals as squares: N1["0"] or N1["1"]
            // Handle complement bit correctly like cudd_iterator does
            int value = Cudd_V(node);
            if (Cudd_IsComplement(node)) {
                value = !value;
            }
            out << "    " << node_id << "[\"" << value << "\"]\n";
            // Add CSS class for terminals
            class_assignments.push_back("    class " + node_id + " terminal");
        } else {
            // Variables as circles: N1(("var_name"))
            int var_index = Cudd_NodeReadIndex(node);
            std::string var_name = (var_index < variable_names.size())
                                       ? variable_names[var_index]
                                       : "x" + std::to_string(var_index);
            out << "    " << node_id << "((\"" << var_name << "\"))\n";
            // Add CSS class for variables
            class_assignments.push_back("    class " + node_id + " bddVariable");
        }
    }

    // Add separator between nodes and edges if we have both
    bool has_edges = std::any_of(nodes.begin(), nodes.end(),
                                 [](DdNode* node) { return !Cudd_IsConstant(node); });

    if (!nodes.empty() && has_edges) {
        out << "\n";
    }

    // Generate edges
    for (DdNode* node : nodes) {
        if (!Cudd_IsConstant(node)) {
            std::string node_id = node_to_id[node];

            DdNode* false_child = Cudd_E(node);
            DdNode* true_child = Cudd_T(node);

            // False edge (dashed): N1 -.-> N2
            if (false_child && node_to_id.find(false_child) != node_to_id.end()) {
                out << "    " << node_id << " -.-> " << node_to_id[false_child] << "\n";
            }

            // True edge (solid): N1 --> N2
            if (true_child && node_to_id.find(true_child) != node_to_id.end()) {
                out << "    " << node_id << " --> " << node_to_id[true_child] << "\n";
            }
        }
    }

    // Add CSS class assignments
    if (!class_assignments.empty()) {
        out << "\n";
        for (const auto& class_assign : class_assignments) {
            out << class_assign << "\n";
        }
    }

    // Add CSS class definitions for BDD node colors
    out << "\n";
    out << "    classDef bddVariable fill:lightblue,stroke:#333,stroke-width:2px,color:#000\n";
    out << "    classDef terminal fill:lightgray,stroke:#333,stroke-width:2px,color:#000\n";
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
