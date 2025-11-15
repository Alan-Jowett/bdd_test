// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file mermaid_graph_generator.hpp
 * @brief Mermaid flowchart generator with configurable options
 *
 * This header provides a generic, iterator-based Mermaid flowchart generator
 * intended to produce stable, deterministic Mermaid markup suitable for
 * automated reference comparisons. The generator is configurable via
 * `MermaidConfig` so callers can reproduce legacy output (e.g. TeDDy
 * expression trees or BDD visualizations) by selecting node-id prefixes,
 * emission ordering and class-definitions.
 *
 * The implementation is intentionally header-only and relies on the
 * `dag_walker` utilities for traversal. It is designed to accept iterator
 * types that conform to the project's `BaseGraphIterator` concept; optional
 * properties (labels, shapes, tooltips, edge labels/styles) are detected
 * via C++20 concepts and used when available.
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
#include "node_id_helpers.hpp"

namespace mermaid_graph {

/**
 * @concept has_get_label
 * @brief True if the iterator exposes a `get_label()` returning a string.
 *
 * Iterators meeting this concept provide a textual label for the node
 * which is used as the Mermaid node label when present.
 */
template <typename T>
concept has_get_label = requires(T t) {
    { t.get_label() } -> std::convertible_to<std::string>;
};
/**
 * @concept has_get_shape
 * @brief True if the iterator exposes a `get_shape()` returning a string.
 *
 * This indicates the iterator can describe a preferred Mermaid node shape
 * (for example `circle`, `square`), which will be used instead of the
 * `MermaidConfig::default_node_shape` when available.
 */
template <typename T>
concept has_get_shape = requires(T t) {
    { t.get_shape() } -> std::convertible_to<std::string>;
};
/**
 * @concept has_get_css_class
 * @brief True if the iterator exposes a `get_css_class()` returning a string.
 *
 * When present this value is used to assign a Mermaid CSS class to the
 * generated node (via `class <node> <class>` statements).
 */
template <typename T>
concept has_get_css_class = requires(T t) {
    { t.get_css_class() } -> std::convertible_to<std::string>;
};
/**
 * @concept has_get_tooltip
 * @brief True if the iterator exposes a `get_tooltip()` returning a string.
 *
 * Tooltips are not always emitted by the generator, but this concept marks
 * the iterator as capable of providing one for future use or extensions.
 */
template <typename T>
concept has_get_tooltip = requires(T t) {
    { t.get_tooltip() } -> std::convertible_to<std::string>;
};
/**
 * @concept has_get_edge_label
 * @brief True if the iterator exposes `get_edge_label(child, idx)`.
 *
 * Edge labels are used when `MermaidConfig::show_edge_labels` is enabled
 * to annotate edges with additional information (for example branch
 * conditions in expression trees).
 */
template <typename T>
concept has_get_edge_label = requires(T t, T child, std::size_t idx) {
    { t.get_edge_label(child, idx) } -> std::convertible_to<std::string>;
};
/**
 * @concept has_get_edge_style
 * @brief True if the iterator exposes `get_edge_style(child, idx)`.
 *
 * Edge styles are rendered as Mermaid link labels or attributes when
 * supported by the iterator and configuration.
 */
template <typename T>
concept has_get_edge_style = requires(T t, T child, std::size_t idx) {
    { t.get_edge_style(child, idx) } -> std::convertible_to<std::string>;
};

/**
 * @brief Concept alias for the project's `BaseGraphIterator`.
 *
 * @tparam T type to check
 */
template <typename T>
concept MermaidGraphIterator = BaseGraphIterator<T>;

/**
 * @concept MermaidGraphIterator
 * @brief Alias for the project's `BaseGraphIterator` used by the Mermaid
 *        generator.
 *
 * This concept documents the iterator requirements expected by the
 * `generate_mermaid_graph` template. Additional optional capabilities
 * (labels, shapes, edge labels/styles) are detected via the `has_get_*`
 * concepts above and used when available.
 */

struct MermaidConfig {
    /**
     * Title placed in the Mermaid frontmatter. When `show_frontmatter` is
     * enabled a YAML frontmatter block is emitted containing this title.
     */

    /**
     * @brief Human-readable title emitted in YAML frontmatter (when
     *        `show_frontmatter` is true).
     */
    std::string graph_title = "Graph";

