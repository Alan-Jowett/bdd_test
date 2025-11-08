/**
 * @file dot_graph_generator.hpp
 * @brief Generic template for generating Graphviz DOT diagrams from tree-like structures
 * 
 * This header provides a pure iterator-based template system for generating DOT format
 * graphs. The iterator represents both the current node and the traversal logic,
 * eliminating the need for separate data types or property classes. Iterator methods
 * return DOT attribute strings directly for maximum simplicity.
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
 * @brief Pure iterator-based DOT generation template with optional property methods
 * 
 * This function works with iterators that are self-contained nodes.
 * The iterator represents both the current position and the traversal logic,
 * with optional methods for individual DOT properties.
 *
 * Required iterator interface:
 * - std::vector<Iterator> get_children() const  
 * - std::string get_node_id() const
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
 * @tparam Iterator The iterator type that represents tree nodes
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
    out << "    rankdir=" << config.rankdir << ";\n";
    out << "    node [fontname=\"" << config.font_name << "\"];\n";
    out << "    edge [fontname=\"" << config.font_name << "\"];\n";
    
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
    
    // Helper to build node attributes from individual property methods
    auto build_node_attributes = [](const Iterator& iter) -> std::string {
        std::string attrs = "[";
        bool first = true;
        
        // Add label (required for readability, use node_id as fallback)
        if constexpr (has_get_label<Iterator>::value) {
            attrs += "label=\"" + iter.get_label() + "\"";
            first = false;
        } else {
            attrs += "label=\"" + iter.get_node_id() + "\"";
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
            attrs += "style=" + iter.get_style();
            first = false;
        }
        
        if constexpr (has_get_fillcolor<Iterator>::value) {
            if (!first) attrs += ", ";
            attrs += "fillcolor=" + iter.get_fillcolor();
            first = false;
        }
        
        if constexpr (has_get_fontcolor<Iterator>::value) {
            if (!first) attrs += ", ";
            attrs += "fontcolor=" + iter.get_fontcolor();
            first = false;
        }
        
        if constexpr (has_get_tooltip<Iterator>::value) {
            if (!first) attrs += ", ";
            attrs += "tooltip=\"" + iter.get_tooltip() + "\"";
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
                attrs += "label=\"" + label + "\"";
                first = false;
            }
        }
        
        if constexpr (has_get_edge_style<Iterator>::value) {
            if (!first) attrs += ", ";
            attrs += "style=" + parent.get_edge_style(child, index);
            first = false;
        }
        
        if constexpr (has_get_edge_color<Iterator>::value) {
            if (!first) attrs += ", ";
            attrs += "color=" + parent.get_edge_color(child, index);
            first = false;
        }
        
        if constexpr (has_get_edge_fontcolor<Iterator>::value) {
            if (!first) attrs += ", ";
            attrs += "fontcolor=" + parent.get_edge_fontcolor(child, index);
            first = false;
        }
        
        attrs += "]";
        return first ? "" : attrs; // Return empty if no attributes
    };
    
    // Use a recursive helper to traverse the tree
    std::function<void(const Iterator&)> traverse = [&](const Iterator& current) {
        // Skip nodes that shouldn't be processed
        if (!should_process_impl(current)) {
            return;
        }
        
        // Generate node definition
        std::string node_id = current.get_node_id();
        std::string node_attrs = build_node_attributes(current);
        out << "    " << node_id << " " << node_attrs << ";\n";
        
        // Get children and process them
        std::vector<Iterator> children = current.get_children();
        for (size_t i = 0; i < children.size(); ++i) {
            const Iterator& child = children[i];
            
            // Skip children that shouldn't be processed
            if (!should_process_impl(child)) {
                continue;
            }
            
            // Recursively process child
            traverse(child);
            
            // Generate edge from this node to child
            std::string child_id = child.get_node_id();
            std::string edge_attrs = build_edge_attributes(current, child, i);
            out << "    " << node_id << " -> " << child_id;
            
            if (!edge_attrs.empty()) {
                out << " " << edge_attrs;
            }
            out << ";\n";
        }
    };
    
    // Start traversal from root
    traverse(root_iterator);
    
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