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

#include <concepts>
#include <format>
#include <iostream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "dag_walker.hpp"

namespace dot_graph {

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
concept has_get_style = requires(T t) {
    { t.get_style() } -> std::convertible_to<std::string>;
};

template <typename T>
concept has_get_fillcolor = requires(T t) {
    { t.get_fillcolor() } -> std::convertible_to<std::string>;
};

template <typename T>
concept has_get_fontcolor = requires(T t) {
    { t.get_fontcolor() } -> std::convertible_to<std::string>;
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

template <typename T>
concept has_get_edge_color = requires(T t, T child, std::size_t index) {
    { t.get_edge_color(child, index) } -> std::convertible_to<std::string>;
};

template <typename T>
concept has_get_edge_fontcolor = requires(T t, T child, std::size_t index) {
    { t.get_edge_fontcolor(child, index) } -> std::convertible_to<std::string>;
};

/// @}

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
    bool show_node_ids = false;                 ///< Whether to show internal node IDs
};

/**
 * @brief Pure iterator-based DOT generation using dag_walker for traversal
 *
 * This function generates Graphviz DOT format from tree/DAG structures by leveraging
 * the dag_walker for all traversal logic and focusing purely on DOT formatting.
 *
 * Required iterator interface:
 * - std::vector<Iterator> get_children() const
 * - const void* get_node_address() const (for unique node identification)
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
 */
template <typename Iterator>
void generate_dot_graph(const Iterator& root_iterator, std::ostream& out,
                        const DotConfig& config = DotConfig()) {
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
    std::unordered_map<const void*, std::string> node_id_map;
    int next_node_id = 0;

    auto get_node_id = [&](const Iterator& iter) -> std::string {
        const void* key = iter.get_node_address();
        auto it = node_id_map.find(key);
        if (it != node_id_map.end()) {
            return it->second;
        }
        std::string id = std::format("node{}", next_node_id++);
        node_id_map[key] = id;
        return id;
    };

    // Generate node attributes from iterator properties
    auto build_node_attributes = [&](const Iterator& iter,
                                     const std::string& node_id) -> std::string {
        std::string attrs = "[";
        bool first = true;

        // Label (use iterator label or fallback to node_id)
        if constexpr (has_get_label<Iterator>) {
            attrs += "label = \"" + iter.get_label() + "\"";
        } else {
            attrs += "label = \"" + node_id + "\"";
        }
        first = false;

        // Optional node properties
        if constexpr (has_get_shape<Iterator>) {
            attrs += ", shape=" + iter.get_shape();
        }
        if constexpr (has_get_style<Iterator>) {
            attrs += ", style = " + iter.get_style();
        }
        if constexpr (has_get_fillcolor<Iterator>) {
            attrs += ", fillcolor = " + iter.get_fillcolor();
        }
        if constexpr (has_get_fontcolor<Iterator>) {
            attrs += ", fontcolor = " + iter.get_fontcolor();
        }
        if constexpr (has_get_tooltip<Iterator>) {
            attrs += ", tooltip = \"" + iter.get_tooltip() + "\"";
        }

        attrs += "]";
        return attrs;
    };

    // Generate edge attributes from iterator properties
    auto build_edge_attributes = [](const Iterator& parent, const Iterator& child,
                                    size_t index) -> std::string {
        std::string attrs;
        std::vector<std::string> properties;

        if constexpr (has_get_edge_label<Iterator>) {
            std::string label = parent.get_edge_label(child, index);
            if (!label.empty()) {
                properties.push_back("label = \"" + label + "\"");
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

        if (!properties.empty()) {
            attrs = "[";
            for (size_t i = 0; i < properties.size(); ++i) {
                if (i > 0)
                    attrs += ", ";
                attrs += properties[i];
            }
            attrs += "]";
        }

        return attrs;
    };

    // Use dag_walker to collect all unique nodes and output them
    auto unique_nodes = dag_walker::collect_unique_nodes(root_iterator);
    for (const auto& node : unique_nodes) {
        std::string node_id = get_node_id(node);
        std::string node_attrs = build_node_attributes(node, node_id);
        out << "    " << node_id << " " << node_attrs << ";\n";
    }

    // Use dag_walker to collect all edges and output them
    out << "\n";
    auto edges = dag_walker::collect_edges(root_iterator);
    for (const auto& edge : edges) {
        std::string parent_id = get_node_id(edge.parent);
        std::string child_id = get_node_id(edge.child);
        std::string edge_attrs = build_edge_attributes(edge.parent, edge.child, edge.child_index);

        out << "    " << parent_id << " -> " << child_id;
        if (!edge_attrs.empty()) {
            out << " " << edge_attrs;
        }
        out << ";\n";
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
 */
template <typename Iterator>
void generate_dot_graph(const Iterator& root_iterator, std::ostream& out,
                        const std::string& graph_name) {
    generate_dot_graph(root_iterator, out, DotConfig(graph_name));
}

}  // namespace dot_graph