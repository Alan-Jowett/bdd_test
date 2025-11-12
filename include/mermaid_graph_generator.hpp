// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file mermaid_graph_generator.hpp
 * @brief Specialized template for generating Mermaid flowchart diagrams from tree and DAG
 * structures
 *
 * This header provides Mermaid-specific formatting and output generation for tree and DAG
 * structures. It uses the generic dag_walker.hpp for traversal logic and focuses purely
 * on Mermaid format generation, including node and edge styling.
 *
 * The iterator represents both the current node and the traversal logic,
 * eliminating the need for separate data types or property classes. Iterator methods
 * return Mermaid attribute strings directly for maximum simplicity.
 *
 * @author Alan Jowett
 * @date 2025
 * @copyright Copyright (c) 2025 Alan Jowett. Licensed under the MIT License
 */

#pragma once

#include <algorithm>
#include <concepts>
#include <format>
#include <iostream>
#include <ranges>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "dag_walker.hpp"
#include "graph_iterator_concepts.hpp"

namespace mermaid_graph {

/// @name C++20 Concepts for optional property methods
/// @{

// Node property concepts
template <typename T>
concept has_get_label = requires(T t) {
    { t.get_label() } -> std::convertible_to<std::string>;
};

template <typename T>
concept has_get_shape = requires(T t) {
    { t.get_shape() } -> std::convertible_to<std::string>;
};

template <typename T>
concept has_get_css_class = requires(T t) {
    { t.get_css_class() } -> std::convertible_to<std::string>;
};

template <typename T>
concept has_get_tooltip = requires(T t) {
    { t.get_tooltip() } -> std::convertible_to<std::string>;
};

// Edge property concepts
template <typename T>
concept has_get_edge_label = requires(T t, T child, std::size_t index) {
    { t.get_edge_label(child, index) } -> std::convertible_to<std::string>;
};

template <typename T>
concept has_get_edge_style = requires(T t, T child, std::size_t index) {
    { t.get_edge_style(child, index) } -> std::convertible_to<std::string>;
};

/// @}

/**
 * @brief Core concept for Mermaid graph iterators
 *
 * Defines the minimum interface required for generating Mermaid graphs.
 * All Mermaid generation functions require iterators that satisfy this concept.
 * This extends the base graph iterator interface.
 */
template <typename T>
concept MermaidGraphIterator = BaseGraphIterator<T>;

/**
 * @brief Configuration settings for Mermaid graph generation
 */
struct MermaidConfig {
    std::string graph_title = "Graph";          ///< Title of the Mermaid graph
    std::string direction = "TD";               ///< Graph direction (TD, BT, LR, RL)
    std::string default_node_shape = "circle";  ///< Default shape for nodes (circle, square, etc.)
    bool show_frontmatter = true;               ///< Whether to include YAML frontmatter
    bool show_css_classes = true;               ///< Whether to include CSS class definitions
    std::string default_css_class = "default";  ///< Default CSS class for nodes

