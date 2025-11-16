// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

#include <catch2/catch_all.hpp>
#include <libteddy/core.hpp>

#include "graph.hpp"
#include "teddy_test_utils.hpp"
#include "teddy_view.hpp"

TEST_CASE("teddy_view models external_dag_view and integrates with algorithms", "[teddy][view]") {
    using namespace teddy::ops;

    // Create a small TeDDy manager with 2 variables
    teddy::bdd_manager manager(2, 1000);

    // Build a simple BDD: f = x0 AND (NOT x1)  i.e. x0 & !x1
    auto x0 = manager.variable(0);
    auto x1 = manager.variable(1);
    auto not_x1 = manager.apply<XOR>(x1, manager.constant(1));
    auto f = manager.apply<AND>(x0, not_x1);

    teddy_view view(&manager, f);

    // Concept check
    STATIC_REQUIRE(graph::external_dag_view<teddy_view>);

    // roots should contain the root handle
    auto roots = view.roots();
    REQUIRE(!roots.empty());

    // topo_order should return a vector of handles
    auto order = graph::topo_order(view, roots);
    REQUIRE(!order.empty());

    // postorder_fold: count variable nodes (non-terminals)
    auto count = graph::postorder_fold<teddy_view, int>(
        view, order[0],
        [](const teddy_view& g, teddy_view::handle h, std::span<const int> child_sums) {
            int sum = 0;
            for (auto&& e : g.children(h)) {
                // label isn't used here
            }
            for (int v : child_sums)
                sum += v;
            // if node is non-terminal count 1, else 0
            return h.p && !h.p->is_terminal() ? (1 + sum) : sum;
        });

    REQUIRE(count == 2);
}
