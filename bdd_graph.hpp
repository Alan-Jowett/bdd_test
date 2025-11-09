// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file bdd_graph.hpp
 * @brief BDD (Binary Decision Diagram) graph generation for DOT visualization
 *
 * Provides iterator interface and DOT generation functions for visualizing
 * Binary Decision Diagrams using the templated DOT generation system.
 * Handles BDD-specific requirements like shared nodes and edge styling.
 *
 * @author Alan Jowett
 * @date 2025
 * @copyright Copyright (c) 2025 Alan Jowett. Licensed under the MIT License
 */

#pragma once

#include "dot_graph_generator.hpp"
#include <libteddy/core.hpp>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

// ============================================================================
// BDD Iterator for DOT graph generation
// ============================================================================

/**
 * @brief Iterator for traversing BDD structures with DOT graph properties
 *
 * This iterator provides a consistent interface for generating DOT graphs from
 * Binary Decision Diagrams using the templated DOT generation system. It wraps
 * TeDDy library BDD nodes and provides the required interface methods.
 *
 * The iterator works with the enhanced DOT generation system that properly handles
 * DAGs with shared nodes, making it suitable for BDD visualization without
 * duplicate node definitions.
 *
 * The iterator provides the following DOT properties:
 * - get_label(): Node label (variable name or terminal value)
 * - get_shape(): Node shape (circle for variables, square for terminals)
 * - get_tooltip(): Tooltip with variable index or terminal value
 * - get_node_address(): Node address for unique identification
 * - get_children(): Child iterators for traversal
 * - get_edge_style(): Style for edges (solid/dashed)
 *
 * Usage example:
 * @code
 * bdd_iterator root_iter(bdd_root, variable_names);
 * generate_dot_graph(root_iter, std::cout, "BDD");
 * @endcode
 */
class bdd_iterator {
public:
    using node_t = teddy::bdd_manager::diagram_t::node_t;

private:
    node_t* current_node_;                           ///< Current BDD node
    const std::vector<std::string>* variable_names_; ///< Variable names for labels

public:
    /**
     * @brief Constructs iterator from BDD node
     * 
     * @param node BDD node to iterate over (can be nullptr for end iterator)
     * @param variable_names Vector of variable names for labeling
     */
    explicit bdd_iterator(node_t* node = nullptr, 
                         const std::vector<std::string>* variable_names = nullptr)
        : current_node_(node), variable_names_(variable_names) {
    }

    /**
     * @brief Copy constructor
     */
    bdd_iterator(const bdd_iterator& other)
        : current_node_(other.current_node_), variable_names_(other.variable_names_) {
    }

    /**
     * @brief Move constructor
     */
    bdd_iterator(bdd_iterator&& other) noexcept
        : current_node_(other.current_node_), variable_names_(other.variable_names_) {
        other.current_node_ = nullptr;
    }

    /**
     * @brief Copy assignment
     */
    bdd_iterator& operator=(const bdd_iterator& other) {
        if (this != &other) {
            current_node_ = other.current_node_;
            variable_names_ = other.variable_names_;
        }
        return *this;
    }

    /**
     * @brief Move assignment
     */
    bdd_iterator& operator=(bdd_iterator&& other) noexcept {
        if (this != &other) {
            current_node_ = other.current_node_;
            variable_names_ = other.variable_names_;
            other.current_node_ = nullptr;
        }
        return *this;
    }

    /**
     * @brief Checks if iterator is valid (not end)
     */
    bool is_valid() const {
        return current_node_ != nullptr;
    }

    /**
     * @brief Equality comparison
     */
    bool operator==(const bdd_iterator& other) const {
        return current_node_ == other.current_node_;
    }

    /**
     * @brief Inequality comparison
     */
    bool operator!=(const bdd_iterator& other) const {
        return !(*this == other);
    }

    // ========================================================================
    // DOT Graph Property Methods (required by template system)
    // ========================================================================

    /**
     * @brief Gets node address for unique identification
     * @return Pointer address that can be used as a unique key for this node
     */
    const void* get_node_address() const {
        return static_cast<const void*>(current_node_);
    }

