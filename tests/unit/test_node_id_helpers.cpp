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
