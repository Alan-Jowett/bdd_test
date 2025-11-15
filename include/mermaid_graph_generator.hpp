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
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "dag_walker.hpp"
#include "graph_iterator_concepts.hpp"
#include "node_id_allocator.hpp"

namespace mermaid_graph {

/// @name C++20 Concepts for optional property methods
/// @{

// Node property concepts
/**
 * @brief Concept for iterators that provide node labels
 * @tparam T The iterator type to check
 *
 * This concept verifies that the iterator can provide string labels
 * for nodes in the graph, essential for generating readable Mermaid diagrams.
 */
template <typename T>
concept has_get_label = requires(T t) {
    { t.get_label() } -> std::convertible_to<std::string>;
};

/**
 * @brief Concept for iterators that provide node shapes
 * @tparam T The iterator type to check
 *
 * Enables custom node shapes in Mermaid diagrams (circle, rect, diamond, etc.)
 * for visual distinction of different node types.
 */
template <typename T>
concept has_get_shape = requires(T t) {
    { t.get_shape() } -> std::convertible_to<std::string>;
};

/**
 * @brief Concept for iterators that provide CSS classes
 * @tparam T The iterator type to check
 *
 * Allows custom styling through CSS classes, enabling rich visual
 * customization in web-based Mermaid diagram renderings.
 */
template <typename T>
concept has_get_css_class = requires(T t) {
    { t.get_css_class() } -> std::convertible_to<std::string>;
};

/**
 * @brief Concept for iterators that provide tooltips
 * @tparam T The iterator type to check
 *
 * Enables interactive tooltips in Mermaid diagrams, providing additional
 * information when users hover over nodes.
 */
template <typename T>
concept has_get_tooltip = requires(T t) {
    { t.get_tooltip() } -> std::convertible_to<std::string>;
};

// Edge property concepts
/**
 * @brief Concept for iterators that provide edge labels
 * @tparam T The iterator type to check
 *
 * Enables custom labels on edges between nodes, useful for showing
 * decision conditions or probability values in BDD structures.
 */
template <typename T>
concept has_get_edge_label = requires(T t, T child, std::size_t index) {
    { t.get_edge_label(child, index) } -> std::convertible_to<std::string>;
};

/**
 * @brief Concept for iterators that provide edge styles
 * @tparam T The iterator type to check
 *
 * Allows customization of edge appearance (solid, dashed, dotted, etc.)
 * for visual distinction of different edge types in Mermaid diagrams.
 */
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
    static_assert(
        MermaidGraphIterator<Iterator>,
        "Iterator must satisfy MermaidGraphIterator concept for Mermaid graph generation");

    // Write YAML frontmatter if enabled
    if (config.show_frontmatter) {
        out << "---\n";
        out << "title: " << config.graph_title << "\n";
        out << "---\n";
    }

    // Write flowchart header with direction
    out << "flowchart " << config.direction << "\n";

    // Node ID management for unique Mermaid identifiers (shared allocator)
    // Start at 0 for BDD diagrams to match reference numbering
    graph_common::node_id_allocator id_alloc("N", 0);