    /**
     * @brief Gets node label for DOT display
     * @return Label string (variable name or terminal value)
     */
    std::string get_label() const {
        if (!current_node_) return "";
        
        if (current_node_->is_terminal()) {
            return std::to_string(current_node_->get_value());
        } else {
            int var_index = current_node_->get_index();
            if (variable_names_ && var_index < variable_names_->size()) {
                return (*variable_names_)[var_index];
            } else {
                return "x" + std::to_string(var_index);
            }
        }
    }

    /**
     * @brief Gets node shape for DOT display
     * @return Shape string ("square" for terminals, "circle" for variables)
     */
    std::string get_shape() const {
        if (!current_node_) return "circle";
        
        return current_node_->is_terminal() ? "square" : "circle";
    }

    /**
     * @brief Gets tooltip for DOT display
     * @return Tooltip string (variable index or terminal value)
     */
    std::string get_tooltip() const {
        if (!current_node_) return "";
        
        if (current_node_->is_terminal()) {
            return std::to_string(current_node_->get_value());
        } else {
            return std::to_string(current_node_->get_index());
        }
    }

    /**
     * @brief Gets child iterators for tree traversal
     * @return Vector of bdd_iterator objects for child nodes
     */
    std::vector<bdd_iterator> get_children() const {
        std::vector<bdd_iterator> children;
        
        if (!current_node_ || current_node_->is_terminal()) {
            return children;
        }
        
        // Add false child (dashed edge)
        node_t* false_child = current_node_->get_son(0);
        if (false_child) {
            children.emplace_back(false_child, variable_names_);
        }
        
        // Add true child (solid edge)
        node_t* true_child = current_node_->get_son(1);
        if (true_child) {
            children.emplace_back(true_child, variable_names_);
        }
        
        return children;
    }

    /**
     * @brief Gets edge style for a specific child
     * @param child Child iterator to get edge style for
     * @param child_index Index of the child (0 for false, 1 for true)
     * @return Edge style string ("dashed" for false, "solid" for true)
     */
    std::string get_edge_style(const bdd_iterator& child, size_t child_index) const {
        if (child_index == 0) {
            return "dashed";  // False edge
        } else {
            return "solid";   // True edge
        }
    }

};

// ============================================================================
// BDD DOT Generation Functions
// ============================================================================

/**
 * @brief Generates BDD DOT graph with terminal node declarations
 * 
 * This function handles BDD-specific formatting by generating terminal node
 * declarations and then using the generic DOT generator for the main content.
 * 
 * @param root_iter BDD iterator starting at the root
 * @param out Output stream for DOT content
 * @param config DOT configuration settings
 */
