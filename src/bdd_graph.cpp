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

#include "dag_walker.hpp"
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

    // Type-safe node access
    node_t* get_node() const {
        return current_node_;
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

/**
 * @brief Convenience wrapper around dag_walker::walk_dag for BDD nodes
 *
 * @param root_iter Starting iterator for traversal
 * @param visitor Function to call on each unique node during traversal
 */
void traverse_bdd_nodes(const bdd_iterator& root_iter,
                        std::function<void(const bdd_iterator&)> visitor) {
    dag_walker::walk_dag(root_iter, [&](const dag_walker::NodeInfo<bdd_iterator>& node_info) {
        if (!node_info.is_revisit) {
            visitor(node_info.node);
        }
    });
}

/**
 * @brief Collect all nodes and identify terminal nodes in a BDD structure
 */
void collect_and_categorize_nodes(const bdd_iterator& root_iter,
                                  std::vector<const void*>& all_nodes,
                                  std::vector<const void*>& terminal_nodes) {
    all_nodes.clear();
    terminal_nodes.clear();

    // Use dag_walker to collect all unique nodes
    std::vector<bdd_iterator> unique_node_iters = dag_walker::collect_unique_nodes(root_iter);

    // Extract node addresses and identify terminals
    for (const auto& node_iter : unique_node_iters) {
        const void* node_key = node_iter.get_node_address();
        all_nodes.push_back(node_key);

        // Check if this is a terminal node (no children)
        std::vector<bdd_iterator> children = node_iter.get_children();
        if (children.empty()) {
            terminal_nodes.push_back(node_key);
        }
    }
}

/**
 * @brief Output terminal node styling declarations
 */
void output_terminal_node_styling(const std::vector<const void*>& terminal_nodes,
                                  const std::unordered_map<const void*, std::string>& node_id_map,
                                  std::ostream& out) {
    if (!terminal_nodes.empty()) {
        out << "    node [shape = square]";
        for (const void* terminal_node : terminal_nodes) {
            out << " " << node_id_map.at(terminal_node);
        }
        out << ";\n";
        out << "    node [shape = circle];\n\n";
    }
}

/**
 * @brief Output node declarations with labels and tooltips
 */
void output_node_declarations(const bdd_iterator& root_iter,
                              const std::unordered_map<const void*, std::string>& node_id_map,
                              std::ostream& out) {
    traverse_bdd_nodes(root_iter, [&](const bdd_iterator& current) {
        const void* node_key = current.get_node_address();
        std::string node_id = node_id_map.at(node_key);
        out << "    " << node_id << " [label = \"" << current.get_label() << "\", tooltip = \""
            << current.get_tooltip() << "\"];\n";
    });
}

/**
 * @brief Output edge declarations with proper styling (preserves original traversal order)
 */
void output_edge_declarations(const bdd_iterator& root_iter,
                              const std::unordered_map<const void*, std::string>& node_id_map,
                              std::ostream& out) {
    std::unordered_set<const void*> processed_nodes;

    std::function<void(const bdd_iterator&)> output_edges = [&](const bdd_iterator& current) {
        const void* node_key = current.get_node_address();

        if (processed_nodes.find(node_key) != processed_nodes.end()) {
            return;
        }
        processed_nodes.insert(node_key);

        std::vector<bdd_iterator> children = current.get_children();
        for (size_t i = 0; i < children.size(); ++i) {
            const bdd_iterator& child = children[i];
            std::string parent_id = node_id_map.at(node_key);
            std::string child_id = node_id_map.at(child.get_node_address());
            std::string style = current.get_edge_style(child, i);

            out << "    " << parent_id << " -> " << child_id << " [style = " << style << "];\n";
            output_edges(child);
        }
    };

    output_edges(root_iter);
}

void generate_bdd_dot_graph(const bdd_iterator& root_iter, std::ostream& out,
                            const dot_graph::DotConfig& config) {
    // Write DOT header
    out << "digraph " << config.graph_name << " {\n\n";

    // Collect and categorize all nodes
    std::vector<const void*> all_nodes;
    std::vector<const void*> terminal_nodes;
    collect_and_categorize_nodes(root_iter, all_nodes, terminal_nodes);

    // Create node ID mapping
    std::unordered_map<const void*, std::string> node_id_map;
    for (size_t i = 0; i < all_nodes.size(); ++i) {
        node_id_map[all_nodes[i]] = "node" + std::to_string(i);
    }

    // Output BDD-specific terminal node styling
    output_terminal_node_styling(terminal_nodes, node_id_map, out);

    // Generate node declarations
    output_node_declarations(root_iter, node_id_map, out);

    // Generate edge declarations
    out << "\n";
    output_edge_declarations(root_iter, node_id_map, out);

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

// ============================================================================
// DAG Walker Based BDD Node Collection
// ============================================================================

std::vector<teddy::bdd_manager::diagram_t::node_t*> collect_bdd_nodes_topological(
    teddy::bdd_manager::diagram_t diagram, const std::vector<std::string>& variable_names) {
    using node_t = teddy::bdd_manager::diagram_t::node_t;

    // Create BDD iterator and collect nodes in topological order
    bdd_iterator root_iter(diagram.unsafe_get_root(), &variable_names);
    std::vector<bdd_iterator> nodes_in_order =
        dag_walker::collect_unique_nodes_topological(root_iter);

    // Convert iterator vector to node pointer vector using type-safe access
    std::vector<node_t*> result;
    result.reserve(nodes_in_order.size());

    for (const auto& node_iter : nodes_in_order) {
        result.push_back(node_iter.get_node());
    }

    return result;
}