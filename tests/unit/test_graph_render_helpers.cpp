#// SPDX-License-Identifier: MIT
#// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file tests/unit/test_graph_render_helpers.cpp
 * @brief Tests for graph rendering helper functions
 *
 * Validates label escaping, node/edge rendering helpers, property joining and
 * class assignment flattening used across DOT and Mermaid generators.
 */

#include <catch2/catch_test_macros.hpp>
// Product files under test:
// - include/graph_render_helpers.hpp
#include <unordered_map>
#include <vector>

#include "graph_render_helpers.hpp"

TEST_CASE("graph_render_helpers - escape_label escapes quotes", "[graph_render_helpers]") {
    std::string in = "He said \"hello\"";
    auto out = graph_render_helpers::escape_label(in);
    REQUIRE(out.find("\\\"") != std::string::npos);
}

TEST_CASE("graph_render_helpers - render_node_line shapes", "[graph_render_helpers]") {
    auto box = graph_render_helpers::render_node_line("N1", "Label", "square");
    REQUIRE(box.find("N1[") != std::string::npos);

    auto circle = graph_render_helpers::render_node_line("N2", "C", "circle");
    REQUIRE(circle.find("N2((") != std::string::npos);

    auto diamond = graph_render_helpers::render_node_line("N3", "D", "diamond");
    REQUIRE(diamond.find("N3{\"") != std::string::npos);
}

TEST_CASE("graph_render_helpers - render_edge_line styles and labels", "[graph_render_helpers]") {
    auto solid = graph_render_helpers::render_edge_line("A", "B", "", "");
    REQUIRE(solid.find("A --> B") != std::string::npos);

    auto dashed = graph_render_helpers::render_edge_line("A", "B", "dashed", "lbl");
    REQUIRE(dashed.find("A -.->") != std::string::npos);
    REQUIRE(dashed.find("|\"lbl\"|") != std::string::npos);
}

TEST_CASE("graph_render_helpers - flatten_and_sort_class_assignments and join_dot_properties",
          "[graph_render_helpers]") {
    std::unordered_map<std::string, std::vector<std::string>> map;
    map["classA"] = {"N2", "N10"};
    map["classB"] = {"N1"};

    auto pairs = graph_render_helpers::flatten_and_sort_class_assignments(map);
    REQUIRE(!pairs.empty());
    // Ensure sort order uses node_id_less (N1, N2, N10 expected order)
    REQUIRE(pairs[0].first == "N1");

    std::vector<std::string> props = {"label = \"X\"", "shape=box"};
    auto joined = graph_render_helpers::join_dot_properties(props);
    REQUIRE(joined.front() == '[');
    REQUIRE(joined.find("label = \"X\"") != std::string::npos);
}
