// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

#pragma once

#include <climits>
#include <stdexcept>
#include <vector>

#include "libteddy/core.hpp"

// Shared test helper for evaluating a TeDDy BDD with a boolean assignment
inline bool evaluate_teddy_bdd(teddy::bdd_manager& manager, teddy::bdd_manager::diagram_t bdd,
                               const std::vector<bool>& assignment) {
    using namespace teddy::ops;

    // Guard against pathological assignment sizes overflowing `int`.
    if (assignment.size() > static_cast<size_t>(INT_MAX)) {
        throw std::out_of_range("Assignment size exceeds int range in test helper");
    }

    // NOTE: This helper assumes the provided `assignment` length does not exceed
    // the number of variables configured in `manager`. Calling `manager.variable(i)`
    // with i >= manager's variable count may lead to undefined behavior in TeDDy.
    // Tests that need to exercise mismatched-length behavior should handle the
    // guarded behavior explicitly (see tests/unit/test_teddy_graph.cpp).

    teddy::bdd_manager::diagram_t cube = manager.constant(1);
    for (size_t i = 0; i < assignment.size(); ++i) {
        int idx = static_cast<int>(i);
        teddy::bdd_manager::diagram_t var = manager.variable(idx);
        if (!assignment[i]) {
            teddy::bdd_manager::diagram_t one = manager.constant(1);
            var = manager.apply<XOR>(var, one);
        }
        cube = manager.apply<AND>(cube, var);
    }

    teddy::bdd_manager::diagram_t result = manager.apply<AND>(bdd, cube);
    return result.unsafe_get_root() == cube.unsafe_get_root();
}
