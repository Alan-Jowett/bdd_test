// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett
/**
 * @file teddy_view.hpp
 * @brief TeDDy adapter implementing graph::external_dag_view
 *
 * Provides a lightweight adapter over TeDDy's internal node representation
 * exposing `handle`, `edge`, `children()`, and `roots()` used by generic
 * graph algorithms.
 */

#pragma once

#include <cstdint>
#include <libteddy/core.hpp>
#include <vector>

#include "graph_concepts.hpp"

struct teddy_view {
    using node_t = teddy::bdd_manager::diagram_t::node_t;

    /**
     * @brief Node handle wrapper for TeDDy nodes
     *
     * `stable_key()` returns a stable integer derived from the node pointer
     * and is suitable for hashing/memoization by algorithms.
     */
    struct handle {
        node_t* p = nullptr;
        std::uint64_t stable_key() const noexcept {
            return static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(p));
        }
        const void* debug_address() const noexcept {
            return static_cast<const void*>(p);
        }
        friend bool operator==(const handle& a, const handle& b) noexcept {
            return a.p == b.p;
        }
        friend bool operator!=(const handle& a, const handle& b) noexcept {
            return a.p != b.p;
        }
    };

    /**
     * @brief Edge descriptor returned by `children()`
     */
    struct edge {
        handle tgt;
        int branch = 0;  // 0 = false/low, 1 = true/high
        handle target() const noexcept {
            return tgt;
        }
        int label() const noexcept {
            return branch;
        }
    };

    const teddy::bdd_manager* manager{};
    teddy::bdd_manager::diagram_t diagram;

    teddy_view() = default;
    teddy_view(const teddy::bdd_manager* m, const teddy::bdd_manager::diagram_t& d)
        : manager(m), diagram(d) {}

    /**
     * @brief Return the child edges for node `h`.
     *
     * Returns an empty vector for null or terminal nodes. Binary nodes
     * typically yield two edges (low and high children).
     */
    auto children(handle h) const {
        std::vector<edge> out;
        node_t* n = h.p;
        if (!n || n->is_terminal())
            return out;

        node_t* low = n->get_son(0);
        if (low)
            out.push_back(edge{handle{low}, 0});

        node_t* high = n->get_son(1);
        if (high)
            out.push_back(edge{handle{high}, 1});

        return out;
    }

    /**
     * @brief Return the root handle for the diagram
     */
    auto roots() const {
        return std::vector<handle>{handle{diagram.unsafe_get_root()}};
    }
};

static_assert(graph::external_dag_view<teddy_view>, "teddy_view should model external_dag_view");