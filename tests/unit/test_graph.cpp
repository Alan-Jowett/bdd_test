// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file test_graph.hpp
 * @brief Unit tests for graph concepts and utilities.
 *
 * @author Alan Jowett
 * @date 2025
 */

#include <catch2/catch_all.hpp>
#include <cstdint>
#include <vector>

#include "graph.hpp"
#include "graph_iterator_concepts.hpp"

// Storage:
struct AdjDAG {
    struct Edge {
        std::uint32_t to;
        float w;
    };
    std::vector<std::vector<Edge>> out;  // 0..N-1
    std::vector<std::uint32_t> roots_;
};

// Handle & EdgeRef:
struct AdjHandle {
    std::uint32_t id{};
    std::uint64_t stable_key() const {
        return id;
    }
    const void* debug_address() const {
        return nullptr;
    }
    friend bool operator==(AdjHandle a, AdjHandle b) {
        return a.id == b.id;
    }
    friend bool operator!=(AdjHandle a, AdjHandle b) {
        return !(a == b);
    }
};

struct AdjEdge {
    AdjHandle tgt;
    float w;
    AdjHandle target() const {
        return tgt;
    }
    float label() const {
        return w;
    }
};

struct AdjView {
    using handle = AdjHandle;

    const AdjDAG* g{};
    auto children(handle h) const {
        std::vector<AdjEdge> outv;
        const auto& src = g->out[h.id];
        outv.reserve(src.size());
        for (const auto& e : src)
            outv.push_back(AdjEdge{AdjHandle{e.to}, e.w});
        return outv;
    }
    auto roots() const {
        std::vector<AdjHandle> rv;
        rv.reserve(g->roots_.size());
        for (auto x : g->roots_)
            rv.push_back(AdjHandle{x});
        return rv;
    }
};
static_assert(graph::external_dag_view<AdjView>, "AdjView should model external_dag_view");

// Test topo_order:
TEST_CASE("topo_order on AdjView", "[graph][topo_order]") {
    AdjDAG dag;
    dag.out = {
        {{1, 1.0f}, {2, 2.0f}},  // 0 -> 1,2
        {{3, 1.0f}},             // 1 -> 3
        {{3, 1.0f}},             // 2 -> 3
        {}                       // 3 ->
    };
    dag.roots_ = {0};

    AdjView view{&dag};
    auto order = graph::topo_order(view, view.roots());

    REQUIRE(order.size() == 4);
    REQUIRE(order[0].id == 0);
    REQUIRE((order[1].id == 1 || order[1].id == 2));
    REQUIRE((order[2].id == 1 || order[2].id == 2));
    REQUIRE(order[1].id != order[2].id);
    REQUIRE(order[3].id == 3);
}

// Test postorder_fold:
TEST_CASE("postorder_fold on AdjView", "[graph][postorder_fold]") {
    AdjDAG dag;
    dag.out = {
        {{1, 1.0f}, {2, 2.0f}},  // 0 -> 1,2
        {{3, 1.0f}},             // 1 -> 3
        {{3, 1.0f}},             // 2 -> 3
        {}                       // 3 ->
    };
    dag.roots_ = {0};
    AdjView view{&dag};
    auto sum_weights = graph::postorder_fold<AdjView, float>(
        view, AdjHandle{0}, [](const AdjView& g, AdjHandle h, std::span<const float> child_sums) {
            float total = 0.0f;
            for (const auto& e : g.children(h)) {
                total += e.label();  // edge weight
            }
            for (float cs : child_sums) {
                total += cs;  // sum from children
            }
            return total;
        });
    REQUIRE(sum_weights == Catch::Approx(5.0f));  // 1.0 + 2.0 + 1.0 + 1.0
}
