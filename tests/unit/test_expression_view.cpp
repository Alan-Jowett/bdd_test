#include <catch2/catch_all.hpp>
#include <cstdint>
#include <unordered_set>

#include "expression_view.hpp"
#include "graph.hpp"

TEST_CASE("expression_view models external_dag_view and algorithms work", "[expression_view]") {
    using view_t = expression_view;

    // build AST: (a AND b) OR (NOT c)
    auto left_leaf = std::make_unique<my_expression>(my_variable{"a"});
    auto right_leaf = std::make_unique<my_expression>(my_variable{"b"});
    auto mid_and =
        std::make_unique<my_expression>(my_and{std::move(left_leaf), std::move(right_leaf)});

    auto not_leaf = std::make_unique<my_expression>(my_variable{"c"});
    auto mid_not = std::make_unique<my_expression>(my_not{std::move(not_leaf)});

    auto root = my_expression{my_or{std::move(mid_and), std::move(mid_not)}};

    view_t v(&root);

    STATIC_REQUIRE(graph::external_dag_view<view_t>);

    // collect topo order
    auto topo = graph::topo_order(v, v.roots());
    REQUIRE(!topo.empty());

    // collect postorder fold: produce set of stable_keys
    auto folded = graph::postorder_fold<view_t, std::unordered_set<std::uint64_t>>(
        v, v.roots().front(),
        [](const view_t& /*g*/, const view_t::handle& h,
           std::span<const std::unordered_set<std::uint64_t>> child_keys) {
            std::unordered_set<std::uint64_t> acc;
            acc.insert(h.stable_key());
            for (auto const& ck : child_keys)
                for (auto k : ck)
                    acc.insert(k);
            return acc;
        });

    // all topo nodes' stable keys should be in the folded set
    for (auto const& h : topo) {
        REQUIRE(folded.find(h.stable_key()) != folded.end());
    }

    // verify children mapping for the root: should have two children
    auto roots = v.roots();
    REQUIRE(roots.size() == 1);
    auto ch = v.children(roots.front());
    REQUIRE(ch.size() == 2);
}
