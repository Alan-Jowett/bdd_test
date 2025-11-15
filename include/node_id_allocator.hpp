// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

#pragma once

#include <format>
#include <string>
#include <unordered_map>

/**
 * @file node_id_allocator.hpp
 * @brief Deterministic allocator for stable textual node IDs used in graph outputs.
 *
 * This small utility provides a deterministic mapping from a node's address
 * (pointer) to a textual identifier used by DOT and Mermaid generators.
 * The allocator supports configurable prefix and start index so different
 * output formats can choose their preferred numbering scheme.
 */
namespace graph_common {

/**
 * @brief Allocates deterministic textual IDs for node pointers.
 *
 * Example: with prefix "N" and start_index 1 the produced IDs are "N1", "N2", ...
 *
 * This class is intentionally small and non-thread-safe. It is expected to be
 * created per-document or per-generation run and not shared across threads.
 */
class node_id_allocator {
   public:
    /**
     * @brief Construct a new node id allocator.
     * @param prefix Textual prefix for generated IDs (default: "N").
     * @param start_index Starting numeric index for generated IDs (default: 0).
     */
    explicit node_id_allocator(const std::string& prefix = "N", int start_index = 0)
        : prefix_(prefix), next_(start_index) {}

    /**
     * @brief Get (or create) the textual ID for the given node pointer.
     * @param key Pointer used to uniquely identify a node.
     * @return std::string Textual identifier (prefix + index).
     */
    std::string get_id(const void* key) {
        auto it = map_.find(key);
        if (it != map_.end()) {
            return it->second;
        }
        std::string id = std::format("{}{}", prefix_, next_++);
        map_[key] = id;
        return id;
    }

    /**
     * @brief Reset the allocator, clearing mappings and index counter.
     *
     * After reset the next generated ID will start again at the original
     * configured start index only if the caller reconstructs the allocator.
     */
    void reset() {
        map_.clear();
        next_ = 0;
    }

   private:
    std::unordered_map<const void*, std::string> map_;  ///< Pointer -> ID map.
    int next_;                                          ///< Next numeric index to use.
    std::string prefix_;                                ///< ID prefix string.
};

}  // namespace graph_common
