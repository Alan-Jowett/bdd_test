/**
 * @file dot_graph_generator.hpp
 * @brief Generic template for generating Graphviz DOT diagrams from tree and DAG structures
 * 
 * This header provides a pure iterator-based template system for generating DOT format
 * graphs. The iterator represents both the current node and the traversal logic,
 * eliminating the need for separate data types or property classes. Iterator methods
 * return DOT attribute strings directly for maximum simplicity.
 * 
 * Supports both tree and DAG (Directed Acyclic Graph) structures using a two-phase
 * approach: first collecting all unique nodes and their relationships, then outputting
 * all edges. This prevents duplicate node definitions while preserving all edges in
 * DAGs with shared nodes.
 * 
 * @author BDD Expression Converter Project
 * @date November 8, 2025
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

namespace dot_graph {

/// @name SFINAE helpers for optional property methods
/// @{

// Node property detection
template<typename Iterator, typename = void>
struct has_get_label : std::false_type {};
template<typename Iterator>
struct has_get_label<Iterator, std::void_t<decltype(std::declval<Iterator>().get_label())>> : std::true_type {};

template<typename Iterator, typename = void>
struct has_get_shape : std::false_type {};
template<typename Iterator>
struct has_get_shape<Iterator, std::void_t<decltype(std::declval<Iterator>().get_shape())>> : std::true_type {};

template<typename Iterator, typename = void>
struct has_get_style : std::false_type {};
template<typename Iterator>
struct has_get_style<Iterator, std::void_t<decltype(std::declval<Iterator>().get_style())>> : std::true_type {};

template<typename Iterator, typename = void>
struct has_get_fillcolor : std::false_type {};
template<typename Iterator>
struct has_get_fillcolor<Iterator, std::void_t<decltype(std::declval<Iterator>().get_fillcolor())>> : std::true_type {};

template<typename Iterator, typename = void>
struct has_get_fontcolor : std::false_type {};
template<typename Iterator>
struct has_get_fontcolor<Iterator, std::void_t<decltype(std::declval<Iterator>().get_fontcolor())>> : std::true_type {};

template<typename Iterator, typename = void>
struct has_get_tooltip : std::false_type {};
template<typename Iterator>
struct has_get_tooltip<Iterator, std::void_t<decltype(std::declval<Iterator>().get_tooltip())>> : std::true_type {};

// Edge property detection
template<typename Iterator, typename = void>
struct has_get_edge_label : std::false_type {};
template<typename Iterator>
struct has_get_edge_label<Iterator, std::void_t<decltype(std::declval<Iterator>().get_edge_label(std::declval<Iterator>(), std::size_t{}))>> : std::true_type {};

template<typename Iterator, typename = void>
struct has_get_edge_style : std::false_type {};
template<typename Iterator>
struct has_get_edge_style<Iterator, std::void_t<decltype(std::declval<Iterator>().get_edge_style(std::declval<Iterator>(), std::size_t{}))>> : std::true_type {};

template<typename Iterator, typename = void>
struct has_get_edge_color : std::false_type {};
template<typename Iterator>
struct has_get_edge_color<Iterator, std::void_t<decltype(std::declval<Iterator>().get_edge_color(std::declval<Iterator>(), std::size_t{}))>> : std::true_type {};

template<typename Iterator, typename = void>
struct has_get_edge_fontcolor : std::false_type {};
template<typename Iterator>
struct has_get_edge_fontcolor<Iterator, std::void_t<decltype(std::declval<Iterator>().get_edge_fontcolor(std::declval<Iterator>(), std::size_t{}))>> : std::true_type {};

/**
 * @brief Helper to detect if an iterator has a should_process method
 */
template<typename Iterator, typename = void>
struct has_should_process : std::false_type {};

template<typename Iterator>
struct has_should_process<Iterator, std::void_t<decltype(std::declval<Iterator>().should_process())>>
    : std::true_type {};

template<typename Iterator>
constexpr bool has_should_process_v = has_should_process<Iterator>::value;

/// @}

/**
 * @brief Configuration settings for DOT graph generation
 */
struct DotConfig {
    std::string graph_name = "Graph";           ///< Name of the DOT graph
    std::string rankdir = "TB";                 ///< Graph direction (TB, BT, LR, RL)
    std::string font_name = "Arial";            ///< Font family for nodes and edges
    std::string default_node_shape = "circle";  ///< Default shape for nodes
    std::string default_node_style = "filled"; ///< Default style for nodes
    std::string default_edge_style = "solid";  ///< Default style for edges
    bool show_node_ids = false;                 ///< Whether to show internal node IDs
    