    // C++20 spaceship operator for automatic comparison generation
    auto operator<=>(const MermaidConfig& other) const = default;
};

/**
 * @brief Pure iterator-based Mermaid generation using dag_walker for traversal
 *
 * This function generates Mermaid flowchart format from tree/DAG structures by leveraging
 * the dag_walker for all traversal logic and focusing purely on Mermaid formatting.
 *
 * Required iterator interface (enforced by MermaidGraphIterator concept):
 * - std::vector<Iterator> get_children() const
 * - const void* get_node_address() const (for unique node identification)
 * - bool operator==(const Iterator&) const / bool operator!=(const Iterator&) const
 *
 * Optional node property methods (auto-detected via C++20 concepts):
 * - std::string get_label() const
 * - std::string get_shape() const (circle, square, etc.)
 * - std::string get_css_class() const
 * - std::string get_tooltip() const
 *
 * Optional edge property methods (auto-detected via C++20 concepts):
 * - std::string get_edge_label(const Iterator& child, size_t index) const
 * - std::string get_edge_style(const Iterator& child, size_t index) const (solid, dashed, dotted)
 *
 * Optional filtering:
 * - bool should_process() const (handled by dag_walker)
 *
 * @tparam Iterator The iterator type that represents tree/DAG nodes
 * @param root_iterator The root iterator to start traversal from
 * @param out Output stream for the Mermaid content
 * @param config Configuration for the Mermaid graph appearance
 *
 * @requires MermaidGraphIterator<Iterator>
 */
template <MermaidGraphIterator Iterator>
void generate_mermaid_graph(const Iterator& root_iterator, std::ostream& out,
                            const MermaidConfig& config = MermaidConfig()) {
    // Write YAML frontmatter if enabled
    if (config.show_frontmatter) {
        out << "---\n";
        out << "title: " << config.graph_title << "\n";
        out << "---\n";
    }

    // Write flowchart header with direction
    out << "flowchart " << config.direction << "\n";

    // Node ID management for unique Mermaid identifiers
    std::unordered_map<const void*, std::string> node_id_map;
    int next_node_id = 0;

    auto get_node_id = [&](const Iterator& iter) -> std::string {
        const void* key = iter.get_node_address();
        auto it = node_id_map.find(key);
        if (it != node_id_map.end()) {
            return it->second;
        }
        std::string id = std::format("N{}", next_node_id++);
        node_id_map[key] = id;
        return id;
    };

    // Generate node definition from iterator properties
    auto build_node_definition = [&](const Iterator& iter,
                                     const std::string& node_id) -> std::string {
        std::string label;
        if constexpr (has_get_label<Iterator>) {
            label = iter.get_label();
        } else {
            label = node_id;
        }

        // Determine shape and format
        std::string shape = config.default_node_shape;
        if constexpr (has_get_shape<Iterator>) {
            shape = iter.get_shape();
        }

        // Format node based on shape
        if (shape == "square") {
            return std::format("    {}[\"{}\"]\n", node_id, label);
        } else if (shape == "circle") {
            return "    " + node_id + "((\"" + label + "\"))\n";
        } else if (shape == "diamond") {
            return "    " + node_id + "{\"" + label + "\"}\n";
        } else if (shape == "hexagon") {
            return "    " + node_id + "{{\"" + label + "\"}}\n";
        } else {
            // Default to circle
            return "    " + node_id + "((\"" + label + "\"))\n";
        }
    };

    // Generate edge definition from iterator properties
    auto build_edge_definition = [](const Iterator& parent, const Iterator& child, size_t index,
                                    const std::string& parent_id,
                                    const std::string& child_id) -> std::string {
        std::string edge_style = " --> ";
        if constexpr (has_get_edge_style<Iterator>) {
            std::string style = parent.get_edge_style(child, index);
            if (style == "dashed") {
                edge_style = " -.-> ";
            } else if (style == "dotted") {
                edge_style = " -..-> ";
            } else if (style == "thick") {
                edge_style = " ==> ";
            }
            // Default remains solid: " --> "
        }

        std::string edge_label;
        if constexpr (has_get_edge_label<Iterator>) {
            edge_label = parent.get_edge_label(child, index);
        }

        if (!edge_label.empty()) {
            return std::format("    {}{} |\"{}\"| {}\n", parent_id, edge_style, edge_label,
                               child_id);
        } else {
            return std::format("    {}{}{}\n", parent_id, edge_style, child_id);
        }
    };

    // Use dag_walker to collect all unique nodes in topological order
    // This ensures node IDs match the node table indices for consistency
    auto unique_nodes = dag_walker::collect_unique_nodes_topological(root_iterator);
    for (const auto& node : unique_nodes) {
        std::string node_id = get_node_id(node);
        std::string node_def = build_node_definition(node, node_id);
        out << node_def;
    }

    // Use dag_walker to collect all edges and output them
    // For BDD visualization, we need ALL edges including those to revisited nodes
    // (e.g., multiple nodes pointing to the same terminal node)
    dag_walker::WalkConfig edge_config;
    edge_config.collect_all_edges = true;
    auto edges = dag_walker::collect_edges(root_iterator, edge_config);
    
    // Sort edges to match reference output format:
    // 1. By parent node (in topological order, which is the order nodes appear in unique_nodes)
    // 2. For same parent, false edge (child_index=0) before true edge (child_index=1)
    std::unordered_map<const void*, size_t> node_order;
    for (size_t i = 0; i < unique_nodes.size(); ++i) {
        node_order[unique_nodes[i].get_node_address()] = i;
    }
    std::ranges::sort(edges, [&](const auto& e1, const auto& e2) {
        size_t order1 = node_order[e1.parent.get_node_address()];
        size_t order2 = node_order[e2.parent.get_node_address()];
        if (order1 != order2) {
            return order1 < order2;
        }
        // Same parent: false edge (index 0) before true edge (index 1)
        return e1.child_index < e2.child_index;
    });
    
    if (!edges.empty() && !unique_nodes.empty()) {
        out << "\n";
    }

    for (const auto& edge : edges) {
        std::string parent_id = get_node_id(edge.parent);
        std::string child_id = get_node_id(edge.child);
        std::string edge_def =
            build_edge_definition(edge.parent, edge.child, edge.child_index, parent_id, child_id);
        out << edge_def;
    }

    // Collect CSS classes and output them if enabled
    if (config.show_css_classes) {
        std::unordered_map<std::string, std::vector<std::string>> class_to_nodes;

        for (const auto& node : unique_nodes) {
            std::string node_id = get_node_id(node);
            std::string css_class = config.default_css_class;

            if constexpr (has_get_css_class<Iterator>) {
                css_class = node.get_css_class();
            }

            class_to_nodes[css_class].push_back(node_id);
        }

        if (!class_to_nodes.empty()) {
            out << "\n";
            // Output CSS class assignments sorted by class name to match reference format
            // (terminal before bddVariable alphabetically)
            std::vector<std::string> class_names;
            for (const auto& [class_name, _] : class_to_nodes) {
                class_names.push_back(class_name);
            }
            std::ranges::sort(class_names);
            
            for (const auto& class_name : class_names) {
                for (const auto& node_id : class_to_nodes[class_name]) {
                    out << "    class " << node_id << " " << class_name << "\n";
                }
            }

            // Add CSS class definitions for BDD node styling
            out << "\n";
            out << "    classDef bddVariable fill:lightblue,stroke:#333,stroke-width:2px,color:#000\n";
            out << "    classDef terminal fill:lightgray,stroke:#333,stroke-width:2px,color:#000\n";
        }
    }
}

/**
 * @brief Convenience function for Mermaid generation with simple graph title
 *
 * @tparam Iterator The iterator type that represents tree/DAG nodes
 * @param root_iterator The root iterator to start traversal from
 * @param out Output stream for the Mermaid content
 * @param graph_title Title for the generated graph
 *
 * @requires MermaidGraphIterator<Iterator>
 */
template <MermaidGraphIterator Iterator>
void generate_mermaid_graph(const Iterator& root_iterator, std::ostream& out,
                            const std::string& graph_title) {
    MermaidConfig config;
    config.graph_title = graph_title;
    generate_mermaid_graph(root_iterator, out, config);
}

}  // namespace mermaid_graph