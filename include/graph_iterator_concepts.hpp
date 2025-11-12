#ifndef GRAPH_ITERATOR_CONCEPTS_HPP
#define GRAPH_ITERATOR_CONCEPTS_HPP

#include <concepts>
#include <string>
#include <vector>

/**
 * @file graph_iterator_concepts.hpp
 * @brief Common concepts for graph iterators used throughout the BDD system
 *
 * This file defines the base concept that all graph iterators must satisfy,
 * along with specialized concepts for specific use cases like DOT generation,
 * Mermaid generation, node tables, and DAG walking.
 */

/**
 * @brief Base concept defining the core interface for all graph iterators
 *
 * This concept captures the fundamental requirements that all iterators
 * in the BDD system must satisfy. It provides the minimum interface needed
 * for graph traversal and node identification.
 *
 * All specialized iterator concepts (DotGraphIterator, MermaidGraphIterator,
 * NodeTableIterator, DagWalkerIterator) build upon this base concept.
 */
template <typename T>
concept BaseGraphIterator = requires(T t) {
    // Core traversal interface
    { t.get_children() } -> std::convertible_to<std::vector<T>>;
    { t.get_node_address() } -> std::convertible_to<const void*>;

    // Comparison operators for container operations
    { t == t } -> std::convertible_to<bool>;
    { t != t } -> std::convertible_to<bool>;

    // Must be copyable for storage in containers and function parameters
    requires std::copyable<T>;
};

#endif  // GRAPH_ITERATOR_CONCEPTS_HPP