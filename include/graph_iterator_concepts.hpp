// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file graph_iterator_concepts.hpp
 * @brief C++20 concepts for graph iterator interfaces
 *
 * This file defines C++20 concepts that specify the requirements for graph iterators
 * used in BDD traversal and visualization. These concepts ensure type safety and
 * provide clear interfaces for template-based graph generation.
 *
 * @author Alan Jowett
 * @date 2025
 * @copyright Copyright (c) 2025 Alan Jowett. Licensed under the MIT License
 */

#ifndef GRAPH_ITERATOR_CONCEPTS_HPP
#define GRAPH_ITERATOR_CONCEPTS_HPP

#include <concepts>
#include <ranges>
#include <utility>

/**
 * @file graph_iterator_concepts.hpp
 * @brief Common concepts for graph iterators used throughout the BDD system
 *
 * This file defines the base concept that all graph iterators must satisfy,
 * along with specialized concepts for specific use cases like DOT generation,
 * Mermaid generation, node tables, and DAG walking.
 */

/**
 * @brief Concept requiring a children range
 *
 * Requires that `T` exposes a `get_children()` member that returns a
 * C++20 range whose `range_value_t` is convertible to `T` (i.e. the
 * range yields elements that are iterator/node-like objects of the same
 * type).
 *
 * This accepts owned containers (for example `std::vector<T>` returned
 * by value), borrowed views (`std::span<T>`, `std::ranges::subrange`, or
 * other borrowed ranges) and range adaptors. If an implementation
 * returns a view into internal storage, it is the implementer's
 * responsibility to ensure the referred storage remains valid for the
 * duration of use.
 */
template <typename T>
concept ChildrenRange = requires(T t) {
    requires std::ranges::range<decltype(t.get_children())>;
    requires std::convertible_to<std::ranges::range_value_t<decltype(t.get_children())>, T>;
};

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
concept BaseGraphIterator = std::copyable<T> && ChildrenRange<T> && requires(T t) {
    // Node identification
    { t.get_node_address() } -> std::convertible_to<const void*>;

    // Comparison operators for container operations
    { t == t } -> std::convertible_to<bool>;
    { t != t } -> std::convertible_to<bool>;
};

#endif  // GRAPH_ITERATOR_CONCEPTS_HPP
