// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <sstream>

#include "dot_graph_generator.hpp"

// Minimal iterator that only satisfies BaseGraphIterator (no optional properties)
struct minimal_node {
    std::vector<minimal_node> children;
    minimal_node() = default;
    explicit minimal_node(std::initializer_list<minimal_node> ch) : children(ch) {}
    std::vector<minimal_node> get_children() const {
        return children;
    }
    const void* get_node_address() const {
        return reinterpret_cast<const void*>(this);
    }
    bool operator==(const minimal_node& o) const {
        return this == &o;
    }
    bool operator!=(const minimal_node& o) const {
        return !(*this == o);
    }
};

TEST_CASE("DotGenerator - fallback iterator and convenience overload", "[dot_graph][fallback]") {
    minimal_node leaf;
    minimal_node root{leaf, leaf};

    // Use convenience overload that accepts graph name string
    std::ostringstream out;
    REQUIRE_NOTHROW(dot_graph::generate_dot_graph(root, out, std::string("MinimalGraph")));

    auto s = out.str();
    REQUIRE(s.find("digraph MinimalGraph") != std::string::npos);
    // Fallback label uses node id, so expect 'node' or similar prefix
    REQUIRE(s.find("label = \"") != std::string::npos);
}
