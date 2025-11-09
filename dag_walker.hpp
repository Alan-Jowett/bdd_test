// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file dag_walker.hpp
 * @brief Generic template for walking DAG (Directed Acyclic Graph) and tree structures
 *
 * This header provides a pure iterator-based template system for traversing
 * tree and DAG structures. The iterator represents both the current node and the
 * traversal logic, making it suitable for various operations beyond just visualization.
 *
 * The walker supports both tree and DAG structures using a visitor pattern approach,
 * properly handling cycles and shared nodes by tracking visited nodes.
 *
 * @author Alan Jowett
 * @date 2025
 * @copyright Copyright (c) 2025 Alan Jowett. Licensed under the MIT License
 */

#pragma once

#include <functional>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace dag_walker {

/**
 * @brief Helper to detect if an iterator has a should_process method
 */
template <typename Iterator, typename = void>
struct has_should_process : std::false_type {};

template <typename Iterator>
struct has_should_process<Iterator,
                          std::void_t<decltype(std::declval<Iterator>().should_process())>>
    : std::true_type {};

template <typename Iterator>
constexpr bool has_should_process_v = has_should_process<Iterator>::value;

/**
 * @brief Configuration for DAG walking behavior
 */
struct WalkConfig {
    bool track_unique_nodes = true;        ///< Whether to track and avoid revisiting unique nodes
    bool call_visitor_on_revisit = false;  ///< Whether to call visitor when revisiting a node

    WalkConfig() = default;
    explicit WalkConfig(bool track_unique) : track_unique_nodes(track_unique) {}
};

/**
 * @brief Information about the current node during traversal
 * @tparam Iterator The iterator type representing the node
 */
template <typename Iterator>
struct NodeInfo {
    const Iterator& node;      ///< The current node iterator
    size_t depth;              ///< Depth in the traversal tree
    size_t index_from_parent;  ///< Index in parent's children vector
    bool is_revisit;           ///< Whether this node was visited before
    const Iterator* parent;    ///< Parent node (nullptr for root)

    NodeInfo(const Iterator& n, size_t d, size_t idx, bool revisit = false,
             const Iterator* p = nullptr)
        : node(n), depth(d), index_from_parent(idx), is_revisit(revisit), parent(p) {}
};

/**
 * @brief Walk a DAG or tree structure using a visitor pattern
 *
 * This function traverses a tree or DAG structure starting from a root iterator,
 * calling a visitor function for each node. The visitor receives detailed information
 * about each node including its position in the traversal.
 *
 * Required iterator interface:
 * - std::vector<Iterator> get_children() const
 * - const void* get_node_address() const (for unique node identification)
 *
 * Optional filtering:
 * - bool should_process() const (defaults to true if not provided)
 *
 * @tparam Iterator The iterator type that represents tree/DAG nodes
 * @tparam Visitor A callable that accepts NodeInfo<Iterator>&
 * @param root_iterator The root iterator to start traversal from
 * @param visitor Function called for each node during traversal
 * @param config Configuration for the walking behavior
 */
template <typename Iterator, typename Visitor>
void walk_dag(const Iterator& root_iterator, Visitor&& visitor,
              const WalkConfig& config = WalkConfig()) {
    std::unordered_set<const void*> visited_nodes;

    // Helper to check if iterator has should_process method
    auto should_process_impl = [&](const Iterator& iter) -> bool {
        if constexpr (has_should_process_v<Iterator>) {
            return iter.should_process();
        } else {
            return true;  // Default behavior if method not provided
        }
    };

    std::function<void(const Iterator&, size_t, size_t, const Iterator*)> walk_impl =
        [&](const Iterator& current, size_t depth, size_t index_from_parent,
            const Iterator* parent) {
            const void* node_key = current.get_node_address();

            // Skip nodes that shouldn't be processed
            if (!should_process_impl(current)) {
                return;
            }

            bool is_revisit = false;
            if (config.track_unique_nodes) {
                // Check if already visited (DAG cycle detection)
                if (visited_nodes.find(node_key) != visited_nodes.end()) {
                    is_revisit = true;
                    if (!config.call_visitor_on_revisit) {
                        return;
                    }
                } else {
                    visited_nodes.insert(node_key);
                }
            }

            // Create node info and call visitor
            NodeInfo<Iterator> node_info(current, depth, index_from_parent, is_revisit, parent);
            visitor(node_info);

            // Don't traverse children if this is a revisit (to prevent infinite loops)
            if (is_revisit) {
                return;
            }

            // Process children
            std::vector<Iterator> children = current.get_children();
            for (size_t i = 0; i < children.size(); ++i) {
                const Iterator& child = children[i];
                walk_impl(child, depth + 1, i, &current);
            }
        };

    // Start traversal from root
    walk_impl(root_iterator, 0, 0, nullptr);
}

