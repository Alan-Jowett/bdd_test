// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file test_example.cpp
 * @brief Example unit tests demonstrating Catch2 usage patterns
 *
 * This file shows example test patterns and best practices for the BDD project.
 * These are demonstration tests only - replace with actual tests as needed.
 */

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

// Example test demonstrating basic Catch2 features
TEST_CASE("Example basic functionality", "[example]") {
    SECTION("String operations") {
        std::string test_string = "BDD_Demo";

        REQUIRE(test_string.length() == 8);
        REQUIRE(test_string.find("BDD") == 0);
        REQUIRE(test_string.ends_with("Demo"));
    }

    SECTION("Vector operations") {
        std::vector<int> numbers = {1, 2, 3, 4, 5};

        REQUIRE(numbers.size() == 5);
        REQUIRE(numbers.front() == 1);
        REQUIRE(numbers.back() == 5);

        // Demonstrate CHECK vs REQUIRE
        for (size_t i = 0; i < numbers.size(); ++i) {
            CHECK(numbers[i] == static_cast<int>(i + 1));
        }
    }
}

// Example test demonstrating exception handling
TEST_CASE("Example exception handling", "[example]") {
    SECTION("Exception throwing") {
        auto throw_something = []() { throw std::runtime_error("Test exception"); };

        REQUIRE_THROWS(throw_something());
        REQUIRE_THROWS_AS(throw_something(), std::runtime_error);
        // Note: REQUIRE_THROWS_WITH requires different headers in some Catch2 versions
        // REQUIRE_THROWS_WITH(throw_something(), "Test exception");
    }

    SECTION("No exception throwing") {
        auto safe_function = []() { return 42; };

        REQUIRE_NOTHROW(safe_function());
        REQUIRE(safe_function() == 42);
    }
}

// Example demonstrating test tags
TEST_CASE("Example performance test", "[example][performance]") {
    // This test could measure performance of critical operations
    std::vector<int> large_vector(10000, 42);

    REQUIRE(large_vector.size() == 10000);
    REQUIRE(
        std::all_of(large_vector.begin(), large_vector.end(), [](int val) { return val == 42; }));
}

// Example demonstrating edge cases
TEST_CASE("Example edge cases", "[example][edge_case]") {
    SECTION("Empty containers") {
        std::vector<int> empty_vector;
        std::string empty_string;

        REQUIRE(empty_vector.empty());
        REQUIRE(empty_string.empty());
    }

    SECTION("Boundary values") {
        REQUIRE(0 >= 0);      // Minimum boundary
        REQUIRE(100 <= 100);  // Maximum boundary
    }
}