    /**
     * @brief Mermaid graph direction (e.g. "TD", "LR").
     */
    std::string direction = "TD";

    /**
     * @brief Default node shape used when the iterator doesn't provide one.
     */
    std::string default_node_shape = "circle";

    /**
     * @brief Emit a YAML frontmatter block with the graph title.
     */
    bool show_frontmatter = true;

    /**
     * @brief Emit `class` assignments and `classDef` blocks.
     */
    bool show_css_classes = true;

    /**
     * @brief Default CSS class assigned to nodes when the iterator doesn't
     *        provide one and no mapping exists for the node label.
     */
    std::string default_css_class = "default";

    /**
     * @brief Prefix applied to generated node ids (e.g. "N").
     */
    std::string node_id_prefix = "N";

    /**
     * @brief Starting index for node id allocation.
     */
    std::size_t node_id_start = 0;

    /**
     * @brief When true append the generated node id to the displayed label.
     */
    bool show_node_ids = false;

    /**
     * @brief When true render edge labels if the iterator exposes them.
     */
    bool show_edge_labels = true;

    /**
     * @brief Mapping from node label to CSS class name used when the
     *        iterator does not directly provide a CSS class.
     */
    std::unordered_map<std::string, std::string> label_to_class_map;

    /**
     * @brief Ordered list of `classDef` entries to emit at the end of the
     *        graph. Each pair contains the class name and the definition body.
     */
    std::vector<std::pair<std::string, std::string>> class_definitions;

