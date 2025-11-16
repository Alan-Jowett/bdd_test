// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

#pragma once
#include <concepts>
#include <cstdint>
#include <ranges>
#include <type_traits>

namespace graph {

/**
 * @brief Handle concept for nodes in external DAG views
 *
 * A handle type represents a stable identifier for a node. Requirements:
 * - `stable_key()` returns a value convertible to `std::uint64_t` suitable
 *   for use as a key in memoization/hash maps.
 * - equality (`==`, `!=`) must be provided.
 * - the type must be `std::copyable`.
 *
 * This concept intentionally makes no assumptions about underlying storage
 * (pointer, integer, small-value type) beyond copyability and stable_key().
 */
template <class H>
concept node_handle = std::copyable<H> && requires(H h) {
    { h.stable_key() } -> std::convertible_to<std::uint64_t>;  // memo key
    { h == h } -> std::same_as<bool>;
    { h != h } -> std::same_as<bool>;
};

/**
 * @brief Optional: handle exposes a debug address
 *
 * Some handle types provide a `debug_address()` accessor that returns either
 * a `const void*` or `std::uintptr_t`. This is only used for diagnostics and
 * for producing stable human-readable IDs; it is optional for algorithms.
 * Use `if constexpr (has_debug_address<H>)` to detect availability.
 */
template <class H>
concept has_debug_address = (requires(H h) {
                                { h.debug_address() } -> std::convertible_to<const void*>;
                            }) || (requires(H h) {
                                { h.debug_address() } -> std::convertible_to<std::uintptr_t>;
                            });

/**
 * @brief Edge reference concept
 *
 * Models an edge-like view returned from a node's children range. Minimal
 * requirements:
 * - `e.target()` returns a value convertible to the node handle type `H`.
 * - optionally an edge label may be available via `e.label()`.
 */
template <class E, class H>
concept edge_ref = requires(const E& e) {
    { e.target() } -> std::convertible_to<H>;
    // Optional: edge label/weight (ignored by algorithms that don't need it)
    // { e.label() } -> /* any type */;
};

/**
 * @brief Children range concept
 *
 * A children range is any `std::ranges::input_range` whose `range_value_t`
 * (or reference) models `edge_ref<..., H>`. Implementations are free to
 * return owned containers (e.g. `std::vector<E>`), borrowed views, or
 * adaptor-produced ranges.
 */
template <class R, class H>
concept children_range = std::ranges::input_range<R> && requires(R r) {
    requires edge_ref<std::ranges::range_value_t<R>, H>
                 || edge_ref<std::remove_cvref_t<std::ranges::range_reference_t<R>>, H>;
};

/**
 * @brief External DAG view concept
 *
 * Types that model `external_dag_view` expose a nested `handle` type and
 * provide the following operations:
 * - `g.children(h)` returns a children range yielding elements convertible
 *   to edge-like objects whose `target()` yields a `handle`.
 * - optionally `g.roots()` returns an input-range of root handles.
 *
 * This abstraction decouples algorithms from concrete graph storage
 * implementations and enables generic traversal utilities.
 */
template <class G>
concept external_dag_view = requires(const G& g, typename G::handle h) {
    typename G::handle;
    requires node_handle<typename G::handle>;
    // Children of a node as a range of edges:
    requires std::ranges::input_range<decltype(g.children(h))>;
    requires children_range<decltype(g.children(h)), typename G::handle>;
    // Roots are optional; many algorithms accept an explicit root list
    requires std::ranges::input_range<decltype(g.roots())>;  // optional but nice to have
};

}  // namespace graph