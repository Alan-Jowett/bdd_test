#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <cudd/cuddObj.hh>
#include <unordered_set>

#include "cudd_view.hpp"
#include "graph.hpp"
#include "graph_concepts.hpp"

TEST_CASE("cudd_view models external_dag_view and preserves complement semantics") {
    static_assert(graph::external_dag_view<cudd_view>);

    // Build simple BDD: f = x0 XOR x1 = (x0 & !x1) | (!x0 & x1)
    // Using Cudd C++ API via BDD wrapper
    Cudd bdd_mgr;
    BDD x0 = bdd_mgr.bddVar(0);
    BDD x1 = bdd_mgr.bddVar(1);

    BDD f = x0.Xor(x1);

    // Get raw DdNode* for the root (CUDD pointer). Use manager from bdd_mgr
    DdNode* root = f.getNode();

    cudd_view view(&bdd_mgr, root);

    // Use graph algorithms to discover nodes and fold values.
    auto roots = view.roots();
    REQUIRE(roots.size() == 1);

    // Topological order from graph::topo_order
    auto order = graph::topo_order(view, roots);
    REQUIRE(!order.empty());

    // Use postorder_fold to compute the set of visited stable_keys
    auto key_set = graph::postorder_fold<cudd_view, std::unordered_set<std::uint64_t>>(
        view, roots[0],
        [](auto const& g, auto h, std::span<const std::unordered_set<std::uint64_t>> child_vals) {
            std::unordered_set<std::uint64_t> s;
            for (auto const& cs : child_vals)
                s.insert(cs.begin(), cs.end());
            s.insert(h.stable_key());
            return s;
        });

    // The number of unique stable keys discovered by folding should equal topo_order size
    REQUIRE(key_set.size() == order.size());

    // Complement check: ensure complemented root is treated as distinct handle
    DdNode* comp_root = Cudd_Not(root);
    cudd_view view2(&bdd_mgr, comp_root);
    auto order2 = graph::topo_order(view2, view2.roots());
    REQUIRE(order2.size() == order.size());

    // Roots must differ (complement bit preserved)
    REQUIRE(view.roots()[0].stable_key() != view2.roots()[0].stable_key());

    // Immediate children of the root should be complemented versions (stable keys differ)
    auto ch = view.children(view.roots()[0]);
    auto ch2 = view2.children(view2.roots()[0]);
    REQUIRE(ch.size() == ch2.size());
    for (size_t i = 0; i < ch.size(); ++i)
        REQUIRE(ch[i].target().stable_key() != ch2[i].target().stable_key());
}