    auto operator<=>(const MermaidConfig& other) const = default;
};

/**
 * @brief Generate a Mermaid flowchart for the graph rooted at `root_iterator`.
 *
 * The function traverses the graph using `dag_walker` and emits a deterministic
 * Mermaid flowchart block to the provided output stream.
 *
 * The generator detects optional iterator capabilities (labels, shapes,
 * css-classes, edge labels and styles) at compile-time via concepts and
 * uses them when available. The output is intended to be stable and suitable
 * for automated reference comparisons.
 *
 * @tparam Iterator Iterator type satisfying `MermaidGraphIterator`.
 * @param root_iterator Iterator pointing at the root of the graph to render.
 * @param out Output stream to which Mermaid markup will be written.
 * @param config Configuration object controlling formatting, ordering, and
 *               CSS/class emission. If omitted, a default-initialized
 *               `MermaidConfig` is used.
 */
template <MermaidGraphIterator Iterator>
void generate_mermaid_graph(const Iterator& root_iterator, std::ostream& out,
                            const MermaidConfig& config = MermaidConfig()) {
    static_assert(MermaidGraphIterator<Iterator>);

    if (config.show_frontmatter) {
        out << "---\n";
        out << "title: " << config.graph_title << "\n";
        out << "---\n";
    }

    out << "flowchart " << config.direction << "\n";

    graph_common::node_id_allocator id_alloc(config.node_id_prefix, config.node_id_start);
    auto get_node_id = [&](const Iterator& iter) -> std::string {
        return id_alloc.get_id(iter.get_node_address());
    };

    using graph_render_helpers::render_node_line;
    auto build_node_definition = [&](const Iterator& iter, const std::string& node_id) {
        std::string label = node_id;
        if constexpr (has_get_label<Iterator>)
            label = iter.get_label();
        if (config.show_node_ids)
            label = label + " (" + node_id + ")";
        std::string shape = config.default_node_shape;
        if constexpr (has_get_shape<Iterator>)
            shape = iter.get_shape();
        return render_node_line(node_id, label, shape);
    };

    using graph_render_helpers::render_edge_line;
    auto build_edge_definition = [&](const Iterator& parent, const Iterator& child, size_t index,
                                     const std::string& parent_id, const std::string& child_id) {
        std::string style;
        if constexpr (has_get_edge_style<Iterator>)
            style = parent.get_edge_style(child, index);
        std::string label;
        if (config.show_edge_labels) {
            if constexpr (has_get_edge_label<Iterator>)
                label = parent.get_edge_label(child, index);
        }
        return render_edge_line(parent_id, child_id, style, label);
    };

    std::unordered_map<std::string, std::vector<std::string>> class_to_nodes;
    std::vector<Iterator> unique_nodes;

    // Collect all edges first so we can know which nodes are parents before emission
    dag_walker::WalkConfig edge_config;
    edge_config.collect_all_edges = true;
    auto edges = dag_walker::collect_edges(root_iterator, edge_config);

    std::unordered_map<const void*, std::vector<dag_walker::EdgeInfo<Iterator>>> edges_by_parent;
    for (const auto& edge : edges)
        edges_by_parent[edge.parent.get_node_address()].push_back(edge);

    std::vector<std::pair<std::string, const void*>> parent_ids_and_keys;
    // Map node address -> emitted node id to avoid repeated linear searches
    std::unordered_map<const void*, std::string> node_address_to_id;

    // Topological emission: collect nodes in numeric/topological order,
    // emit node definitions, and then emit edges grouped by parent id order.
    unique_nodes = dag_walker::collect_unique_nodes_topological(root_iterator);
    for (const auto& node : unique_nodes) {
        std::string node_id = get_node_id(node);
        out << build_node_definition(node, node_id);
        // cache the node id for fast lookups later
        node_address_to_id[node.get_node_address()] = node_id;
        if (config.show_css_classes) {
            std::string css_class;
            if constexpr (has_get_css_class<Iterator>) {
                css_class = node.get_css_class();
            } else if constexpr (has_get_label<Iterator>) {
                auto it = config.label_to_class_map.find(node.get_label());
                css_class =
                    (it != config.label_to_class_map.end()) ? it->second : config.default_css_class;
            } else {
                css_class = config.default_css_class;
            }
            if (!css_class.empty())
                class_to_nodes[css_class].push_back(node_id);
        }
    }

    // collect parent ids in numeric order-aware list for topological emission
    for (const auto& [parent_key, _] : edges_by_parent) {
        auto it = node_address_to_id.find(parent_key);
        if (it != node_address_to_id.end())
            parent_ids_and_keys.push_back({it->second, parent_key});
    }

    std::sort(
        parent_ids_and_keys.begin(), parent_ids_and_keys.end(),
        [](const auto& a, const auto& b) { return graph_common::node_id_less(a.first, b.first); });
    if (!edges.empty() && !unique_nodes.empty())
        out << "\n";

    for (const auto& [parent_id, parent_key] : parent_ids_and_keys) {
        auto& parent_edges = edges_by_parent[parent_key];
        std::sort(parent_edges.begin(), parent_edges.end(),
                  [](const auto& a, const auto& b) { return a.child_index < b.child_index; });
        for (const auto& edge : parent_edges) {
            const void* parent_addr = edge.parent.get_node_address();
            const void* child_addr = edge.child.get_node_address();
            auto pit = node_address_to_id.find(parent_addr);
            auto cit = node_address_to_id.find(child_addr);
            const std::string& pid =
                (pit != node_address_to_id.end()) ? pit->second : get_node_id(edge.parent);
            const std::string& cid =
                (cit != node_address_to_id.end()) ? cit->second : get_node_id(edge.child);
            out << build_edge_definition(edge.parent, edge.child, edge.child_index, pid, cid);
        }
    }

    if (config.show_css_classes && !class_to_nodes.empty()) {
        out << "\n";
        auto sorted_class_assignments =
            graph_render_helpers::flatten_and_sort_class_assignments(class_to_nodes);
        for (const auto& [node_id, class_name] : sorted_class_assignments) {
            out << "    class " << node_id << " " << class_name << "\n";
        }

        if (!config.class_definitions.empty()) {
            out << "\n";
            for (const auto& [class_name, def_body] : config.class_definitions) {
                out << "    classDef " << class_name << " " << def_body << "\n";
            }
        }
    }
}

template <MermaidGraphIterator Iterator>
/**
 * @brief Convenience overload that accepts a graph title.
 *
 * This overload constructs a `MermaidConfig`, sets the `graph_title` and
 * forwards to the primary `generate_mermaid_graph` implementation.
 *
 * @tparam Iterator Iterator type satisfying `MermaidGraphIterator`.
 * @param root_iterator Iterator pointing at the root of the graph to render.
 * @param out Output stream to which Mermaid markup will be written.
 * @param graph_title Title to place in the emitted frontmatter.
 */
void generate_mermaid_graph(const Iterator& root_iterator, std::ostream& out,
                            const std::string& graph_title) {
    MermaidConfig cfg;
    cfg.graph_title = graph_title;
    generate_mermaid_graph(root_iterator, out, cfg);
}

}  // namespace mermaid_graph
