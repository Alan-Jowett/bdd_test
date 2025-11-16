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
 * @brief Information about the current node during traversal
 * @tparam Iterator The iterator type representing the node
 */
template <DagWalkerIterator Iterator>
struct NodeInfo {
    const Iterator& node;      ///< The current node iterator
    size_t index_from_parent;  ///< Index in parent's children vector
    const Iterator* parent;    ///< Parent node (nullptr for root)

    NodeInfo(const Iterator& n, size_t idx, const Iterator* p = nullptr)
        : node(n), index_from_parent(idx), parent(p) {}
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
 *
 * @requires Iterator must satisfy DagWalkerIterator concept
 * @requires V must satisfy Visitor<V, Iterator> concept
 */
template <DagWalkerIterator Iterator, Visitor<Iterator> V>
void walk_dag_topological_order(const Iterator& root_iterator, V&& visitor) {
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

    std::function<void(const Iterator&, size_t, const Iterator*)> walk_impl =
        [&](const Iterator& current, size_t index_from_parent, const Iterator* parent) {
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
            if (visited_nodes.find(node_key) != visited_nodes.end()) {
                // We're in a cycle - treat as a revisit but don't recurse
                NodeInfo<Iterator> node_info(current, index_from_parent, parent);
                visitor(node_info);
                return;
            }
            visited_nodes.insert(node_key);

            // Process all children first (post-order traversal)
            std::vector<Iterator> children = current.get_children();
            for (size_t i = 0; i < children.size(); ++i) {
                const Iterator& child = children[i];
                walk_impl(child, i, &current);
            }

            // Now visit this node after all children have been processed
            NodeInfo<Iterator> node_info(current, index_from_parent, parent);
            visitor(node_info);

            // Mark this node as completed
            completed_nodes.insert(node_key);
        };

    // Start traversal from root
    walk_impl(root_iterator, 0, nullptr);
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

    EdgeInfo(const Iterator& p, const Iterator& c, size_t idx)
        : parent(p), child(c), child_index(idx) {}
};

/**
 * @brief Collect nodes and edges from a DAG or tree in weak topological order
 *
 * This performs a single traversal and returns both the list of nodes (in
 * topological order) and the list of edges (parent->child) discovered while
 * visiting parents. This avoids doing two separate traversals when callers
 * need both nodes and edges.
 *
 * @tparam Iterator The iterator type that represents tree/DAG nodes
 * @param root_iterator The root iterator to start traversal from
 * @return pair of (nodes vector, edges vector)
 */
template <DagWalkerIterator Iterator>
std::pair<std::vector<Iterator>, std::vector<EdgeInfo<Iterator>>>
collect_nodes_and_edges_topological(const Iterator& root_iterator) {
    static_assert(DagWalkerIterator<Iterator>,
                  "Iterator must satisfy DagWalkerIterator concept for topological collection");

    std::vector<Iterator> nodes;
    std::vector<EdgeInfo<Iterator>> edges;

    // Use the existing walk_dag_topological_order implementation to ensure
    // cycle handling and should_process behavior remain consistent.
    walk_dag_topological_order(root_iterator, [&](const NodeInfo<Iterator>& info) {
        // Record the node in topological order
        nodes.push_back(info.node);

        // Record edges observed for this parent
        auto children = info.node.get_children();
        for (size_t i = 0; i < children.size(); ++i) {
            edges.emplace_back(info.node, children[i], i);
        }
    });

    return {nodes, edges};
}

/**
 * @brief Count the number of nodes reachable from a root in weak topological order
 *
 * This helper walks the DAG using the existing topological walker and increments
 * a counter for each node that would be processed. It respects the
 * `should_process()` filtering if provided by the iterator and uses the
 * centralized traversal logic (cycle detection, visitation tracking).
 *
 * Using this avoids allocating and filling a vector when callers only need
 * the total number of nodes.
 *
 * @tparam Iterator The iterator type that represents tree/DAG nodes
 * @param root_iterator The root iterator to start traversal from
 * @return size_t Number of nodes processed/collected
 */
template <DagWalkerIterator Iterator>
size_t count_nodes_topological(const Iterator& root_iterator) {
    size_t counter = 0;
    walk_dag_topological_order(root_iterator, [&](const NodeInfo<Iterator>& info) {
        (void)info;  // keep unused-param quiet in release builds
        ++counter;
    });
    return counter;
}

}  // namespace dag_walker
