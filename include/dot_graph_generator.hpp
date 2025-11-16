// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file dot_graph_generator.hpp
 * @brief Specialized template for generating Graphviz DOT diagrams from tree and DAG structures
 *
 * This header provides DOT-specific formatting and output generation for tree and DAG
 * structures. It uses the generic dag_walker.hpp for traversal logic and focuses purely
 * on DOT format generation, including node and edge styling.
 *
 * The iterator represents both the current node and the traversal logic,
 * eliminating the need for separate data types or property classes. Iterator methods
 * return DOT attribute strings directly for maximum simplicity.
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
#include "graph_render_helpers.hpp"
#include "node_id_allocator.hpp"

namespace dot_graph {

/// @name C++20 Concepts for optional property methods
/// @{

// Node property concepts
/**
 * @brief Concept for iterators that provide node labels
 * @tparam T The iterator type to check
 *
 * This concept verifies that the iterator can provide string labels
 * for nodes in DOT graphs, essential for generating readable GraphViz output.
 */
template <typename T>
concept has_get_label = requires(T t) {
    { t.get_label() } -> std::convertible_to<std::string>;
};

/**
 * @brief Concept for iterators that provide node shapes
 * @tparam T The iterator type to check
 *
 * Enables custom node shapes in DOT graphs (box, ellipse, diamond, etc.)
 * for visual distinction of different node types in GraphViz output.
 */
template <typename T>
concept has_get_shape = requires(T t) {
    { t.get_shape() } -> std::convertible_to<std::string>;
};

/**
 * @brief Concept for iterators that provide node styles
 * @tparam T The iterator type to check
 *
 * Allows specification of node appearance attributes like filled,
 * bold, dashed, etc. for GraphViz rendering customization.
 */
template <typename T>
concept has_get_style = requires(T t) {
    { t.get_style() } -> std::convertible_to<std::string>;
};

/**
 * @brief Concept for iterators that provide node fill colors
 * @tparam T The iterator type to check
 *
 * Enables custom fill colors for nodes, supporting both named colors
 * and hex color codes for GraphViz diagram customization.
 */
template <typename T>
concept has_get_fillcolor = requires(T t) {
    { t.get_fillcolor() } -> std::convertible_to<std::string>;
};

/**
 * @brief Concept for iterators that provide font colors
 * @tparam T The iterator type to check
 *
 * Allows customization of text color within nodes, supporting
 * both named colors and hex codes for optimal readability.
 */
template <typename T>
concept has_get_fontcolor = requires(T t) {
    { t.get_fontcolor() } -> std::convertible_to<std::string>;
};

/**
 * @brief Concept for iterators that provide tooltips
 * @tparam T The iterator type to check
 *
 * Enables interactive tooltips in SVG output from GraphViz,
 * providing additional information for interactive viewing.
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
 * Enables custom labels on edges, useful for showing decision
 * paths, weights, or conditions in GraphViz BDD visualizations.
 */
template <typename T>
concept has_get_edge_label = requires(T t, T child, std::size_t index) {
    { t.get_edge_label(child, index) } -> std::convertible_to<std::string>;
};

/**
 * @brief Concept for iterators that provide edge styles
 * @tparam T The iterator type to check
 *
 * Allows customization of edge appearance (solid, dashed, dotted, bold)
 * for visual distinction in GraphViz output.
 */
template <typename T>
concept has_get_edge_style = requires(T t, T child, std::size_t index) {
    { t.get_edge_style(child, index) } -> std::convertible_to<std::string>;
};

/**
 * @brief Concept for iterators that provide edge colors
 * @tparam T The iterator type to check
 *
 * Enables custom colors for edges in GraphViz diagrams,
 * supporting both named colors and hex color codes.
 */
template <typename T>
concept has_get_edge_color = requires(T t, T child, std::size_t index) {
    { t.get_edge_color(child, index) } -> std::convertible_to<std::string>;
};

/**
 * @brief Concept for iterators that provide edge font colors
 * @tparam T The iterator type to check
 *
 * Allows customization of text color for edge labels in GraphViz,
 * enhancing readability of edge annotations.
 */
template <typename T>
concept has_get_edge_fontcolor = requires(T t, T child, std::size_t index) {
    { t.get_edge_fontcolor(child, index) } -> std::convertible_to<std::string>;
};

/// @}

/**
 * @brief Core concept for DOT graph iterators
 *
 * Defines the minimum interface required for generating DOT graphs.
 * All DOT generation functions require iterators that satisfy this concept.
 * This extends the base graph iterator interface.
 */
template <typename T>
concept DotGraphIterator = BaseGraphIterator<T>;

/**
 * @brief Configuration settings for DOT graph generation
 */
struct DotConfig {
    std::string graph_name = "Graph";           ///< Name of the DOT graph
    std::string rankdir = "TB";                 ///< Graph direction (TB, BT, LR, RL)
    std::string font_name = "Arial";            ///< Font family for nodes and edges
    std::string default_node_shape = "circle";  ///< Default shape for nodes
    std::string default_node_style = "filled";  ///< Default style for nodes
    std::string default_edge_style = "solid";   ///< Default style for edges
    bool use_bdd_format = false;  ///< Whether to use BDD-specific grouped shape format

