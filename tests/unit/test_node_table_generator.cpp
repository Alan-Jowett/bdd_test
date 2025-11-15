// SPDX-License-Identifier: MIT

/**
 * @file tests/unit/test_node_table_generator.cpp
 * @brief Unit tests for node table generation (text/markdown/csv)
 *
 * Verifies that the node table generators produce correctly formatted output
 * including headers, counts, and terminal/internal node representations.
 */

#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <string>
#include <vector>

#include "node_table_generator.hpp"

// Simple fake iterator satisfying BaseGraphIterator and NodeTableIterator
struct FakeIterator {
    std::string var;
    std::string type;
    int term_value = 0;
    std::vector<FakeIterator> children;

    // BaseGraphIterator requirements
    const std::vector<FakeIterator>& get_children() const {
        return children;
    }
    const void* get_node_address() const {
        return static_cast<const void*>(this);
    }

    bool operator==(const FakeIterator& other) const {
        return this == &other;
    }
    bool operator!=(const FakeIterator& other) const {
        return !(*this == other);
    }

    // NodeTableIterator requirements
    bool is_valid() const {
        return true;
    }
    bool is_terminal() const {
        return children.empty();
    }
    const std::string& get_variable_name() const {
        return var;
    }
    const std::string& get_type() const {
        return type;
    }
    int get_terminal_value() const {
        return term_value;
    }
};

TEST_CASE("node_table: text/markdown/csv generation", "[node_table]") {
    // Build a small graph: root -> (false: leaf0, true: leaf1)
    FakeIterator leaf0;
    leaf0.var = "leaf0";
    leaf0.type = "terminal";
    leaf0.term_value = 0;
    FakeIterator leaf1;
    leaf1.var = "leaf1";
    leaf1.type = "terminal";
    leaf1.term_value = 1;

    FakeIterator root;
    root.var = "x";
    root.type = "internal";
    root.children = {leaf0, leaf1};

    std::ostringstream out;

    SECTION("text table includes headers and counts") {
        node_table::generate_text_table(root, out, node_table::TextTableConfig(true, "TestTitle"));
        std::string s = out.str();
        REQUIRE(s.find("TestTitle:") != std::string::npos);
        REQUIRE(s.find("Index") != std::string::npos);
        REQUIRE(s.find("Total nodes") != std::string::npos);
    }

    out.str("");
    out.clear();
    SECTION("markdown table format") {
        node_table::generate_markdown_table(root, out, node_table::MarkdownTableConfig("left"));
        std::string s = out.str();
        REQUIRE(s.find("| Index |") != std::string::npos);
        // Terminals are rendered as "- | - | - | <type> |" in markdown
        REQUIRE(s.find("- | - | - | terminal |") != std::string::npos);
        REQUIRE(s.find("internal") != std::string::npos);
    }

    out.str("");
    out.clear();
    SECTION("csv output") {
        node_table::generate_csv_table(root, out, true);
        std::string s = out.str();
        REQUIRE(s.find("Index,Variable,False Child,True Child,Type") != std::string::npos);
        REQUIRE(s.find("leaf1") != std::string::npos);
    }
}

TEST_CASE("node_table - negative: empty node and malformed children", "[node_table][negative]") {
    // Empty node: no variable name and no children (terminal-like)
    FakeIterator empty_node;
    empty_node.var = "";
    empty_node.type = "";
    empty_node.term_value = 0;

    std::ostringstream out;

    // Generators should not throw on empty or malformed input and should produce some output
    REQUIRE_NOTHROW(node_table::generate_text_table(
        empty_node, out, node_table::TextTableConfig(false, "EmptyTest")));
    REQUIRE(out.str().size() > 0);

    out.str("");
    out.clear();
    REQUIRE_NOTHROW(node_table::generate_markdown_table(empty_node, out,
                                                        node_table::MarkdownTableConfig("left")));
    REQUIRE(out.str().size() > 0);

    out.str("");
    out.clear();
    REQUIRE_NOTHROW(node_table::generate_csv_table(empty_node, out, false));
    REQUIRE(out.str().size() > 0);

    // Malformed: child vector contains self-reference (duplicate address)
    FakeIterator self_node;
    self_node.var = "self";
    self_node.type = "internal";
    self_node.children = {self_node};  // note: creates a copy with same values; addresses will
                                       // differ in this simple struct

    out.str("");
    out.clear();
    // Ensure generation still succeeds; real cycle detection isn't part of node_table generation
    REQUIRE_NOTHROW(node_table::generate_text_table(self_node, out,
                                                    node_table::TextTableConfig(true, "SelfTest")));
    REQUIRE(out.str().size() > 0);
}
