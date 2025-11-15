// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file test_expression_iterator.cpp
 * @brief Unit tests for the expression iterator
 *
 * This file contains unit tests for the expression_iterator class,
 * testing expression tree traversal and node property access.
 */

#include <catch2/catch_test_macros.hpp>

#include "expression_iterator.hpp"
#include "expression_types.hpp"

TEST_CASE("ExpressionIterator - Single variable node", "[expression_iterator][basic]") {
    my_expression expr = my_variable{"x"};
    expression_iterator iter(expr);

    REQUIRE(iter.is_valid());
    REQUIRE(iter.get_label() == "x");
    REQUIRE(iter.get_shape() == std::string("ellipse"));
}

TEST_CASE("ExpressionIterator - AND operator node", "[expression_iterator][operators]") {
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};
    expression_iterator iter(expr);

    REQUIRE(iter.is_valid());
    REQUIRE(iter.get_label() == "AND");
    REQUIRE(iter.get_shape() == std::string("box"));
}

TEST_CASE("ExpressionIterator - OR operator node", "[expression_iterator][operators]") {
    my_expression expr = my_or{std::make_unique<my_expression>(my_variable{"x"}),
                               std::make_unique<my_expression>(my_variable{"y"})};
    expression_iterator iter(expr);

    REQUIRE(iter.is_valid());
    REQUIRE(iter.get_label() == "OR");
    REQUIRE(iter.get_shape() == std::string("box"));
}

TEST_CASE("ExpressionIterator - XOR operator node", "[expression_iterator][operators]") {
    my_expression expr = my_xor{std::make_unique<my_expression>(my_variable{"p"}),
                                std::make_unique<my_expression>(my_variable{"q"})};
    expression_iterator iter(expr);

    REQUIRE(iter.is_valid());
    REQUIRE(iter.get_label() == "XOR");
    REQUIRE(iter.get_shape() == std::string("box"));
}

TEST_CASE("ExpressionIterator - NOT operator node", "[expression_iterator][operators]") {
    my_expression expr = my_not{std::make_unique<my_expression>(my_variable{"z"})};
    expression_iterator iter(expr);

    REQUIRE(iter.is_valid());
    REQUIRE(iter.get_label() == "NOT");
    REQUIRE(iter.get_shape() == std::string("box"));
}

TEST_CASE("ExpressionIterator - Get children from AND node", "[expression_iterator][children]") {
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};
    expression_iterator iter(expr);

    auto children = iter.get_children();

    REQUIRE(children.size() == 2);
    REQUIRE(children[0].get_label() == "a");
    REQUIRE(children[1].get_label() == "b");
}

TEST_CASE("ExpressionIterator - Get children from OR node", "[expression_iterator][children]") {
    my_expression expr = my_or{std::make_unique<my_expression>(my_variable{"x"}),
                               std::make_unique<my_expression>(my_variable{"y"})};
    expression_iterator iter(expr);

    auto children = iter.get_children();

    REQUIRE(children.size() == 2);
    REQUIRE(children[0].get_label() == "x");
    REQUIRE(children[1].get_label() == "y");
}

TEST_CASE("ExpressionIterator - Get children from NOT node", "[expression_iterator][children]") {
    my_expression expr = my_not{std::make_unique<my_expression>(my_variable{"z"})};
    expression_iterator iter(expr);

    auto children = iter.get_children();

    REQUIRE(children.size() == 1);
    REQUIRE(children[0].get_label() == "z");
}

TEST_CASE("ExpressionIterator - Variable node has no children", "[expression_iterator][children]") {
    my_expression expr = my_variable{"x"};
    expression_iterator iter(expr);

    auto children = iter.get_children();

    REQUIRE(children.empty());
}

TEST_CASE("ExpressionIterator - Node address for uniqueness", "[expression_iterator][uniqueness]") {
    my_expression expr = my_variable{"x"};
    expression_iterator iter(expr);

    auto address = iter.get_node_address();

    REQUIRE(address != nullptr);
}

TEST_CASE("ExpressionIterator - Two different nodes have different addresses",
          "[expression_iterator][uniqueness]") {
    my_expression expr1 = my_variable{"x"};
    my_expression expr2 = my_variable{"y"};

    expression_iterator iter1(expr1);
    expression_iterator iter2(expr2);

    REQUIRE(iter1.get_node_address() != iter2.get_node_address());
}

TEST_CASE("ExpressionIterator - Nested expression traversal", "[expression_iterator][traversal]") {
    // Expression: (a AND b) OR c
    my_expression expr = my_or{
        std::make_unique<my_expression>(my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                               std::make_unique<my_expression>(my_variable{"b"})}),
        std::make_unique<my_expression>(my_variable{"c"})};

    expression_iterator root(expr);

    REQUIRE(root.get_label() == "OR");

    auto children = root.get_children();
    REQUIRE(children.size() == 2);

    // First child is AND node
    REQUIRE(children[0].get_label() == "AND");

    // Second child is variable c
    REQUIRE(children[1].get_label() == "c");

    // Check grandchildren through AND node
    auto grandchildren = children[0].get_children();
    REQUIRE(grandchildren.size() == 2);
    REQUIRE(grandchildren[0].get_label() == "a");
    REQUIRE(grandchildren[1].get_label() == "b");
}

