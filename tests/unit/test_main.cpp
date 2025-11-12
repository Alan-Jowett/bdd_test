// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file test_main.cpp
 * @brief Main entry point for Catch2 unit tests
 *
 * This file serves as the main entry point for all Catch2 unit tests.
 * It uses Catch2's provided main function, so individual test files
 * only need to include catch2/catch_test_macros.hpp and define their tests.
 *
 * To add new test files:
 * 1. Create a new .cpp file in the unit/ directory
 * 2. Include catch2/catch_test_macros.hpp
 * 3. Add the new file to the unit_tests target in CMakeLists.txt
 * 4. Write your tests using Catch2 macros like TEST_CASE, SECTION, etc.
 *
 * Example test file structure:
 *
 * ```cpp
 * #include <catch2/catch_test_macros.hpp>
 * #include "your_header.hpp"
 *
 * TEST_CASE("Description of what you're testing", "[tag]") {
 *     SECTION("Specific scenario") {
 *         // Setup
 *         // Test
 *         // Assertions using REQUIRE or CHECK
 *     }
 * }
 * ```
 */

// This file intentionally left empty - Catch2::Catch2WithMain provides the main function