    // C++20 spaceship operator for automatic comparison generation
    auto operator<=>(const DotConfig& other) const = default;
};

/**
 * @brief Pure iterator-based DOT generation using dag_walker for traversal
 *
 * This function generates Graphviz DOT format from tree/DAG structures by leveraging
 * the dag_walker for all traversal logic and focusing purely on DOT formatting.
 *
 * Required iterator interface (enforced by DotGraphIterator concept):
 * - std::vector<Iterator> get_children() const
 * - const void* get_node_address() const (for unique node identification)
 * - bool operator==(const Iterator&) const / bool operator!=(const Iterator&) const
 *
 * Optional node property methods (auto-detected via C++20 concepts):
 * - std::string get_label() const
 * - std::string get_shape() const
 * - std::string get_style() const
 * - std::string get_fillcolor() const
 * - std::string get_fontcolor() const
 * - std::string get_tooltip() const
 *
 * Optional edge property methods (auto-detected via C++20 concepts):
 * - std::string get_edge_label(const Iterator& child, size_t index) const
 * - std::string get_edge_style(const Iterator& child, size_t index) const
 * - std::string get_edge_color(const Iterator& child, size_t index) const
 * - std::string get_edge_fontcolor(const Iterator& child, size_t index) const
 *
 * Optional filtering:
 * - bool should_process() const (handled by dag_walker)
 *
 * @tparam Iterator The iterator type that represents tree/DAG nodes
 * @param root_iterator The root iterator to start traversal from
 * @param out Output stream for the DOT content
 * @param config Configuration for the DOT graph appearance
 *
 * @requires DotGraphIterator<Iterator>
 */
template <DotGraphIterator Iterator>
void generate_dot_graph(const Iterator& root_iterator, std::ostream& out,
                        const DotConfig& config = DotConfig()) {
    static_assert(DotGraphIterator<Iterator>,
                  "Iterator must satisfy DotGraphIterator concept for DOT graph generation");

    // Write DOT header with configuration
    out << "digraph " << config.graph_name << " {\n";

    if (!config.rankdir.empty()) {
        out << "    rankdir=" << config.rankdir << ";\n";
    }

    if (!config.font_name.empty()) {
        out << "    node [fontname=\"" << config.font_name << "\"];\n";
        out << "    edge [fontname=\"" << config.font_name << "\"];\n";
    }

    if (!config.default_node_shape.empty() || !config.default_node_style.empty()) {
        out << "    node [";
        bool first = true;
        if (!config.default_node_shape.empty()) {
            out << "shape=" << config.default_node_shape;
            first = false;
        }
        if (!config.default_node_style.empty()) {
            if (!first)
                out << ", ";
            out << "style=" << config.default_node_style;
        }
        out << "];\n";
    }

    if (!config.default_edge_style.empty()) {
        out << "    edge [style=" << config.default_edge_style << "];\n";
    }

    out << "\n";

    // Node ID management for unique DOT identifiers
    // Use shared node_id_allocator to produce stable node identifiers
    graph_common::node_id_allocator id_alloc("node");

    auto get_node_id = [&](const Iterator& iter) -> std::string {
        const void* key = iter.get_node_address();
        return id_alloc.get_id(key);
    };

    // Generate node attributes from iterator properties using shared helpers
    auto build_node_attributes = [&](const Iterator& iter,
                                     const std::string& node_id) -> std::string {
        std::vector<std::string> props;

        // Label (use iterator label or fallback to node_id)
        if constexpr (has_get_label<Iterator>) {
            props.push_back("label = \"" + graph_render_helpers::escape_label(iter.get_label())
                            + "\"");
        } else {
            props.push_back("label = \"" + graph_render_helpers::escape_label(node_id) + "\"");
        }

        // Optional node properties
        if constexpr (has_get_shape<Iterator>) {
            props.push_back("shape=" + iter.get_shape());
        }
        if constexpr (has_get_style<Iterator>) {
            props.push_back("style = " + iter.get_style());
        }
        if constexpr (has_get_fillcolor<Iterator>) {
            props.push_back("fillcolor = " + iter.get_fillcolor());
        }
        if constexpr (has_get_fontcolor<Iterator>) {
            props.push_back("fontcolor = " + iter.get_fontcolor());
        }
        if constexpr (has_get_tooltip<Iterator>) {
            props.push_back("tooltip = \"" + graph_render_helpers::escape_label(iter.get_tooltip())
                            + "\"");
        }

        return graph_render_helpers::join_dot_properties(props);
    };

    // Generate edge attributes from iterator properties
    auto build_edge_attributes = [](const Iterator& parent, const Iterator& child,
                                    size_t index) -> std::string {
        std::vector<std::string> properties;

        if constexpr (has_get_edge_label<Iterator>) {
            std::string label = parent.get_edge_label(child, index);
            if (!label.empty()) {
                properties.push_back("label = \"" + graph_render_helpers::escape_label(label)
                                     + "\"");
            }
        }
        if constexpr (has_get_edge_style<Iterator>) {
            properties.push_back("style = " + parent.get_edge_style(child, index));
        }
        if constexpr (has_get_edge_color<Iterator>) {
            properties.push_back("color = " + parent.get_edge_color(child, index));
        }
        if constexpr (has_get_edge_fontcolor<Iterator>) {
            properties.push_back("fontcolor = " + parent.get_edge_fontcolor(child, index));
        }

        return graph_render_helpers::join_dot_properties(properties);
    };

    // Collect nodes and edges in a single traversal to avoid double-walking
    auto [unique_nodes, edges] = dag_walker::collect_nodes_and_edges_topological(root_iterator);

    if (config.use_bdd_format) {
        // BDD-specific grouped shape format

        // First, create node ID mapping by touching each node
        for (const auto& node : unique_nodes) {
            id_alloc.get_id(node.get_node_address());
        }

        // Group nodes by shape for BDD-style declarations
        std::vector<std::string> square_nodes;
        std::vector<std::string> circle_nodes;

        for (const auto& node : unique_nodes) {
            std::string node_id = id_alloc.get_id(node.get_node_address());
            std::string shape = "circle";  // default
            if constexpr (has_get_shape<Iterator>) {
                shape = node.get_shape();
            }

            if (shape == "square") {
                square_nodes.push_back(node_id);
            } else {
                circle_nodes.push_back(node_id);  // Default to circle
            }
        }

        // Output shape declarations (BDD-specific format)
        if (!square_nodes.empty()) {
            out << "    node [shape = square]";
            for (const auto& node_id : square_nodes) {
                out << " " << node_id;
            }
            out << ";\n";
            out << "    node [shape = circle];\n\n";
        }

        // Output individual node declarations (without shape attributes for BDD format)
        std::vector<std::string> node_lines;
        node_lines.reserve(unique_nodes.size());
        for (const auto& node : unique_nodes) {
            std::string node_id = id_alloc.get_id(node.get_node_address());
            std::vector<std::string> props;
            if constexpr (has_get_label<Iterator>) {
                props.push_back("label = \"" + graph_render_helpers::escape_label(node.get_label())
                                + "\"");
            } else {
                props.push_back("label = \"" + graph_render_helpers::escape_label(node_id) + "\"");
            }
            if constexpr (has_get_tooltip<Iterator>) {
                props.push_back("tooltip = \""
                                + graph_render_helpers::escape_label(node.get_tooltip()) + "\"");
            }
            node_lines.push_back("    " + node_id + " "
                                 + graph_render_helpers::join_dot_properties(props) + ";\n");
        }

        std::sort(node_lines.begin(), node_lines.end());
        for (const auto& line : node_lines) {
            out << line;
        }
    } else {
        // Standard format: individual node declarations with all attributes
        std::vector<std::string> node_lines;
        node_lines.reserve(unique_nodes.size());
        for (const auto& node : unique_nodes) {
            std::string node_id = get_node_id(node);
            std::string node_attrs = build_node_attributes(node, node_id);
            node_lines.push_back("    " + node_id + " " + node_attrs + ";\n");
        }

        std::sort(node_lines.begin(), node_lines.end());
        for (const auto& line : node_lines) {
            out << line;
        }
    }

    // Use the edges collected above
    out << "\n";
    std::vector<std::string> edge_lines;
    edge_lines.reserve(edges.size());
    for (const auto& edge : edges) {
        std::string parent_id = get_node_id(edge.parent);
        std::string child_id = get_node_id(edge.child);
        std::string edge_attrs = build_edge_attributes(edge.parent, edge.child, edge.child_index);

        std::string line = "    " + parent_id + " -> " + child_id;
        if (!edge_attrs.empty()) {
            line += " " + edge_attrs;
        }
        line += ";\n";
        edge_lines.push_back(std::move(line));
    }

    std::sort(edge_lines.begin(), edge_lines.end());
    for (const auto& line : edge_lines) {
        out << line;
    }

    out << "}\n";
}

/**
 * @brief Convenience function for DOT generation with simple graph name
 *
 * @tparam Iterator The iterator type that represents tree/DAG nodes
 * @param root_iterator The root iterator to start traversal from
 * @param out Output stream for the DOT content
 * @param graph_name Name for the generated graph
 *
 * @requires DotGraphIterator<Iterator>
 */
template <DotGraphIterator Iterator>
void generate_dot_graph(const Iterator& root_iterator, std::ostream& out,
                        const std::string& graph_name) {
    static_assert(DotGraphIterator<Iterator>,
                  "Iterator must satisfy DotGraphIterator concept for DOT graph generation");

    generate_dot_graph(root_iterator, out, DotConfig(graph_name));
}

}  // namespace dot_graph
