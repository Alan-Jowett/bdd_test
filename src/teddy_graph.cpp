// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file teddy_graph.cpp
 * @brief TeDDy BDD graph generation using generic template systems implementation
 *
 * Implementation of TeDDy BDD graph generation functions that use the generic
 * dot_graph_generator.hpp and mermaid_graph_generator.hpp templates.
 * Provides identical output to BDD-specific implementations.
 *
 * @author Alan Jowett
 * @date 2025
 * @copyright Copyright (c) 2025 Alan Jowett. Licensed under the MIT License
 */

#include "teddy_graph.hpp"

#include <format>
#include <iostream>
#include <ranges>
#include <unordered_map>

#include "dag_walker.hpp"
#include "dot_graph_generator.hpp"
#include "mermaid_graph_generator.hpp"
#include "teddy_iterator.hpp"

// ============================================================================
// Exported functions (global namespace)
// ============================================================================

void write_teddy_to_dot(const teddy::bdd_manager& manager, teddy::bdd_manager::diagram_t diagram,
                        const std::vector<std::string>& variable_names, std::ostream& out,
                        const std::string& graph_name) {
    // Create an iterator from the BDD root
    teddy_iterator root_iter(diagram.unsafe_get_root(), &variable_names);

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

void write_teddy_to_mermaid(const teddy::bdd_manager& manager,
                            teddy::bdd_manager::diagram_t diagram,
                            const std::vector<std::string>& variable_names, std::ostream& out,
                            const std::string& graph_title) {
    using node_t = teddy::bdd_manager::diagram_t::node_t;

    out << "---\n";
    out << "title: " << graph_title << "\n";
    out << "---\n";
    out << "flowchart TD\n";

    // Get all nodes using the same topological collection as the original
    std::vector<node_t*> nodes = collect_teddy_nodes_topological(diagram, variable_names);

    // Create node ID mapping and class assignments
    std::unordered_map<node_t*, std::string> node_to_id;
    std::vector<std::string> class_assignments;

    for (size_t i = 0; i < nodes.size(); ++i) {
        node_to_id[nodes[i]] = "N" + std::to_string(i);
    }

    // Generate node definitions
    for (node_t* node : nodes) {
        std::string node_id = node_to_id[node];

        if (node->is_terminal()) {
            // Terminals as squares: N1["0"] or N1["1"]
            out << "    " << node_id << "[\"" << node->get_value() << "\"]\n";
            // Add CSS class for terminals
            class_assignments.push_back("    class " + node_id + " terminal");
        } else {
            // Variables as circles: N1(("var_name"))
            int var_index = node->get_index();
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
                                 [](const node_t* node) { return !node->is_terminal(); });

    if (!nodes.empty() && has_edges) {
        out << "\n";
    }

    // Generate edges
    for (node_t* node : nodes) {
        if (!node->is_terminal()) {
            std::string node_id = node_to_id[node];

            node_t* false_child = node->get_son(0);
            node_t* true_child = node->get_son(1);

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

std::vector<teddy::bdd_manager::diagram_t::node_t*> collect_teddy_nodes_topological(
    teddy::bdd_manager::diagram_t diagram, const std::vector<std::string>& variable_names) {
    using node_t = teddy::bdd_manager::diagram_t::node_t;

    // Create TeDDy iterator and collect nodes in topological order
    teddy_iterator root_iter(diagram.unsafe_get_root(), &variable_names);
    std::vector<teddy_iterator> nodes_in_order =
        dag_walker::collect_unique_nodes_topological(root_iter);

    // Convert iterator vector to node pointer vector using type-safe access
    std::vector<node_t*> result;
    result.reserve(nodes_in_order.size());

    std::ranges::transform(nodes_in_order, std::back_inserter(result),
                           [](const auto& node_iter) { return node_iter.get_node(); });

    return result;
}

void write_teddy_nodes_to_stream(const teddy::bdd_manager& manager,
                                 teddy::bdd_manager::diagram_t diagram,
                                 const std::vector<std::string>& variable_names, std::ostream& out,
                                 bool include_headers) {
    // Create TeDDy iterator for the root node
    teddy_iterator root_iter(diagram.unsafe_get_root(), &variable_names);

    // Configure the table generator with BDD Node Table title to match legacy format
    node_table::TextTableConfig config(include_headers, "BDD Node Table (topological ordering)");

    // Use the generic text table generator
    node_table::generate_text_table(root_iter, out, config);
}

void write_teddy_nodes_to_markdown(const teddy::bdd_manager& manager,
                                   teddy::bdd_manager::diagram_t diagram,
                                   const std::vector<std::string>& variable_names,
                                   std::ostream& out) {
    // Create TeDDy iterator for the root node
    teddy_iterator root_iter(diagram.unsafe_get_root(), &variable_names);

    // Configure the table generator
    node_table::MarkdownTableConfig config;

    // Use the generic markdown table generator
    node_table::generate_markdown_table(root_iter, out, config);
}