// SPDX-License-Identifier: MIT
// Helper functions for node ID parsing and comparison.
#pragma once

#include <string>

namespace graph_common {

inline int node_id_numeric(const std::string &id) {
    if (id.size() <= 1)
        return 0;
    try {
        return std::stoi(id.substr(1));
    } catch (...) {
        return 0;
    }
}

inline bool node_id_less(const std::string &a, const std::string &b) {
    return node_id_numeric(a) < node_id_numeric(b);
}

}  // namespace graph_common
