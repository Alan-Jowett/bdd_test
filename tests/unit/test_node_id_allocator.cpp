// SPDX-License-Identifier: MIT
// Unit tests for `graph_common::node_id_allocator`.
/**
 * @file tests/unit/test_node_id_allocator.cpp
 * @brief Tests for the node id allocator utility
 *
 * Verifies unique id generation, prefix handling, stable mapping for keys, and
 * reset/start-index behaviors expected from the allocator.
 */

#include <catch2/catch_test_macros.hpp>
#include <set>

#include "node_id_allocator.hpp"

TEST_CASE("node_id_allocator - unique ids and prefix", "[node_id_allocator]") {
    graph_common::node_id_allocator alloc("X", 1);
    int a, b, c;
    const void* pa = &a;
    const void* pb = &b;
    const void* pc = &c;

    auto ida = alloc.get_id(pa);
    auto idb = alloc.get_id(pb);
    auto idc = alloc.get_id(pc);

    REQUIRE(ida != idb);
    REQUIRE(idb != idc);
    REQUIRE(ida.rfind("X", 0) == 0);  // starts with prefix X
}

TEST_CASE("node_id_allocator - stable ids for same key", "[node_id_allocator]") {
    graph_common::node_id_allocator alloc("N", 0);
    int x;
    const void* px = &x;
    auto id1 = alloc.get_id(px);
    auto id2 = alloc.get_id(px);
    REQUIRE(id1 == id2);
}

TEST_CASE("node_id_allocator - start index and reset", "[node_id_allocator]") {
    graph_common::node_id_allocator alloc("P", 5);
    int x, y;
    const void* px = &x;
    const void* py = &y;
    auto id1 = alloc.get_id(px);
    auto id2 = alloc.get_id(py);
    REQUIRE(id1 != id2);
    // Reset should clear and next ID should return to start index (5)
    alloc.reset();
    auto id3 = alloc.get_id(px);
    REQUIRE(id3 == std::format("P{}", 5));
}
