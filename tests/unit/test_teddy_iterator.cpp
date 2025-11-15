#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

#include "teddy_convert.hpp"
#include "teddy_iterator.hpp"

using namespace teddy::ops;

// This test verifies basic teddy_iterator behavior using a tiny BDD built
// directly with the TeDDy manager APIs. It mirrors the style used in
// other tests by constructing a manager, creating variables/constants and
// checking iterator methods like get_label, is_terminal, get_children, and
// edge styles.

TEST_CASE("teddy_iterator basic operations") {
    // Create a small manager with 2 variables
    teddy::bdd_manager mgr(2, 1000);

    // Build a simple BDD: (a AND b)
    auto var0 = mgr.variable(0);
    auto var1 = mgr.variable(1);
    auto cube = mgr.constant(1);
    auto and_bdd = mgr.apply<AND>(mgr.apply<AND>(var0, var1), cube);

    // variable names
    std::vector<std::string> names{"a", "b"};

    // Obtain root node pointer for iterator
    auto root = and_bdd.unsafe_get_root();
    REQUIRE(root != nullptr);

    teddy_iterator it(root, &names);

    SECTION("iterator valid and labels") {
        REQUIRE(it.is_valid());
        REQUIRE(!it.is_terminal());
        auto lbl = it.get_label();
        REQUIRE((lbl == std::string("a") || (lbl.rfind("x", 0) == 0)));
        REQUIRE(it.get_variable_name() == "a");
        REQUIRE(it.get_type() == "Variable");
        REQUIRE(it.get_tooltip() == std::to_string(root->get_index()));
        REQUIRE(it.get_css_class() == "bddVariable");
    }

    SECTION("children and edge styles") {
        auto children = it.get_children();
        REQUIRE(children.size() == 2);
        // false child is index 0, style dashed
        REQUIRE(it.get_edge_style(children[0], 0) == "dashed");
        REQUIRE((children[0].get_shape() == std::string("square") || children[0].is_terminal()));
        // true child is index 1, style solid
        REQUIRE(it.get_edge_style(children[1], 1) == "solid");
        REQUIRE((children[1].get_shape() == std::string("circle") || children[1].is_terminal()));
    }

    SECTION("terminal iterator properties") {
        // Walk to a terminal child and test terminal-specific methods
        auto children = it.get_children();
        // At least one child should be terminal
        bool found_terminal = false;
        for (auto& c : children) {
            if (c.is_terminal()) {
                found_terminal = true;
                REQUIRE(c.get_type().rfind("Terminal", 0) == 0);
                REQUIRE(c.get_css_class() == "terminal");
                REQUIRE((c.get_terminal_value() == 0 || c.get_terminal_value() == 1));
            }
        }
        REQUIRE(found_terminal);
    }
}
