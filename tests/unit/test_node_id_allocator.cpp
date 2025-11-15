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
    int a = 0, b = 0, c = 0;
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
    int x = 0;
    const void* px = &x;
    auto id1 = alloc.get_id(px);
    auto id2 = alloc.get_id(px);
    REQUIRE(id1 == id2);
}

TEST_CASE("node_id_allocator - start index and reset", "[node_id_allocator]") {
    graph_common::node_id_allocator alloc("P", 5);
    int x = 0, y = 0;
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

TEST_CASE("node_id_allocator - nullptr key handling (negative)", "[node_id_allocator][negative]") {
    graph_common::node_id_allocator alloc("Z", 0);

    const void* n1 = nullptr;
    const void* n2 = nullptr;

    // Asking for id for nullptr twice should return the same id
    auto id_first = alloc.get_id(n1);
    auto id_second = alloc.get_id(n2);
    REQUIRE(id_first == id_second);

    // Asking for ids for two different addresses should be different
    int a = 0;
    const void* pa = &a;
    auto id_pa = alloc.get_id(pa);
    REQUIRE(id_pa != id_first);
}