    DotConfig() = default;
    explicit DotConfig(const std::string& name) : graph_name(name) {}
};

/**
 * @brief Pure iterator-based DOT generation template with tree and DAG support
 * 
 * This function works with iterators that are self-contained nodes.
 * The iterator represents both the current position and the traversal logic,
 * with optional methods for individual DOT properties.
 *
 * The function uses a two-phase approach to properly handle DAGs (Directed Acyclic
 * Graphs) with shared nodes:
 * 1. Node collection phase: Traverse the structure once, defining each unique node
 * 2. Edge output phase: Output all collected edges without node redefinition
 *
 * This prevents duplicate node definitions while preserving all relationships,
 * making it suitable for both trees (expression trees) and DAGs (BDDs).
 *
 * Required iterator interface:
 * - std::vector<Iterator> get_children() const  
 * - const void* get_node_address() const (for unique node identification)
 *
 * Optional node property methods (auto-detected via SFINAE):
 * - std::string get_label() const
 * - std::string get_shape() const
 * - std::string get_style() const
 * - std::string get_fillcolor() const
 * - std::string get_fontcolor() const
 * - std::string get_tooltip() const
 *
 * Optional edge property methods (auto-detected via SFINAE):
 * - std::string get_edge_label(const Iterator& child, size_t index) const
 * - std::string get_edge_style(const Iterator& child, size_t index) const
 * - std::string get_edge_color(const Iterator& child, size_t index) const
 * - std::string get_edge_fontcolor(const Iterator& child, size_t index) const
 *
 * Optional filtering:
 * - bool should_process() const (defaults to true if not provided)
 *
 * @tparam Iterator The iterator type that represents tree/DAG nodes
 * @param root_iterator The root iterator to start traversal from
 * @param out Output stream for the DOT content
 * @param config Configuration for the DOT graph appearance
 */
