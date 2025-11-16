// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett
/**
 * @file cudd_view.hpp
 * @brief Adapter exposing a CUDD DdNode graph as a graph::external_dag_view
 *
 * This adapter wraps raw CUDD pointers (DdNode*) and presents a small
 * view type that models `graph::external_dag_view` used by the generic graph
 * algorithms. The adapter preserves CUDD complement-edge semantics by
 * returning child handles that include the complement bit when appropriate.
 */
#pragma once

#include <cudd/cudd.h>

#include <cstdint>
#include <cudd/cuddObj.hh>
#include <vector>

#include "graph_concepts.hpp"

/**
 * @brief CUDD-backed external_dag_view adapter
 *
 * Exposes a minimal view over CUDD `DdNode*` values. The nested types
 * `handle` and `edge` model the `graph::node_handle` and `graph::edge_ref`
 * concepts respectively. `children()` returns a small `std::vector<edge>` to
 * simplify iterator/range compatibility across compilers.
 */
struct cudd_view {
    /**
     * @brief Lightweight handle wrapper for a CUDD node pointer
     *
     * `stable_key()` encodes the raw pointer value (including the
     * complement bit) so that logically distinct addressed nodes remain
     * distinct for memoization and hashing in algorithms.
     */
    struct handle {
        DdNode* p = nullptr;

        std::uint64_t stable_key() const noexcept {
            return static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(p));
        }

        /**
         * @brief Optional debug pointer for diagnostics
         * @return pointer value suitable for printing or address comparisons
         */
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
     * @brief Edge reference returned by `children()`
     *
     * `label()` encodes branch semantics: 0 == else/low, 1 == then/high.
     */
    struct edge {
        handle tgt;
        int branch = 0;  // 0 = else/low, 1 = then/high
        handle target() const noexcept {
            return tgt;
        }
        int label() const noexcept {
            return branch;
        }
    };

    /// CUDD manager pointer used for helper functions (may be nullptr for some tests)
    const Cudd* manager = nullptr;
    /// The root DdNode pointer for this view
    DdNode* root = nullptr;

    cudd_view() = default;
    cudd_view(const Cudd* m, DdNode* r) : manager(m), root(r) {}

    /**
     * @brief Return child edges for node handle `h`
     *
     * Returns an empty vector for null or constant nodes. For non-constant
     * nodes the `low` and `high` children are returned. If the supplied
     * handle pointer is complemented, the complement bit is propagated to
     * the returned children so callers can treat handles as opaque stable
     * identifiers.
     *
     * @param h Node handle to query
     * @return small vector of `edge` describing the outgoing children
     */
    auto children(handle h) const {
        std::vector<edge> out;
        DdNode* node = h.p;
        if (!node)
            return out;

        DdNode* regular = Cudd_Regular(node);
        if (Cudd_IsConstant(regular))
            return out;

        DdNode* low = Cudd_E(regular);
        DdNode* high = Cudd_T(regular);

        // If the parent pointer was complemented, children must be complemented
        if (Cudd_IsComplement(node)) {
            low = Cudd_Not(low);
            high = Cudd_Not(high);
        }

        if (low)
            out.push_back(edge{handle{low}, 0});
        if (high)
            out.push_back(edge{handle{high}, 1});

        return out;
    }

    /**
     * @brief Return the view roots as a small vector of handles
     */
    auto roots() const {
        return std::vector<handle>{handle{root}};
    }
};

static_assert(graph::external_dag_view<cudd_view>, "cudd_view should model external_dag_view");
