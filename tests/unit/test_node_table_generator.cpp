#// SPDX-License-Identifier: MIT

/**
 * @file tests/unit/test_node_table_generator.cpp
 * @brief Unit tests for node table generation (text/markdown/csv)
 *
 * Verifies that the node table generators produce correctly formatted output
 * including headers, counts, and terminal/internal node representations.
 */

// Product files under test:
// - include/node_table_generator.hpp
// - include/graph_iterator_concepts.hpp

#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <string>

#include "node_table_generator.hpp"

// Simple fake iterator satisfying BaseGraphIterator and NodeTableIterator
struct FakeIterator {
    std::string var;
    std::string type;
    int term_value = 0;
    std::vector<FakeIterator> children;

    // BaseGraphIterator requirements
    std::vector<FakeIterator> get_children() const {
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
    std::string get_variable_name() const {
        return var;
    }
    std::string get_type() const {
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
