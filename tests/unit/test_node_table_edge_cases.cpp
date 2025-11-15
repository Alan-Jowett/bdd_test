// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <sstream>

#include "node_table_generator.hpp"

// Fake iterator similar to existing tests but with single child and missing mapping
struct SingleChildIterator {
    std::string var;
    std::string type;
    std::vector<SingleChildIterator> children;

    const std::vector<SingleChildIterator>& get_children() const {
        return children;
    }
    const void* get_node_address() const {
        return static_cast<const void*>(this);
    }
    bool operator==(const SingleChildIterator& o) const {
        return this == &o;
    }
    bool operator!=(const SingleChildIterator& o) const {
        return !(*this == o);
    }

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
        return 0;
    }
};

TEST_CASE("node_table - single child handling and unsupported format", "[node_table][edge_cases]") {
    SingleChildIterator leaf;
    leaf.var = "leaf";
    leaf.type = "terminal";

    SingleChildIterator root;
    root.var = "x";
    root.type = "internal";
    root.children = {leaf};  // only one child present

    std::ostringstream out;

    // Should not throw even if only one child (false/true child mapping incomplete)
    REQUIRE_NOTHROW(node_table::generate_text_table(root, out));
    REQUIRE(out.str().size() > 0);

    out.str("");
    out.clear();
    REQUIRE_NOTHROW(node_table::generate_markdown_table(root, out));
    REQUIRE(out.str().size() > 0);

    out.str("");
    out.clear();
    REQUIRE_NOTHROW(node_table::generate_csv_table(root, out));
    REQUIRE(out.str().size() > 0);

    // Unsupported format should throw
    out.str("");
    out.clear();
    REQUIRE_THROWS_AS(node_table::generate_table(root, out, "unknown_format"),
                      std::invalid_argument);
}
