// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

#include <catch2/catch_test_macros.hpp>
#include <sstream>

#include "dot_graph_generator.hpp"
#include "expression_iterator.hpp"
#include "expression_types.hpp"

TEST_CASE("DOT Graph generation basic output", "[dot_graph][generation]") {
    // Build a simple expression: (a AND b) OR c
    my_expression expr = my_or{
        std::make_unique<my_expression>(my_and{std::make_unique<my_expression>(my_variable{"a"}),
                                               std::make_unique<my_expression>(my_variable{"b"})}),
        std::make_unique<my_expression>(my_variable{"c"})};

    expression_iterator root_iter(expr);

    std::ostringstream out;
    dot_graph::DotConfig cfg;
    cfg.graph_name = "TestGraph";
    cfg.default_node_shape = "ellipse";

    dot_graph::generate_dot_graph(root_iter, out, cfg);

    std::string dot = out.str();

    REQUIRE(dot.find("digraph TestGraph") != std::string::npos);
    REQUIRE(dot.find("label = \"a\"") != std::string::npos);
    REQUIRE(dot.find("label = \"b\"") != std::string::npos);
    REQUIRE(dot.find("label = \"c\"") != std::string::npos);

    // Check for edges (OR -> AND, OR -> c, AND -> a, AND -> b)
    REQUIRE(dot.find("->") != std::string::npos);
}