/**
 * @brief Walk a DAG in weak topological order using a visitor pattern
 *
 * This function traverses a tree or DAG structure in weak topological order,
 * ensuring that each node is visited after all its dependencies (children) have
 * been processed. This is useful for operations like dependency resolution,
 * bottom-up computation, or post-order processing.
 *
 * The traversal order guarantees that:
 * - Leaf nodes (no children) are visited first
 * - Each node is visited only after all its children have been visited
 * - Shared nodes in DAGs are visited only once, after all paths to them complete
 *
 * Required iterator interface:
 * - std::vector<Iterator> get_children() const
 * - const void* get_node_address() const (for unique node identification)
 *
 * Optional filtering:
 * - bool should_process() const (defaults to true if not provided)
 *
 * @tparam Iterator The iterator type that represents tree/DAG nodes
 * @tparam Visitor A callable that accepts NodeInfo<Iterator>&
 * @param root_iterator The root iterator to start traversal from
 * @param visitor Function called for each node during traversal
 * @param config Configuration for the walking behavior
 */
template <typename Iterator, typename Visitor>
void walk_dag_topological(const Iterator& root_iterator, Visitor&& visitor,
                          const WalkConfig& config = WalkConfig()) {
    std::unordered_set<const void*> visited_nodes;
    std::unordered_set<const void*> completed_nodes;

    // Helper to check if iterator has should_process method
    auto should_process_impl = [&](const Iterator& iter) -> bool {
        if constexpr (has_should_process_v<Iterator>) {
            return iter.should_process();
        } else {
            return true;  // Default behavior if method not provided
        }
    };

    std::function<void(const Iterator&, size_t, size_t, const Iterator*)> walk_impl =
        [&](const Iterator& current, size_t depth, size_t index_from_parent,
            const Iterator* parent) {
            const void* node_key = current.get_node_address();

            // Skip nodes that shouldn't be processed
            if (!should_process_impl(current)) {
                return;
            }

            // If we've already completed this node, don't process it again
            if (completed_nodes.find(node_key) != completed_nodes.end()) {
                return;
            }

            // Check if we're already in the process of visiting this node (cycle detection)
            bool is_revisit = false;
            if (config.track_unique_nodes) {
                if (visited_nodes.find(node_key) != visited_nodes.end()) {
                    // We're in a cycle - mark as revisit but don't recurse
                    is_revisit = true;
                    if (config.call_visitor_on_revisit) {
                        NodeInfo<Iterator> node_info(current, depth, index_from_parent, is_revisit,
                                                     parent);
                        visitor(node_info);
                    }
                    return;
                } else {
                    visited_nodes.insert(node_key);
                }
            }

            // Process all children first (post-order traversal)
            std::vector<Iterator> children = current.get_children();
            for (size_t i = 0; i < children.size(); ++i) {
                const Iterator& child = children[i];
                walk_impl(child, depth + 1, i, &current);
            }

            // Now visit this node after all children have been processed
            NodeInfo<Iterator> node_info(current, depth, index_from_parent, is_revisit, parent);
            visitor(node_info);

            // Mark this node as completed
            if (config.track_unique_nodes) {
                completed_nodes.insert(node_key);
            }
        };

    // Start traversal from root
    walk_impl(root_iterator, 0, 0, nullptr);
}

/**
 * @brief Collect all unique nodes from a DAG or tree structure
 *
 * This function traverses the structure and returns a vector of all unique nodes
 * encountered, useful for operations that need to process each node exactly once.
 *
 * @tparam Iterator The iterator type that represents tree/DAG nodes
 * @param root_iterator The root iterator to start traversal from
 * @param config Configuration for the walking behavior
 * @return std::vector<Iterator> Vector of all unique nodes found
 */
template <typename Iterator>
std::vector<Iterator> collect_unique_nodes(const Iterator& root_iterator,
                                           const WalkConfig& config = WalkConfig()) {
    std::vector<Iterator> unique_nodes;

    walk_dag(
        root_iterator,
        [&](const NodeInfo<Iterator>& node_info) {
            if (!node_info.is_revisit) {
                unique_nodes.push_back(node_info.node);
            }
        },
        config);

    return unique_nodes;
}

/**
 * @brief Collect all edges (parent-child relationships) from a DAG or tree structure
 *
 * This function traverses the structure and returns information about all edges,
 * useful for operations that need to process relationships between nodes.
 *
 * @tparam Iterator The iterator type that represents tree/DAG nodes
 */
template <typename Iterator>
struct EdgeInfo {
    Iterator parent;     ///< Parent node in the relationship
    Iterator child;      ///< Child node in the relationship
    size_t child_index;  ///< Index of child in parent's children vector
    size_t depth;        ///< Depth of the child node

