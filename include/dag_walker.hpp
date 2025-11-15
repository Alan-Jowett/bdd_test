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

#include "graph_iterator_concepts.hpp"

namespace dag_walker {

/**
 * @brief Concept defining the required interface for DAG walker iterators
 *
 * A DAG walker iterator extends the base graph iterator interface.
 * It inherits all the basic requirements and may optionally provide
 * a should_process() method to control whether the node should be processed.
 */
template <typename T>
concept DagWalkerIterator = BaseGraphIterator<T>;

/**
 * @brief Concept to detect if an iterator has a should_process method
 */
template <typename Iterator>
concept has_should_process = requires(Iterator iter) {
    { iter.should_process() } -> std::convertible_to<bool>;
};

/**
 * @brief Configuration for DAG walking behavior
 */
struct WalkConfig {
    bool track_unique_nodes = true;        ///< Whether to track and avoid revisiting unique nodes
    bool call_visitor_on_revisit = false;  ///< Whether to call visitor when revisiting a node
    bool collect_all_edges = false;  ///< Whether to collect ALL edges, even to revisited nodes

    WalkConfig() = default;
    explicit WalkConfig(bool track_unique) : track_unique_nodes(track_unique) {}

    // C++20 spaceship operator for automatic comparison generation
    auto operator<=>(const WalkConfig& other) const = default;
};

/**
 * @brief Information about the current node during traversal
 * @tparam Iterator The iterator type representing the node
 */
template <DagWalkerIterator Iterator>
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
 * @brief Concept defining the required interface for visitor functions
 *
 * A visitor must be callable with a NodeInfo<Iterator> parameter.
 * It can be a function, lambda, function object, or any callable type.
 */
template <typename V, typename Iterator>
concept Visitor = requires(V visitor, const NodeInfo<Iterator>& node_info) { visitor(node_info); };

/**
 * @brief Walk a DAG or tree structure in pre-order using a visitor pattern
 *
 * This function traverses a tree or DAG structure starting from a root iterator,
 * calling a visitor function for each node in pre-order (depth-first, visiting
 * parents before children). The visitor receives detailed information about each
 * node including its position in the traversal.
 *
 * Required iterator interface:
 * - std::vector<Iterator> get_children() const
 * - const void* get_node_address() const (for unique node identification)
 *
 * Optional filtering:
 * - bool should_process() const (defaults to true if not provided)
 *
 * @tparam Iterator The iterator type that represents tree/DAG nodes
 * @tparam V A callable that accepts NodeInfo<Iterator>&
 * @param root_iterator The root iterator to start traversal from
 * @param visitor Function called for each node during traversal
 * @param config Configuration for the walking behavior
 *
 * @requires Iterator must satisfy DagWalkerIterator concept
 * @requires V must satisfy Visitor<V, Iterator> concept
 */
template <DagWalkerIterator Iterator, Visitor<Iterator> V>
void walk_dag_preorder(const Iterator& root_iterator, V&& visitor,
                       const WalkConfig& config = WalkConfig()) {
    static_assert(DagWalkerIterator<Iterator>,
                  "Iterator must satisfy DagWalkerIterator concept for DAG traversal");

    std::unordered_set<const void*> visited_nodes;

    // Helper to check if iterator has should_process method
    auto should_process_impl = [&](const Iterator& iter) -> bool {
        if constexpr (has_should_process<Iterator>) {
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
 * @brief Backward compatibility alias for walk_dag_preorder
 * @deprecated Use walk_dag_preorder instead for clarity
 *
 * @requires Iterator must satisfy DagWalkerIterator concept
 * @requires V must satisfy Visitor<V, Iterator> concept
 */
template <DagWalkerIterator Iterator, Visitor<Iterator> V>
[[deprecated("Use walk_dag_preorder instead for clarity")]]
void walk_dag(const Iterator& root_iterator, V&& visitor, const WalkConfig& config = WalkConfig()) {
    walk_dag_preorder(root_iterator, std::forward<V>(visitor), config);
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
 * @tparam V A callable that accepts NodeInfo<Iterator>&
 * @param root_iterator The root iterator to start traversal from
 * @param visitor Function called for each node during traversal
 * @param config Configuration for the walking behavior
 *
 * @requires Iterator must satisfy DagWalkerIterator concept
 * @requires V must satisfy Visitor<V, Iterator> concept
 */
template <DagWalkerIterator Iterator, Visitor<Iterator> V>
void walk_dag_topological(const Iterator& root_iterator, V&& visitor,
                          const WalkConfig& config = WalkConfig()) {
    static_assert(DagWalkerIterator<Iterator>,
                  "Iterator must satisfy DagWalkerIterator concept for topological traversal");

    std::unordered_set<const void*> visited_nodes;
    std::unordered_set<const void*> completed_nodes;

    // Helper to check if iterator has should_process method
    auto should_process_impl = [&](const Iterator& iter) -> bool {
        if constexpr (has_should_process<Iterator>) {
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
 *
 * @requires Iterator must satisfy DagWalkerIterator concept
 */
template <DagWalkerIterator Iterator>
std::vector<Iterator> collect_unique_nodes(const Iterator& root_iterator,
                                           const WalkConfig& config = WalkConfig()) {
    static_assert(DagWalkerIterator<Iterator>,
                  "Iterator must satisfy DagWalkerIterator concept for node collection");

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
 * @brief Collect all edges (parent-child relationships) from a DAG or tree structure
 *
 * This function traverses the structure and returns information about all edges,
 * useful for operations that need to process relationships between nodes.
 *
 * @tparam Iterator The iterator type that represents tree/DAG nodes
 */
template <DagWalkerIterator Iterator>
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
 *
 * @requires Iterator must satisfy DagWalkerIterator concept
 */
template <DagWalkerIterator Iterator>
std::vector<EdgeInfo<Iterator>> collect_edges(const Iterator& root_iterator,
                                              const WalkConfig& config = WalkConfig()) {
    static_assert(DagWalkerIterator<Iterator>,
                  "Iterator must satisfy DagWalkerIterator concept for edge collection");

    std::vector<EdgeInfo<Iterator>> edges;

    if (config.collect_all_edges) {
        // Special mode: collect ALL edges including those to revisited nodes
        // This ensures we get the same edge set as manual BDD traversal
        std::unordered_set<const void*> processed_nodes;

        std::function<void(const Iterator&, size_t, const Iterator*)> collect_all_edges_impl =
            [&](const Iterator& current, size_t depth, const Iterator* parent) {
                const void* node_key = current.get_node_address();

                // Add edge from parent to current (if parent exists)
                if (parent) {
                    // Find the index of this child in parent's children
                    auto parent_children = parent->get_children();
                    for (size_t i = 0; i < parent_children.size(); ++i) {
                        if (parent_children[i].get_node_address() == node_key) {
                            edges.emplace_back(*parent, current, i, depth);
                            break;
                        }
                    }
                }

                // Stop if we've already processed this node to avoid infinite loops
                if (processed_nodes.find(node_key) != processed_nodes.end()) {
                    return;
                }
                processed_nodes.insert(node_key);

                // Process all children
                auto children = current.get_children();
                for (const auto& child : children) {
                    collect_all_edges_impl(child, depth + 1, &current);
                }
            };

        collect_all_edges_impl(root_iterator, 0, nullptr);
    } else {
        // Original behavior: collect edges during normal traversal
        walk_dag_preorder(
            root_iterator,
            [&](const NodeInfo<Iterator>& node_info) {
                if (node_info.parent && !node_info.is_revisit) {
                    edges.emplace_back(*node_info.parent, node_info.node,
                                       node_info.index_from_parent, node_info.depth);
                }
            },
            config);
    }

    return edges;
}

/**
 * @brief Count the total number of nodes in a DAG or tree structure
 *
 * @tparam Iterator The iterator type that represents tree/DAG nodes
 * @param root_iterator The root iterator to start traversal from
 * @param count_unique_only If true, only count unique nodes (default for DAGs)
 * @return size_t Total number of nodes
 *
 * @requires Iterator must satisfy DagWalkerIterator concept
 */
template <DagWalkerIterator Iterator>
size_t count_nodes(const Iterator& root_iterator, bool count_unique_only = true) {
    size_t count = 0;
    WalkConfig config;
    config.track_unique_nodes = count_unique_only;
    config.call_visitor_on_revisit = !count_unique_only;

    walk_dag_preorder(
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
 *
 * @requires Iterator must satisfy DagWalkerIterator concept
 */
template <DagWalkerIterator Iterator>
size_t get_max_depth(const Iterator& root_iterator) {
    size_t max_depth = 0;

    walk_dag_topological(root_iterator, [&](const NodeInfo<Iterator>& node_info) {
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
 *
 * @requires Iterator must satisfy DagWalkerIterator concept
 */
template <DagWalkerIterator Iterator>
std::vector<Iterator> collect_unique_nodes_topological(const Iterator& root_iterator,
                                                       const WalkConfig& config = WalkConfig()) {
    static_assert(
        DagWalkerIterator<Iterator>,
        "Iterator must satisfy DagWalkerIterator concept for topological node collection");

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
 *
 * @requires Iterator must satisfy DagWalkerIterator concept
 */
template <DagWalkerIterator Iterator>
std::vector<EdgeInfo<Iterator>> collect_edges_topological(const Iterator& root_iterator,
                                                          const WalkConfig& config = WalkConfig()) {
    static_assert(
        DagWalkerIterator<Iterator>,
        "Iterator must satisfy DagWalkerIterator concept for topological edge collection");

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
