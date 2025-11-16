// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett
/**
 * @file expression_view.hpp
 * @brief Adapter exposing the expression AST as a graph::external_dag_view
 *
 * This adapter provides a thin view over the variant-based AST defined in
 * `expression_types.hpp`. It models `graph::external_dag_view` by providing
 * nested `handle`, `edge`, and the `children()` / `roots()` methods used by
 * generic graph algorithms in `graph.hpp`.
 */

#pragma once

#include <cstdint>
#include <optional>
#include <variant>
#include <vector>

#include "expression_types.hpp"
#include "graph.hpp"
#include "graph_concepts.hpp"

struct expression_view {
    using expr_t = my_expression;

    /**
     * @brief Opaque handle representing a pointer into the expression AST
     *
     * `stable_key()` returns a stable integer derived from the pointer value
     * and is used by graph algorithms for memoization and hashing. The
     * `debug_address()` method is provided for optional diagnostics and may be
     * used when available.
     */
    struct handle {
        const expr_t* p = nullptr;
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
     *
     * The `branch` integer encodes a lightweight branch label: for binary
     * operators 0==left, 1==right; for unary operators the branch is 0.
     */
    struct edge {
        handle tgt;
        int branch = 0;  // 0 = left/first, 1 = right/second (for binary ops)
        handle target() const noexcept {
            return tgt;
        }
        int label() const noexcept {
            return branch;
        }
    };

    /// Pointer to the root expression node for this view
    const expr_t* root = nullptr;

    expression_view() = default;
    explicit expression_view(const expr_t& e) : root(&e) {}
    explicit expression_view(const expr_t* p) : root(p) {}

    /**
     * @brief Return the outgoing edges for node `h`
     *
     * Returns an owned `std::vector<edge>` containing the logical children of
     * the expression node. Binary operators yield two edges (when both
     * children exist); `my_not` yields a single child; `my_variable` yields no
     * children.
     *
     * @param h Node handle for which children are requested
     * @return `std::vector<edge>` listing outgoing edges
     */
    auto children(handle h) const {
        std::vector<edge> out;
        if (!h.p)
            return out;

        std::visit(
            [&](auto const& v) {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, my_and> || std::is_same_v<T, my_or>
                              || std::is_same_v<T, my_xor>) {
                    if (v.left)
                        out.push_back(edge{handle{v.left.get()}, 0});
                    if (v.right)
                        out.push_back(edge{handle{v.right.get()}, 1});
                } else if constexpr (std::is_same_v<T, my_not>) {
                    if (v.expr)
                        out.push_back(edge{handle{v.expr.get()}, 0});
                } else if constexpr (std::is_same_v<T, my_variable>) {
                    // no children
                }
            },
            *h.p);

        return out;
    }

    /**
     * @brief Return the root handles for the view
     *
     * The adapter returns a single-element vector containing the root
     * expression pointer wrapped in a `handle`.
     */
    auto roots() const {
        return std::vector<handle>{handle{root}};
    }
};

static_assert(graph::external_dag_view<expression_view>,
              "expression_view should model external_dag_view");
