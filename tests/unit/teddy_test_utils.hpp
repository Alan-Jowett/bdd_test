// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

#pragma once

#include <vector>

#include "libteddy/core.hpp"

// Shared test helper for evaluating a TeDDy BDD with a boolean assignment
inline bool evaluate_teddy_bdd(teddy::bdd_manager& manager, teddy::bdd_manager::diagram_t bdd,
                               const std::vector<bool>& assignment) {
    using namespace teddy::ops;

    teddy::bdd_manager::diagram_t cube = manager.constant(1);
    for (size_t i = 0; i < assignment.size(); ++i) {
        teddy::bdd_manager::diagram_t var = manager.variable(static_cast<int>(i));
        if (!assignment[i]) {
            teddy::bdd_manager::diagram_t one = manager.constant(1);
            var = manager.apply<XOR>(var, one);
        }
        cube = manager.apply<AND>(cube, var);
    }

    teddy::bdd_manager::diagram_t result = manager.apply<AND>(bdd, cube);
    return result.unsafe_get_root() == cube.unsafe_get_root();
}
