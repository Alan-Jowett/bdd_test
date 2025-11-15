// SPDX-License-Identifier: MIT
/**
 * @file
 * @brief Small helpers for rendering nodes and edges in graph output formats.
 *
 * This header provides lightweight, header-only utilities used by the
 * Mermaid and DOT generators to produce deterministic textual lines for
 * nodes, edges and CSS class assignments. The helpers ensure consistent
 * escaping of labels and stable ordering of class-to-node mappings which
 * is important for the test-suite's byte-for-byte comparisons.
 */
#pragma once

#include <algorithm>
#include <format>
#include <iterator>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "node_id_helpers.hpp"

/**
 * @brief Utilities for rendering graph elements (nodes/edges/classes).
 *
 * The functions in this namespace return fully formatted lines suitable for
 * streaming directly into an output stream. Some functions include the
 * trailing newline in the returned string; callers should not append an
 * additional newline when writing the returned string to a stream.
 */
namespace graph_render_helpers {

/**
 * @brief Escape a string for use inside a quoted label.
 *
 * Replaces double-quote characters with the escaped form (\"). The
 * implementation is intentionally minimal — it only escapes quotes because
 * the calling generators wrap labels in double quotes and the test-suite
 * expects this exact escaping behaviour.
 *
 * @param s Input label string to escape.
 * @return Escaped string where each '"' is replaced by '\"'.
 */
inline std::string escape_label(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c == '"')
            out += "\\\"";
        else
            out += c;
    }
    return out;
}

/**
 * @brief Render a Mermaid node definition line for a single node.
 *
 * The function returns a fully formatted line describing the node and its
 * label using a shape appropriate for the requested `shape` parameter. The
 * returned string contains a trailing newline and is ready to stream into
 * an output file.
 *
 * @param node_id The node identifier (already-safe to place in output).
 * @param label The human-readable label for the node (will be escaped).
 * @param shape A textual shape hint: "square", "circle", "diamond",
 *              or "hexagon". Unknown shapes default to a circle-like
 *              representation.
 * @return A formatted Mermaid node definition line (with trailing newline).
 */
inline std::string render_node_line(const std::string& node_id, const std::string& label,
                                    const std::string& shape) {
    std::string esc = escape_label(label);
    if (shape == "square" || shape == "box" || shape == "rect") {
        return std::format("    {}[\"{}\"]\n", node_id, esc);
    } else if (shape == "circle" || shape == "ellipse" || shape == "oval") {
        return std::format("    {}((\"{}\"))\n", node_id, esc);
    } else if (shape == "diamond") {
        return std::format("    {}{{\"{}\"}}\n", node_id, esc);
    } else if (shape == "hexagon") {
        return std::format("    {}{{{{\"{}\"}}}}\n", node_id, esc);
    } else {
        return std::format("    {}((\"{}\"))\n", node_id, esc);
    }
}

/**
 * @brief Render a Mermaid edge line connecting two nodes.
 *
 * The returned string includes the chosen edge style and an optional
 * edge label. The string contains a trailing newline and is ready to
 * be streamed into an output file.
 *
 * @param parent_id The source node id.
 * @param child_id The destination node id.
 * @param edge_style Style hint: "dashed", "dotted", "thick", or empty
 *                   for the default solid edge.
 * @param edge_label Optional label displayed on the edge.
 * @return A formatted Mermaid edge definition line (with trailing newline).
 */
inline std::string render_edge_line(const std::string& parent_id, const std::string& child_id,
                                    const std::string& edge_style, const std::string& edge_label) {
    std::string style = " --> ";
    if (edge_style == "dashed")
        style = " -.-> ";
    else if (edge_style == "dotted")
        style = " -..-> ";
    else if (edge_style == "thick")
        style = " ==> ";

    if (!edge_label.empty()) {
        return std::format("    {}{} |\"{}\"| {}\n", parent_id, style, escape_label(edge_label),
                           child_id);
    }
    return std::format("    {}{}{}\n", parent_id, style, child_id);
}

/**
 * @brief Flatten and deterministically sort class->node mappings.
 *
 * The test-suite expects class assignments to appear in a stable order. This
 * helper converts a map of class_name -> vector(node_ids) into a vector of
 * (node_id, class_name) pairs sorted by the node id using
 * `graph_common::node_id_less` for deterministic numeric-aware ordering.
 *
 * @param class_to_nodes Mapping from CSS class name to nodes assigned to it.
 * @return Vector of (node_id, class_name) pairs sorted by node id.
 */
inline std::vector<std::pair<std::string, std::string>> flatten_and_sort_class_assignments(
    const std::unordered_map<std::string, std::vector<std::string>>& class_to_nodes) {
    std::vector<std::pair<std::string, std::string>> out;
    out.reserve(256);
    for (const auto& p : class_to_nodes) {
        const auto& class_name = p.first;
        const auto& ids = p.second;
        std::transform(ids.begin(), ids.end(), std::back_inserter(out),
                       [&](const std::string& id) { return std::make_pair(id, class_name); });
    }
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) {
        return graph_common::node_id_less(a.first, b.first);
    });
    return out;
}

/**
 * @brief Join DOT property assignments into a single attribute list string.
 *
 * Given property fragments like `label = "X"` this returns
 * `[label = "X", shape=box]`. Returns empty string when no properties
 * are provided.
 */
inline std::string join_dot_properties(const std::vector<std::string>& props) {
    if (props.empty())
        return std::string();
    std::string out = "[";
    for (size_t i = 0; i < props.size(); ++i) {
        if (i > 0)
            out += ", ";
        out += props[i];
    }
    out += "]";
    return out;
}

}  // namespace graph_render_helpers