    EdgeInfo(const Iterator& p, const Iterator& c, size_t idx, size_t d)
        : parent(p), child(c), child_index(idx), depth(d) {}
};

/**
 * @brief Collect all edges from a DAG or tree structure
 *
 * @tparam Iterator The iterator type that represents tree/DAG nodes
 * @param root_iterator The root iterator to start traversal from
 * @param config Configuration for the walking behavior
 * @return std::vector<EdgeInfo<Iterator>> Vector of all edges found
 */
template <typename Iterator>
std::vector<EdgeInfo<Iterator>> collect_edges(const Iterator& root_iterator,
                                              const WalkConfig& config = WalkConfig()) {
    std::vector<EdgeInfo<Iterator>> edges;

    walk_dag(
        root_iterator,
        [&](const NodeInfo<Iterator>& node_info) {
            if (node_info.parent && !node_info.is_revisit) {
                edges.emplace_back(*node_info.parent, node_info.node, node_info.index_from_parent,
                                   node_info.depth);
            }
        },
        config);

    return edges;
}

/**
 * @brief Count the total number of nodes in a DAG or tree structure
 *
 * @tparam Iterator The iterator type that represents tree/DAG nodes
 * @param root_iterator The root iterator to start traversal from
 * @param count_unique_only If true, only count unique nodes (default for DAGs)
 * @return size_t Total number of nodes
 */
template <typename Iterator>
size_t count_nodes(const Iterator& root_iterator, bool count_unique_only = true) {
    size_t count = 0;
    WalkConfig config;
    config.track_unique_nodes = count_unique_only;
    config.call_visitor_on_revisit = !count_unique_only;

    walk_dag(
        root_iterator,
        [&](const NodeInfo<Iterator>& node_info) {
            if (!count_unique_only || !node_info.is_revisit) {
                ++count;
            }
        },
        config);

    return count;
}

/**
 * @brief Get the maximum depth of a DAG or tree structure
 *
 * @tparam Iterator The iterator type that represents tree/DAG nodes
 * @param root_iterator The root iterator to start traversal from
 * @return size_t Maximum depth found (root is depth 0)
 */
template <typename Iterator>
size_t get_max_depth(const Iterator& root_iterator) {
    size_t max_depth = 0;

    walk_dag(root_iterator, [&](const NodeInfo<Iterator>& node_info) {
        if (!node_info.is_revisit) {
            max_depth = std::max(max_depth, node_info.depth);
        }
    });

    return max_depth;
}

/**
 * @brief Collect all unique nodes from a DAG or tree in weak topological order
 *
 * This function traverses the structure in weak topological order and returns
 * a vector of all unique nodes, with each node appearing after all its dependencies.
 * This is useful for operations that need to process nodes after their children.
 *
 * @tparam Iterator The iterator type that represents tree/DAG nodes
 * @param root_iterator The root iterator to start traversal from
 * @param config Configuration for the walking behavior
 * @return std::vector<Iterator> Vector of all unique nodes in topological order
 */
template <typename Iterator>
std::vector<Iterator> collect_unique_nodes_topological(const Iterator& root_iterator,
                                                       const WalkConfig& config = WalkConfig()) {
    std::vector<Iterator> unique_nodes;

    walk_dag_topological(
        root_iterator,
        [&](const NodeInfo<Iterator>& node_info) {
            if (!node_info.is_revisit) {
                unique_nodes.push_back(node_info.node);
            }
        },
        config);

    return unique_nodes;
}

/**
 * @brief Collect all edges from a DAG or tree in weak topological order
 *
 * This function traverses the structure in weak topological order and returns
 * information about all edges. The edges are collected as parent nodes are
 * visited (after their children), providing a specific ordering useful for
 * dependency-aware processing.
 *
 * @tparam Iterator The iterator type that represents tree/DAG nodes
 * @param root_iterator The root iterator to start traversal from
 * @param config Configuration for the walking behavior
 * @return std::vector<EdgeInfo<Iterator>> Vector of all edges in topological order
 */
template <typename Iterator>
std::vector<EdgeInfo<Iterator>> collect_edges_topological(const Iterator& root_iterator,
                                                          const WalkConfig& config = WalkConfig()) {
    std::vector<EdgeInfo<Iterator>> edges;

    walk_dag_topological(
        root_iterator,
        [&](const NodeInfo<Iterator>& node_info) {
            if (node_info.parent && !node_info.is_revisit) {
                edges.emplace_back(*node_info.parent, node_info.node, node_info.index_from_parent,
                                   node_info.depth);
            }
        },
        config);

    return edges;
}

}  // namespace dag_walker