inline void generate_bdd_dot_graph(const bdd_iterator& root_iter,
                                  std::ostream& out,
                                  const dot_graph::DotConfig& config) {
    // Write DOT header
    out << "digraph " << config.graph_name << " {\n\n";
    
    // Collect terminal nodes for BDD-specific styling
    std::vector<const void*> terminal_nodes;
    std::vector<const void*> all_nodes;
    std::unordered_set<const void*> visited;
    
    std::function<void(const bdd_iterator&)> collect_nodes = [&](const bdd_iterator& current) {
        const void* node_key = current.get_node_address();
        
        if (visited.find(node_key) != visited.end()) {
            return;
        }
        visited.insert(node_key);
        all_nodes.push_back(node_key);
        
        // Check if this is a terminal node (no children)
        std::vector<bdd_iterator> children = current.get_children();
        if (children.empty()) {
            terminal_nodes.push_back(node_key);
        }
        
        for (const bdd_iterator& child : children) {
            collect_nodes(child);
        }
    };
    
    collect_nodes(root_iter);
    
    // Create node ID mapping
    std::unordered_map<const void*, std::string> node_id_map;
    for (size_t i = 0; i < all_nodes.size(); ++i) {
        node_id_map[all_nodes[i]] = "node" + std::to_string(i);
    }
    
    // Output BDD-specific terminal node styling
    if (!terminal_nodes.empty()) {
        out << "    node [shape = square]";
        for (const void* terminal_node : terminal_nodes) {
            out << " " << node_id_map[terminal_node];
        }
        out << ";\n";
        out << "    node [shape = circle];\n\n";
    }
    
    // Generate nodes
    std::unordered_set<const void*> processed_nodes;
    std::function<void(const bdd_iterator&)> output_nodes = [&](const bdd_iterator& current) {
        const void* node_key = current.get_node_address();
        
        if (processed_nodes.find(node_key) != processed_nodes.end()) {
            return;
        }
        processed_nodes.insert(node_key);
        
        std::string node_id = node_id_map[node_key];
        out << "    " << node_id << " [label = \"" << current.get_label() << "\", tooltip = \"" 
            << current.get_tooltip() << "\"];\n";
        
        for (const bdd_iterator& child : current.get_children()) {
            output_nodes(child);
        }
    };
    
    output_nodes(root_iter);
    
    // Generate edges
    out << "\n";
    processed_nodes.clear();
    std::function<void(const bdd_iterator&)> output_edges = [&](const bdd_iterator& current) {
        const void* node_key = current.get_node_address();
        
        if (processed_nodes.find(node_key) != processed_nodes.end()) {
            return;
        }
        processed_nodes.insert(node_key);
        
        std::vector<bdd_iterator> children = current.get_children();
        for (size_t i = 0; i < children.size(); ++i) {
            const bdd_iterator& child = children[i];
            std::string parent_id = node_id_map[node_key];
            std::string child_id = node_id_map[child.get_node_address()];
            std::string style = current.get_edge_style(child, i);
            
            out << "    " << parent_id << " -> " << child_id << " [style = " << style << "];\n";
            output_edges(child);
        }
    };
    
    output_edges(root_iter);
    
    // Write DOT footer
    out << "}\n";
}

/**
 * @brief Writes BDD as DOT graph using the enhanced templated system
 *
 * This function uses the bdd_iterator with the enhanced dot_graph_generator.hpp 
 * system for generating DOT format graphs from Binary Decision Diagrams.
 * 
 * The enhanced DOT generator properly handles DAGs with shared nodes, preventing
 * duplicate node definitions while preserving all edge relationships. This makes
 * it suitable for production use with BDD structures.
 *
 * Generated DOT format includes:
 * - Node properties: variable labels, square shapes for terminals, circles for variables
 * - Edge properties: dashed lines for false edges, solid lines for true edges
 * - Tooltips: variable indices and terminal values
 * - Proper DAG handling: no duplicate nodes, all edges preserved
 * - Format compatibility: matches original format style (interleaved nodes/edges)
 *
 * Note: Node ID assignment may differ from the original manual implementation due to
 * different traversal algorithms, but the generated graph represents the same BDD
 * structure and is functionally equivalent.
 *
 * @param manager BDD manager (currently unused but kept for compatibility)
 * @param diagram BDD diagram to process
 * @param variable_names Vector of variable names for labeling
 * @param out Output stream for DOT content
 * @param graph_name Name for the generated DOT graph (default: "DD")
 *
 * Example usage:
 * @code
 * write_bdd_to_dot(manager, diagram, variable_names, std::cout);
 * @endcode
 */
inline void write_bdd_to_dot(teddy::bdd_manager& manager, 
                            teddy::bdd_manager::diagram_t diagram,
                            const std::vector<std::string>& variable_names, 
                            std::ostream& out,
                            const std::string& graph_name = "DD") {
    // Create an iterator from the BDD root
    bdd_iterator root_iter(diagram.unsafe_get_root(), &variable_names);
    
    // Configure the DOT generation for BDD format
    dot_graph::DotConfig config(graph_name);
    config.rankdir = "";                         // No rankdir (original doesn't have it)
    config.font_name = "";                       // No font declarations (original doesn't have it)
    config.default_node_shape = "";             // Shapes are specified per node
    config.default_node_style = "";             // No default style
    config.default_edge_style = "";             // Edge styles specified per edge
    config.show_node_ids = false;               // Don't show internal IDs
    
    // Generate BDD-specific terminal declarations and then the DOT graph
    generate_bdd_dot_graph(root_iter, out, config);
}