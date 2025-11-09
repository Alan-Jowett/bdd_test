// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file bdd_graph.cpp
 * @brief BDD (Binary Decision Diagram) graph generation implementation
 *
 * Implementation of BDD DOT generation functions and iterator class.
 * Provides the implementation for visualizing Binary Decision Diagrams
 * using the templated DOT generation system.
 *
 * @author Alan Jowett
 * @date 2025
 * @copyright Copyright (c) 2025 Alan Jowett. Licensed under the MIT License
 */

#include "bdd_graph.hpp"

#include <functional>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "dot_graph_generator.hpp"

// ============================================================================
// BDD Iterator Class (Implementation Detail)
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
 */
class bdd_iterator {
   public:
    using node_t = teddy::bdd_manager::diagram_t::node_t;

   private:
    node_t* current_node_;                            ///< Current BDD node
    const std::vector<std::string>* variable_names_;  ///< Variable names for labels

   public:
    // Constructors and assignment operators
    explicit bdd_iterator(node_t* node = nullptr,
                          const std::vector<std::string>* variable_names = nullptr)
        : current_node_(node), variable_names_(variable_names) {}

    bdd_iterator(const bdd_iterator& other)
        : current_node_(other.current_node_), variable_names_(other.variable_names_) {}

    bdd_iterator(bdd_iterator&& other) noexcept
        : current_node_(other.current_node_), variable_names_(other.variable_names_) {
        other.current_node_ = nullptr;
    }

    bdd_iterator& operator=(const bdd_iterator& other) {
        if (this != &other) {
            current_node_ = other.current_node_;
            variable_names_ = other.variable_names_;
        }
        return *this;
    }

    bdd_iterator& operator=(bdd_iterator&& other) noexcept {
        if (this != &other) {
            current_node_ = other.current_node_;
            variable_names_ = other.variable_names_;
            other.current_node_ = nullptr;
        }
        return *this;
    }

    // Iterator interface
    bool is_valid() const {
        return current_node_ != nullptr;
    }

    bool operator==(const bdd_iterator& other) const {
        return current_node_ == other.current_node_;
    }

    bool operator!=(const bdd_iterator& other) const {
        return !(*this == other);
    }

    // DOT Graph Property Methods (required by template system)
    const void* get_node_address() const {
        return static_cast<const void*>(current_node_);
    }

    std::string get_label() const {
        if (!current_node_)
            return "";

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

    std::string get_shape() const {
        if (!current_node_)
            return "circle";

        return current_node_->is_terminal() ? "square" : "circle";
    }

    std::string get_tooltip() const {
        if (!current_node_)
            return "";

        if (current_node_->is_terminal()) {
            return std::to_string(current_node_->get_value());
        } else {
            return std::to_string(current_node_->get_index());
        }
    }

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

    std::string get_edge_style(const bdd_iterator& child, size_t child_index) const {
        if (child_index == 0) {
            return "dashed";  // False edge
        } else {
            return "solid";  // True edge
        }
    }
};

// ============================================================================
// BDD DOT Generation Functions Implementation
// ============================================================================

void generate_bdd_dot_graph(const bdd_iterator& root_iter, std::ostream& out,
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

void write_bdd_to_dot(teddy::bdd_manager& manager, teddy::bdd_manager::diagram_t diagram,
                      const std::vector<std::string>& variable_names, std::ostream& out,
                      const std::string& graph_name) {
    // Create an iterator from the BDD root
    bdd_iterator root_iter(diagram.unsafe_get_root(), &variable_names);

    // Configure the DOT generation for BDD format
    dot_graph::DotConfig config(graph_name);
    config.rankdir = "";             // No rankdir (original doesn't have it)
    config.font_name = "";           // No font declarations (original doesn't have it)
    config.default_node_shape = "";  // Shapes are specified per node
    config.default_node_style = "";  // No default style
    config.default_edge_style = "";  // Edge styles specified per edge
    config.show_node_ids = false;    // Don't show internal IDs

    // Generate BDD-specific terminal declarations and then the DOT graph
    generate_bdd_dot_graph(root_iter, out, config);
}