template<typename Iterator>
void generate_dot_graph(const Iterator& root_iterator,
                       std::ostream& out,
                       const DotConfig& config = DotConfig()) {
    
    // Write DOT header
    out << "digraph " << config.graph_name << " {\n";
    
    // Only add rankdir if specified
    if (!config.rankdir.empty()) {
        out << "    rankdir=" << config.rankdir << ";\n";
    }
    
    // Only add font declarations if specified
    if (!config.font_name.empty()) {
        out << "    node [fontname=\"" << config.font_name << "\"];\n";
        out << "    edge [fontname=\"" << config.font_name << "\"];\n";
    }
    
    // Only add default node styling if values are provided
    if (!config.default_node_shape.empty() || !config.default_node_style.empty()) {
        out << "    node [";
        bool first = true;
        if (!config.default_node_shape.empty()) {
            out << "shape=" << config.default_node_shape;
            first = false;
        }
        if (!config.default_node_style.empty()) {
            if (!first) out << ", ";
            out << "style=" << config.default_node_style;
        }
        out << "];\n";
    }
    
    // Only add default edge styling if value is provided
    if (!config.default_edge_style.empty()) {
        out << "    edge [style=" << config.default_edge_style << "];\n";
    }
    
    out << "\n";
    
    // Helper to check if iterator has should_process method
    auto should_process_impl = [&](const Iterator& iter) -> bool {
        if constexpr (has_should_process_v<Iterator>) {
            return iter.should_process();
        } else {
            return true; // Default behavior if method not provided
        }
    };

    // Node ID management and visited tracking for DAG support
    std::unordered_map<const void*, std::string> node_id_map;
    std::unordered_set<const void*> visited_nodes;
    std::unordered_set<const void*> defined_nodes;
    std::vector<std::string> edge_definitions;
    int next_node_id = 0;
    
    // Helper to get or assign node ID
    auto get_node_id = [&](const Iterator& iter) -> std::string {
        const void* key = iter.get_node_address();
        auto it = node_id_map.find(key);
        if (it != node_id_map.end()) {
            return it->second;
        }
        std::string id = "node" + std::to_string(next_node_id++);
        node_id_map[key] = id;
        return id;
    };
    
    // Helper to build node attributes from individual property methods
    auto build_node_attributes = [&](const Iterator& iter, const std::string& node_id) -> std::string {
        std::string attrs = "[";
        bool first = true;
        
        // Add label (required for readability, use node_id as fallback)
        if constexpr (has_get_label<Iterator>::value) {
            attrs += "label = \"" + iter.get_label() + "\"";
            first = false;
        } else {
            attrs += "label = \"" + node_id + "\"";
            first = false;
        }
        
        // Add optional properties
        if constexpr (has_get_shape<Iterator>::value) {
            if (!first) attrs += ", ";
            attrs += "shape=" + iter.get_shape();
            first = false;
        }
        
        if constexpr (has_get_style<Iterator>::value) {
            if (!first) attrs += ", ";
            attrs += "style = " + iter.get_style();
            first = false;
        }
        
        if constexpr (has_get_fillcolor<Iterator>::value) {
            if (!first) attrs += ", ";
            attrs += "fillcolor = " + iter.get_fillcolor();
            first = false;
        }
        
        if constexpr (has_get_fontcolor<Iterator>::value) {
            if (!first) attrs += ", ";
            attrs += "fontcolor = " + iter.get_fontcolor();
            first = false;
        }
        
        if constexpr (has_get_tooltip<Iterator>::value) {
            if (!first) attrs += ", ";
            attrs += "tooltip = \"" + iter.get_tooltip() + "\"";
            first = false;
        }
        
        attrs += "]";
        return attrs;
    };
    
    // Helper to build edge attributes from individual property methods
    auto build_edge_attributes = [](const Iterator& parent, const Iterator& child, size_t index) -> std::string {
        std::string attrs = "[";
        bool first = true;
        
        if constexpr (has_get_edge_label<Iterator>::value) {
            std::string label = parent.get_edge_label(child, index);
            if (!label.empty()) {
                attrs += "label = \"" + label + "\"";
                first = false;
            }
        }
        
        if constexpr (has_get_edge_style<Iterator>::value) {
            if (!first) attrs += ", ";
            attrs += "style = " + parent.get_edge_style(child, index);
            first = false;
        }
        
        if constexpr (has_get_edge_color<Iterator>::value) {
            if (!first) attrs += ", ";
            attrs += "color = " + parent.get_edge_color(child, index);
            first = false;
        }
        
        if constexpr (has_get_edge_fontcolor<Iterator>::value) {
            if (!first) attrs += ", ";
            attrs += "fontcolor = " + parent.get_edge_fontcolor(child, index);
            first = false;
        }
        
        attrs += "]";
        return first ? "" : attrs; // Return empty if no attributes
    };
    
    // Use separated format: all nodes first, then all edges (better for complex DAGs)
    std::function<void(const Iterator&)> collect_graph = [&](const Iterator& current) {
        const void* node_key = current.get_node_address();
        
        // Skip nodes that shouldn't be processed
        if (!should_process_impl(current)) {
            return;
        }
        
        // Skip if already visited (DAG cycle detection)
        if (visited_nodes.find(node_key) != visited_nodes.end()) {
            return;
        }
        visited_nodes.insert(node_key);
        
        // Define node if not already defined
        if (defined_nodes.find(node_key) == defined_nodes.end()) {
            std::string node_id = get_node_id(current);
            std::string node_attrs = build_node_attributes(current, node_id);
            out << "    " << node_id << " " << node_attrs << ";\n";
            defined_nodes.insert(node_key);
        }
        
        // Process children and collect edges
        std::vector<Iterator> children = current.get_children();
        for (size_t i = 0; i < children.size(); ++i) {
            const Iterator& child = children[i];
            
            // Skip children that shouldn't be processed
            if (!should_process_impl(child)) {
                continue;
            }
            
            // Collect edge information
            std::string parent_id = get_node_id(current);
            std::string child_id = get_node_id(child);
            std::string edge_attrs = build_edge_attributes(current, child, i);
            
            std::string edge_def = "    " + parent_id + " -> " + child_id;
            if (!edge_attrs.empty()) {
                edge_def += " " + edge_attrs;
            }
            edge_def += ";\n";
            edge_definitions.push_back(edge_def);
            
            // Recursively collect child
            collect_graph(child);
        }
    };
    
    // Phase 1: Collect all nodes and edges
    collect_graph(root_iterator);
    
    // Phase 2: Output all collected edges
    out << "\n";
    for (const std::string& edge : edge_definitions) {
        out << edge;
    }
    
    // Write DOT footer
    out << "}\n";
}

/**
 * @brief Convenience function for pure iterator-based DOT generation
 * 
 * @tparam Iterator The iterator type that represents tree nodes
 * @param root_iterator The root iterator to start traversal from
 * @param out Output stream for the DOT content
 * @param graph_name Name for the generated graph
 */
template<typename Iterator>
void generate_dot_graph(const Iterator& root_iterator,
                       std::ostream& out,
                       const std::string& graph_name) {
    DotConfig config(graph_name);
    generate_dot_graph(root_iterator, out, config);
}

} // namespace dot_graph