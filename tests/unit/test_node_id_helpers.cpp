#include <catch2/catch_test_macros.hpp>

// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file tests/unit/test_node_id_helpers.cpp
 * @brief Tests for node id helper functions
 *
 * Validates numeric suffix parsing and numeric-aware comparison used for
 * ordering node ids naturally (e.g., N2 < N10).
 */

#include <catch2/catch_test_macros.hpp>

#include "node_id_allocator.hpp"
#include "node_id_helpers.hpp"

TEST_CASE("node_id_numeric parses numeric suffixes correctly", "[node_id_helpers]") {
    using graph_common::node_id_numeric;

    REQUIRE(node_id_numeric("N1") == 1);
    REQUIRE(node_id_numeric("N10") == 10);
    REQUIRE(node_id_numeric("N001") == 1);
    REQUIRE(node_id_numeric("Nabc") == 0);  // non-numeric suffix
    REQUIRE(node_id_numeric("X") == 0);     // only one char
    REQUIRE(node_id_numeric("") == 0);      // empty string
    REQUIRE(node_id_numeric("10") == 0);    // no leading char considered
}

TEST_CASE("node_id_less performs numeric-aware comparison", "[node_id_helpers]") {
    using graph_common::node_id_less;

    REQUIRE(node_id_less("N2", "N10") == true);
    REQUIRE(node_id_less("N10", "N2") == false);
    REQUIRE(node_id_less("N0", "N0") == false);
    REQUIRE(node_id_less("10", "N1") == true);  // 0 < 1
}

TEST_CASE("node_id_helpers - negative: empty prefix and nullptr handling",
          "[node_id_helpers][negative]") {
    // Empty prefix: using allocator with empty prefix will produce IDs like "3"
    graph_common::node_id_allocator alloc_prefix("", 3);
    int dummy = 0;
    auto formatted = alloc_prefix.get_id(&dummy);
    // formatted should at least contain the numeric index
    REQUIRE(formatted.find("3") != std::string::npos);

    // node_id_numeric expects a leading char; with empty prefix it will return 0
    REQUIRE(graph_common::node_id_numeric(formatted) == 0);

    // Now test nullptr handling via allocator: nullptr keys may collide
    graph_common::node_id_allocator alloc_empty("", 0);
    const void* nnull = nullptr;
    auto id1 = alloc_empty.get_id(nnull);
    auto id2 = alloc_empty.get_id(nnull);
    REQUIRE(id1 == id2);

    // Ensure that a real pointer differs from nullptr id
    int x = 42;
    const void* px = &x;
    auto id_px = alloc_empty.get_id(px);
    REQUIRE(id_px != id1);
}
