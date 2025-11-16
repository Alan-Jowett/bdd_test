// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

#pragma once
#include <any>
#include <functional>
#include <queue>
#include <span>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "graph_concepts.hpp"

namespace graph {

/**
 * @brief Compute a topological order of nodes reachable from `roots`
 *
 * Performs a BFS discovery followed by Kahn's algorithm to produce a weak
 * topological ordering of the subgraph reachable from the provided `roots`.
 *
 * @tparam G A type modeling `external_dag_view`.
 * @param g The DAG view.
 * @param roots An input-range of root handles to start discovery from.
 * @return A `std::vector` of handles in topological order.
 * @throws std::logic_error if a cycle is detected.
 *
 * Complexity: linear in the number of discovered nodes plus edges.
 */
template <external_dag_view G>
std::vector<typename G::handle> topo_order(const G& g, std::ranges::input_range auto roots) {
    /**
     * Implied concepts:
     * - `typename G::handle` must model `graph::node_handle` (stable_key(), equality).
     * - `decltype(g.children(h))` must model `graph::children_range` whose
     *   element type models `graph::edge_ref`.
     * - `graph::has_debug_address` is optional and not required by these
     *   algorithms; it can be used by callers for diagnostics when available.
     */
    using H = typename G::handle;
    static_assert(graph::node_handle<H>, "G::handle must model graph::node_handle");

    std::unordered_map<std::uint64_t, std::size_t> indeg;
    std::vector<H> order;

    // 1) Build indegrees by a forward sweep from roots
    std::vector<H> frontier(std::begin(roots), std::end(roots));
    std::unordered_map<std::uint64_t, H> seen;

    auto add = [&](H h) { seen.emplace(h.stable_key(), h); };
    for (auto& r : frontier)
        add(r);

    // BFS to discover nodes and indegrees
    for (std::size_t i = 0; i < frontier.size(); ++i) {
        H u = frontier[i];
        for (auto&& e : g.children(u)) {
            H v = e.target();
            ++indeg[v.stable_key()];
            if (!seen.count(v.stable_key())) {
                add(v);
                frontier.push_back(v);
            }
        }
    }

    // 2) Kahn’s algorithm
    std::queue<H> q;
    for (auto& [k, h] : seen) {
        if (!indeg.count(k))
            q.push(h);  // indegree 0
    }

    while (!q.empty()) {
        H u = q.front();
        q.pop();
        order.push_back(u);
        for (auto&& e : g.children(u)) {
            H v = e.target();
            auto& d = indeg[v.stable_key()];
            if (--d == 0)
                q.push(v);
        }
    }

    if (order.size() != seen.size())
        throw std::logic_error("cycle detected (not a DAG)");

    return order;
}

/**
 * @brief Post-order fold over the DAG with memoization
 *
 * Performs a memoized post-order traversal (children-first) starting at
 * `root`. For each visited node the provided `combine` function is invoked
 * with the graph view, the node handle, and a `std::span` of child results in
 * canonical post-order. The `combine` must return a value convertible to
 * `R`.
 *
 * @tparam G A type modeling `external_dag_view`.
 * @tparam R The result type produced by `combine`.
 * @tparam Combine Callable type: `R(const G&, typename G::handle, std::span<const R>)`.
 * @param g The DAG view.
 * @param root The starting node handle.
 * @param combine A callable that computes the node result from child results.
 * @return The folded result of type `R` for the root node.
 */
template <external_dag_view G, typename R, class Combine>
R postorder_fold(const G& g, typename G::handle root, Combine&& combine) {
    /**
     * Implied concepts and notes:
     * - `typename G::handle` must model `graph::node_handle`.
     * - `g.children(h)` must be a `graph::children_range` producing elements
     *   that model `graph::edge_ref`.
     * - If `graph::has_debug_address<H>` is available, callers may use it for
     *   diagnostics; algorithms rely on `stable_key()` for memoization.
     */
    using H = typename G::handle;
    static_assert(graph::node_handle<H>, "G::handle must model graph::node_handle");
    std::unordered_map<std::uint64_t, R> memo;

    std::function<R(H)> dfs = [&](H u) -> R {
        auto k = u.stable_key();
        if (auto it = memo.find(k); it != memo.end())
            return it->second;

        std::vector<R> child_vals;
        for (auto&& e : g.children(u))
            child_vals.push_back(dfs(e.target()));

        R r = combine(g, u, std::span<const R>(child_vals));
        memo.emplace(k, r);
        return r;
    };

    return dfs(root);
}
}  // namespace graph