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
    config.rankdir = "";             // No rankdir to match original format
    config.font_name = "";           // No font declarations to match original
    config.default_node_shape = "";  // Shapes specified per node
    config.default_node_style = "";  // No default style
    config.default_edge_style = "";  // Edge styles specified per edge
    config.use_bdd_format = true;    // Enable BDD-specific formatting

    // Use the generic DOT graph generator
    dot_graph::generate_dot_graph(root_iter, out, config);
}

void write_teddy_to_mermaid(const teddy::bdd_manager& manager,
                            teddy::bdd_manager::diagram_t diagram,
                            const std::vector<std::string>& variable_names, std::ostream& out,
                            const std::string& graph_title) {
    // Create iterator for the root node
    teddy_iterator root_iter(diagram.unsafe_get_root(), &variable_names);

    // Configure Mermaid generator for BDD-specific output (match TeDDy numbering and classes)
    mermaid_graph::MermaidConfig config;
    config.graph_title = graph_title;
    config.direction = "TD";
    config.default_node_shape = "";
    config.show_frontmatter = true;
    config.show_css_classes = true;
    config.node_id_prefix = "N";
    config.node_id_start = 0;

    // Add BDD-specific class definitions to match legacy output
    config.class_definitions.push_back(
        {"bddVariable", "fill:lightblue,stroke:#333,stroke-width:2px,color:#000"});
    config.class_definitions.push_back(
        {"terminal", "fill:lightgray,stroke:#333,stroke-width:2px,color:#000"});

    mermaid_graph::generate_mermaid_graph(root_iter, out, config);
}

std::vector<teddy::bdd_manager::diagram_t::node_t*> collect_teddy_nodes_topological(
    teddy::bdd_manager::diagram_t diagram, const std::vector<std::string>& variable_names) {
    using node_t = teddy::bdd_manager::diagram_t::node_t;

    // Create TeDDy iterator and collect nodes in topological order
    teddy_iterator root_iter(diagram.unsafe_get_root(), &variable_names);
    std::vector<teddy_iterator> nodes_in_order = dag_walker::collect_nodes_topological(root_iter);

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