    auto get_node_id = [&](const Iterator& iter) -> std::string {
        const void* key = iter.get_node_address();
        return id_alloc.get_id(key);
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

    // Use dag_walker to collect all unique nodes in topological order and output them
    auto unique_nodes = dag_walker::collect_unique_nodes_topological(root_iterator);
    for (const auto& node : unique_nodes) {
        std::string node_id = get_node_id(node);
        std::string node_def = build_node_definition(node, node_id);
        out << node_def;
    }

    // Collect and output edges to match original TeDDy ordering
    // TeDDy outputs edges grouped by parent, in ascending node ID order (not topological)
    dag_walker::WalkConfig edge_config;
    edge_config.collect_all_edges = true;
    auto edges = dag_walker::collect_edges(root_iterator, edge_config);
    if (!edges.empty() && !unique_nodes.empty()) {
        out << "\n";
    }

    // Group edges by their parent node and collect parent info
    std::unordered_map<const void*, std::vector<dag_walker::EdgeInfo<Iterator>>> edges_by_parent;
    std::vector<std::pair<std::string, const void*>> parent_ids_and_keys;

    for (const auto& edge : edges) {
        const void* parent_key = edge.parent.get_node_address();
        edges_by_parent[parent_key].push_back(edge);
    }

    // Collect parent node IDs to sort them
    for (const auto& [parent_key, _] : edges_by_parent) {
        // Find the parent iterator to get its ID
        auto it = std::find_if(
            unique_nodes.begin(), unique_nodes.end(),
            [parent_key](const auto& node) { return node.get_node_address() == parent_key; });
        if (it != unique_nodes.end()) {
            std::string parent_id = get_node_id(*it);
            parent_ids_and_keys.push_back({parent_id, parent_key});
        }
    }

    // Sort by node ID numerically to get consistent TeDDy ordering
    std::sort(parent_ids_and_keys.begin(), parent_ids_and_keys.end(),
              [](const auto& a, const auto& b) {
                  // Extract numeric part from node ID (e.g., "N2" -> 2)
                  int num_a = std::stoi(a.first.substr(1));
                  int num_b = std::stoi(b.first.substr(1));
                  return num_a < num_b;
              });

    // Output edges in sorted parent ID order
    for (const auto& [parent_id, parent_key] : parent_ids_and_keys) {
        auto& parent_edges = edges_by_parent[parent_key];
        // Sort edges within each parent: false edge first, then true edge
        std::sort(parent_edges.begin(), parent_edges.end(),
                  [](const auto& a, const auto& b) { return a.child_index < b.child_index; });

        for (const auto& edge : parent_edges) {
            std::string parent_id_str = get_node_id(edge.parent);
            std::string child_id = get_node_id(edge.child);
            std::string edge_def = build_edge_definition(edge.parent, edge.child, edge.child_index,
                                                         parent_id_str, child_id);
            out << edge_def;
        }
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

            // Sort CSS class assignments by node ID for consistent output
            std::vector<std::pair<std::string, std::string>> sorted_class_assignments;
            for (const auto& [class_name, node_ids] : class_to_nodes) {
                std::transform(node_ids.begin(), node_ids.end(),
                               std::back_inserter(sorted_class_assignments),
                               [&class_name](const std::string& node_id) {
                                   return std::make_pair(node_id, class_name);
                               });
            }
            std::sort(sorted_class_assignments.begin(), sorted_class_assignments.end(),
                      [](const auto& a, const auto& b) {
                          // Sort by node ID numerically (e.g., N1, N2, N3...)
                          int num_a = std::stoi(a.first.substr(1));
                          int num_b = std::stoi(b.first.substr(1));
                          return num_a < num_b;
                      });

            for (const auto& [node_id, class_name] : sorted_class_assignments) {
                out << "    class " << node_id << " " << class_name << "\n";
            }
        }

        // Add CSS class definitions for BDD-specific styling (matching TeDDy format)
        bool has_bdd_variable = class_to_nodes.find("bddVariable") != class_to_nodes.end();
        bool has_terminal = class_to_nodes.find("terminal") != class_to_nodes.end();

        if (has_bdd_variable || has_terminal) {
            out << "\n";
            if (has_bdd_variable) {
                out << "    classDef bddVariable "
                       "fill:lightblue,stroke:#333,stroke-width:2px,color:#000\n";
            }
            if (has_terminal) {
                out << "    classDef terminal "
                       "fill:lightgray,stroke:#333,stroke-width:2px,color:#000\n";
            }
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
    static_assert(
        MermaidGraphIterator<Iterator>,
        "Iterator must satisfy MermaidGraphIterator concept for Mermaid graph generation");

    MermaidConfig config;
    config.graph_title = graph_title;
    generate_mermaid_graph(root_iterator, out, config);
}

}  // namespace mermaid_graph