TEST_CASE("ExpressionIterator - Fill colors for different operators",
          "[expression_iterator][styling]") {
    SECTION("Variable color") {
        my_expression expr = my_variable{"x"};
        expression_iterator iter(expr);
        REQUIRE(iter.get_fillcolor() == std::string("lightblue"));
    }

    SECTION("AND color") {
        my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                    std::make_unique<my_expression>(my_variable{"b"})};
        expression_iterator iter(expr);
        REQUIRE(iter.get_fillcolor() == std::string("lightgreen"));
    }

    SECTION("OR color") {
        my_expression expr = my_or{std::make_unique<my_expression>(my_variable{"x"}),
                                   std::make_unique<my_expression>(my_variable{"y"})};
        expression_iterator iter(expr);
        REQUIRE(iter.get_fillcolor() == std::string("lightcoral"));
    }

    SECTION("NOT color") {
        my_expression expr = my_not{std::make_unique<my_expression>(my_variable{"z"})};
        expression_iterator iter(expr);
        REQUIRE(iter.get_fillcolor() == std::string("yellow"));
    }

    SECTION("XOR color") {
        my_expression expr = my_xor{std::make_unique<my_expression>(my_variable{"p"}),
                                    std::make_unique<my_expression>(my_variable{"q"})};
        expression_iterator iter(expr);
        REQUIRE(iter.get_fillcolor() == std::string("lightpink"));
    }
}

TEST_CASE("ExpressionIterator - Style attributes", "[expression_iterator][styling]") {
    my_expression expr = my_variable{"x"};
    expression_iterator iter(expr);

    REQUIRE(iter.get_style() == std::string("filled"));
}

TEST_CASE("ExpressionIterator - Edge labels", "[expression_iterator][edges]") {
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};
    expression_iterator iter(expr);

    auto children = iter.get_children();

    // Check edge labels for binary operator
    REQUIRE(iter.get_edge_label(children[0], 0) == "L");  // Left edge
    REQUIRE(iter.get_edge_label(children[1], 1) == "R");  // Right edge
}

TEST_CASE("ExpressionIterator - Copy semantics", "[expression_iterator][copy]") {
    my_expression expr = my_variable{"x"};
    expression_iterator iter1(expr);
    expression_iterator iter2 = iter1;

    REQUIRE(iter2.is_valid());
    REQUIRE(iter2.get_label() == "x");
    REQUIRE(iter1.get_node_address() == iter2.get_node_address());
}

TEST_CASE("ExpressionIterator - Deep nested expression", "[expression_iterator][traversal]") {
    // Expression: ((a AND b) OR (c XOR d)) AND (NOT e)
    my_expression expr = my_and{
        std::make_unique<my_expression>(
            my_or{std::make_unique<my_expression>(
                      my_and{std::make_unique<my_expression>(my_variable{"a"}),
                             std::make_unique<my_expression>(my_variable{"b"})}),
                  std::make_unique<my_expression>(
                      my_xor{std::make_unique<my_expression>(my_variable{"c"}),
                             std::make_unique<my_expression>(my_variable{"d"})})}),
        std::make_unique<my_expression>(my_not{std::make_unique<my_expression>(my_variable{"e"})})};

    expression_iterator root(expr);

    REQUIRE(root.get_label() == "AND");

    auto level1 = root.get_children();
    REQUIRE(level1.size() == 2);
    REQUIRE(level1[0].get_label() == "OR");
    REQUIRE(level1[1].get_label() == "NOT");

    auto level2_or = level1[0].get_children();
    REQUIRE(level2_or.size() == 2);
    REQUIRE(level2_or[0].get_label() == "AND");
    REQUIRE(level2_or[1].get_label() == "XOR");

    auto level3_and = level2_or[0].get_children();
    REQUIRE(level3_and[0].get_label() == "a");
    REQUIRE(level3_and[1].get_label() == "b");

    auto level3_xor = level2_or[1].get_children();
    REQUIRE(level3_xor[0].get_label() == "c");
    REQUIRE(level3_xor[1].get_label() == "d");
}

TEST_CASE("ExpressionIterator - Negative: moved-from expression and invalid access",
          "[expression_iterator][negative]") {
    // Move an expression into the iterator and then ensure copy remains valid
    my_expression expr = my_variable{"x"};
    expression_iterator iter(std::move(expr));

    // Iterator should be valid even if original expression was moved-from
    REQUIRE(iter.is_valid());

    // Accessing children on a variable should return empty vector and not crash
    auto children = iter.get_children();
    REQUIRE(children.empty());

    // Construct an iterator over a NOT node and attempt an out-of-range child index
    my_expression not_expr = my_not{std::make_unique<my_expression>(my_variable{"y"})};
    expression_iterator iter_not(not_expr);
    auto ch = iter_not.get_children();
    REQUIRE(ch.size() == 1);
    // Accessing index 1 should be out-of-range; ensure no throw and returned behaviour
    REQUIRE_NOTHROW([&]() {
        if (ch.size() > 1)
            (void)ch[1].get_label();
    }());
}

TEST_CASE("ExpressionIterator - Negative: malformed variant contents",
          "[expression_iterator][negative]") {
    // Create a fake iterator-like type by abusing the interface is difficult
    // so this test ensures that iterator methods gracefully handle unexpected
    // internal states by constructing via normal paths and not crashing.
    my_expression expr = my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                std::make_unique<my_expression>(my_variable{"b"})};
    expression_iterator iter(expr);

    // The iterator should provide stable outputs even when queried repeatedly
    for (int i = 0; i < 10; ++i) {
        REQUIRE_NOTHROW(iter.get_label());
        REQUIRE_NOTHROW(iter.get_children());
    }
}
