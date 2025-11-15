// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file test_dot_graph_bdd_format.cpp
 * @brief Tests DOT generator BDD-specific formatting and optional iterator properties
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <memory>
#include <sstream>

#include "dot_graph_generator.hpp"

using Catch::Matchers::ContainsSubstring;

// Minimal custom iterator type that implements many optional properties
struct BddNode {
    std::string name;
    std::string shape;
    std::string tooltip;
    std::vector<std::shared_ptr<BddNode>> children;
    BddNode(std::string n, std::string s, std::string t)
        : name(std::move(n)), shape(std::move(s)), tooltip(std::move(t)) {}
};

struct bdd_iter {
    std::shared_ptr<BddNode> node;
    bdd_iter() = default;
    explicit bdd_iter(std::shared_ptr<BddNode> n) : node(std::move(n)) {}

    std::vector<bdd_iter> get_children() const {
        std::vector<bdd_iter> out;
        if (!node)
            return out;
        for (const auto& c : node->children)
            out.emplace_back(c);
        return out;
    }

    const void* get_node_address() const {
        return node ? reinterpret_cast<const void*>(node.get()) : nullptr;
    }

    bool operator==(const bdd_iter& other) const {
        return get_node_address() == other.get_node_address();
    }
    bool operator!=(const bdd_iter& other) const {
        return !(*this == other);
    }

    // Optional node properties
    std::string get_label() const {
        return node ? node->name : std::string();
    }
    std::string get_shape() const {
        return node ? node->shape : std::string("circle");
    }
    std::string get_style() const {
        return std::string("filled");
    }
    std::string get_fillcolor() const {
        return std::string("#ffeecc");
    }
    std::string get_fontcolor() const {
        return std::string("#000000");
    }
    std::string get_tooltip() const {
        return node ? node->tooltip : std::string();
    }

    // Optional edge properties
    std::string get_edge_label(const bdd_iter&, size_t idx) const {
        return idx == 0 ? std::string("L") : std::string("R");
    }
    std::string get_edge_style(const bdd_iter&, size_t) const {
        return std::string("dashed");
    }
    std::string get_edge_color(const bdd_iter&, size_t) const {
        return std::string("blue");
    }
    std::string get_edge_fontcolor(const bdd_iter&, size_t) const {
        return std::string("green");
    }
};

TEST_CASE("DotGenerator - BDD format and optional properties", "[dot_graph][bdd_format]") {
    // Build a small DAG: root -> s1, root -> s2, s1 -> s2 (shared child)
    auto s2 = std::make_shared<BddNode>("shared_node", "circle", "shared tip");
    auto s1 = std::make_shared<BddNode>("square_node", "square", "square tip");
    auto root = std::make_shared<BddNode>("root_node", "circle", "root tip");

    s1->children = {s2};
    root->children = {s1, s2};

    bdd_iter root_iter(root);

    std::ostringstream out;
    dot_graph::DotConfig cfg;
    cfg.graph_name = "BDDTest";
    cfg.use_bdd_format = true;     // exercise BDD grouping branch
    cfg.collect_all_edges = true;  // force dag_walker collect_all_edges path

    REQUIRE_NOTHROW(dot_graph::generate_dot_graph(root_iter, out, cfg));

    std::string dot = out.str();

    // Basic structure
    REQUIRE_THAT(dot, ContainsSubstring("digraph BDDTest"));
    REQUIRE_THAT(dot, ContainsSubstring("shared_node"));
    REQUIRE_THAT(dot, ContainsSubstring("square_node"));
    REQUIRE_THAT(dot, ContainsSubstring("root_node"));

    // BDD grouping should emit shape declaration for squares
    REQUIRE_THAT(dot, ContainsSubstring("shape = square"));

    // Tooltip content from iterator should appear
    REQUIRE_THAT(dot, ContainsSubstring("shared tip"));

    // Edge attributes emitted (style/color/label)
    REQUIRE_THAT(dot, ContainsSubstring("dashed"));
    REQUIRE_THAT(dot, ContainsSubstring("blue"));
    REQUIRE_THAT(dot, ContainsSubstring("label = \"L\""));

    // Edges exist
    REQUIRE_THAT(dot, ContainsSubstring("->"));
}
