// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file tests/unit/test_cudd_iterator.cpp
 * @brief Unit tests for the CUDD BDD iterator wrapper
 *
 * Verifies iterator semantics, labels, terminal detection and DOT attribute
 * generation for the CUDD-based iterator interface.
 */

#include <cudd/cudd.h>

#include <catch2/catch_test_macros.hpp>
#include <cudd/cuddObj.hh>
#include <string>
#include <vector>

#include "cudd_iterator.hpp"

TEST_CASE("cudd_iterator basic operations") {
    // Create a CUDD manager and some simple BDDs
    Cudd cudd_mgr;

    std::vector<std::string> varnames = {"v0", "v1"};

    BDD var0 = cudd_mgr.bddVar(0);
    BDD var1 = cudd_mgr.bddVar(1);
    BDD one = cudd_mgr.bddOne();
    BDD zero = cudd_mgr.bddZero();

    // Non-terminal variable node
    cudd_iterator it_var0(cudd_mgr, var0.getNode(), &varnames);
    REQUIRE(it_var0.is_valid());
    REQUIRE(!it_var0.is_terminal());
    REQUIRE(it_var0.get_label() == "v0");
    REQUIRE(it_var0.get_variable_name() == "v0");
    REQUIRE(it_var0.get_type() == "Variable");

    auto children = it_var0.get_children();
    REQUIRE(children.size() == 2);
    REQUIRE(children[0].is_terminal());
    REQUIRE(children[0].get_terminal_value() == 0);
    REQUIRE(children[1].is_terminal());
    REQUIRE(children[1].get_terminal_value() == 1);

    // Complemented access to the same node should flip child terminal values
    DdNode* not_var0_ptr = Cudd_Not(var0.getNode());
    cudd_iterator it_not_var0(cudd_mgr, not_var0_ptr, &varnames);
    auto children_not = it_not_var0.get_children();
    REQUIRE(children_not.size() == 2);
    REQUIRE(children_not[0].is_terminal());
    REQUIRE(children_not[0].get_terminal_value() == 1);
    REQUIRE(children_not[1].is_terminal());
    REQUIRE(children_not[1].get_terminal_value() == 0);

    // Node addresses should preserve complement bit and therefore differ
    REQUIRE(it_var0.get_node_address() != it_not_var0.get_node_address());
    REQUIRE(it_var0 != it_not_var0);
    REQUIRE(it_var0 == it_var0);

    // Terminal node attributes
    cudd_iterator it_one(cudd_mgr, one.getNode(), nullptr);
    REQUIRE(it_one.is_terminal());
    REQUIRE(it_one.get_label() == "1");
    auto attrs = it_one.get_dot_node_attributes();
    REQUIRE(attrs.find("fillcolor=lightgreen") != std::string::npos);

    // Edge attributes
    REQUIRE(it_var0.get_dot_edge_attributes(children[0], 0).find("label=\"0\"")
            != std::string::npos);
    REQUIRE(it_var0.get_dot_edge_attributes(children[1], 1).find("label=\"1\"")
            != std::string::npos);

    // Tooltip and CSS class
    REQUIRE(it_one.get_tooltip() == "1");
    REQUIRE(it_one.get_css_class() == "terminal");
    REQUIRE(it_var0.get_css_class() == "bddVariable");
}
