#pragma once

#include <cudd/cudd.h>

#include <cudd/cuddObj.hh>
#include <format>
#include <stdexcept>
#include <string>
#include <vector>

// ============================================================================
// CUDD BDD Iterator Class (Implementation Detail)
// ============================================================================

/**
 * @brief Iterator for traversing CUDD BDD structures with generic template properties
 *
 * This iterator provides the interface needed for both DOT and Mermaid generation
 * using the generic template systems. It wraps CUDD library BDD nodes and provides
 * all the required interface methods for both graph generators.
 *
 * Note: CUDD represents BDD nodes with possible complement bits in pointers. This means
 * that the same memory can represent different logical nodes depending on the complement bit.
 * Handle this by swapping the children accordingly when traversing.
 */
class cudd_iterator {
   private:
    const Cudd* cudd_manager_;  ///< CUDD manager pointer
    DdNode* node_;              ///< Current BDD node (may have complement bit)
    const std::vector<std::string>* variable_names_;  ///< Variable names for display

   public:
    /**
     * @brief Construct iterator for a CUDD BDD node
     * @param cudd_mgr CUDD manager wrapper
     * @param node BDD node (handles pointer complementing)
     * @param variable_names Variable names for display
     */
    cudd_iterator(const Cudd& cudd_mgr, DdNode* node,
                  const std::vector<std::string>* variable_names)
        : cudd_manager_(&cudd_mgr), node_(node), variable_names_(variable_names) {}

    // ========================================================================
    // BaseGraphIterator Interface Requirements
    // ========================================================================

    /**
     * @brief Get child iterators for this node
     * @return Vector of child iterators (THEN, ELSE for BDD nodes)
     */
    std::vector<cudd_iterator> get_children() const {
        std::vector<cudd_iterator> children;

        // Only non-constant nodes have children
        if (!Cudd_IsConstant(node_)) {
            // Get ELSE child (low edge) - FALSE child (first, to match TeDDy convention)
            DdNode* else_node = Cudd_E(node_);
            children.emplace_back(*cudd_manager_, else_node, variable_names_);

            // Get THEN child (high edge) - TRUE child (second, to match TeDDy convention)
            DdNode* then_node = Cudd_T(node_);
            children.emplace_back(*cudd_manager_, then_node, variable_names_);

            // Note: Do NOT swap children for complemented nodes - this breaks the BDD structure
            // The complement bit is already handled properly in get_terminal_value() and
            // get_node_address()
        }

        return children;
    }

    /**
     * @brief Get unique address for this node (for equality comparisons)
     * @return Pointer value as const void* (with complement bit preserved)
     */
    const void* get_node_address() const {
        return reinterpret_cast<const void*>(node_);
    }

    /**
     * @brief Equality operator for iterators
     */
    bool operator==(const cudd_iterator& other) const {
        return node_ == other.node_;
    }

    /**
     * @brief Inequality operator for iterators
     */
    bool operator!=(const cudd_iterator& other) const {
        return node_ != other.node_;
    }

    // ========================================================================
    // DOT Graph Generator Interface
    // ========================================================================

    /**
     * @brief Get DOT node identifier
     */
    std::string get_dot_node_id() const {
        return std::format("node_{:x}", reinterpret_cast<uintptr_t>(node_));
    }

    /**
     * @brief Get DOT node label
     */
    std::string get_label() const {
        if (Cudd_IsConstant(node_)) {
            return get_terminal_value() ? "1" : "0";
        } else {
            // Internal node - show variable name
            unsigned int var_index = Cudd_NodeReadIndex(node_);
            if (variable_names_ && var_index < variable_names_->size()) {
                return (*variable_names_)[var_index];
            } else {
                return std::format("x{}", var_index);
            }
        }
    }

    /**
     * @brief Get DOT node attributes
     */
    std::string get_dot_node_attributes() const {
        DdNode* regular_node = node_;

        if (Cudd_IsConstant(regular_node)) {
            // Terminal nodes are squares
            bool final_value = get_terminal_value() == 1;
            std::string color = final_value ? "lightgreen" : "lightcoral";
            return std::format("shape=box,style=filled,fillcolor={}", color);
        } else {
            // Internal nodes are circles
            return "shape=circle";
        }
    }

    std::string get_edge_style(const cudd_iterator& child, size_t child_index) const {
        if (child_index == 0) {
            return "dashed";  // False edge
        } else {
            return "solid";  // True edge
        }
    }

    std::string get_shape() const {
        if (!node_)
            return "circle";

        return is_terminal() ? "square" : "circle";
    }

    std::string get_tooltip() const {
        if (!node_)
            return "";

        if (is_terminal()) {
            return std::to_string(get_terminal_value());
        } else {
            return std::to_string(Cudd_NodeReadIndex(node_));
        }
    }

    /**
     * @brief Get DOT edge attributes to a child
     */
    std::string get_dot_edge_attributes(const cudd_iterator& child, size_t child_index) const {
        if (child_index == 0) {
            // ELSE edge (low)
            return "label=\"0\",style=dashed";
        } else {
            // THEN edge (high)
            return "label=\"1\",style=solid";
        }
    }

    // ========================================================================
    // Mermaid Graph Generator Interface
    // ========================================================================

    /**
     * @brief Get Mermaid node identifier
     */
    std::string get_mermaid_node_id() const {
        return std::format("N{:x}", reinterpret_cast<uintptr_t>(node_));
    }

    /**
     * @brief Get Mermaid node declaration
     */
    std::string get_mermaid_node_declaration() const {
        std::string node_id = get_mermaid_node_id();
        std::string label = get_label();  // Reuse DOT label

        DdNode* regular_node = node_;
        if (Cudd_IsConstant(regular_node)) {
            // Terminal nodes as rectangles
            return std::format("{}[{}]", node_id, label);
        } else {
            // Internal nodes as circles
            return std::format("{}({})", node_id, label);
        }
    }

    /**
     * @brief Get Mermaid edge declaration to a child
     */
    std::string get_mermaid_edge_declaration(const cudd_iterator& child, size_t child_index) const {
        std::string edge_label = (child_index == 0) ? "0" : "1";
        return std::format("{} -->|{}| {}", get_mermaid_node_id(), edge_label,
                           child.get_mermaid_node_id());
    }

    // ========================================================================
    // Node Table Generator Interface
    // ========================================================================

    /**
     * @brief Get variable name for node table
     */
    std::string get_variable_name() const {
        DdNode* regular_node = node_;

        if (Cudd_IsConstant(regular_node)) {
            return "-";
        } else {
            unsigned int var_index = Cudd_NodeReadIndex(regular_node);
            if (variable_names_ && var_index < variable_names_->size()) {
                return (*variable_names_)[var_index];
            } else {
                return std::format("x{}", var_index);
            }
        }
    }

    /**
     * @brief Get low child identifier for node table
     */
    std::string get_low_child_id() const {
        if (Cudd_IsConstant(node_)) {
            return "-";
        }

        auto children = get_children();
        return children.size() > 1 ? children[1].get_dot_node_id() : "-";
    }

    /**
     * @brief Get high child identifier for node table
     */
    std::string get_high_child_id() const {
        if (Cudd_IsConstant(node_)) {
            return "-";
        }

        auto children = get_children();
        return children.size() > 0 ? children[0].get_dot_node_id() : "-";
    }

    /**
     * @brief Check if this is a terminal node
     */
    bool is_terminal() const {
        return Cudd_IsConstant(node_);
    }

    /**
     * @brief Get terminal value (only valid for terminal nodes)
     */
    int get_terminal_value() const {
        if (!is_terminal()) {
            throw std::runtime_error("get_terminal_value called on non-terminal node");
        }

        DdNode* regular_node = node_;
        int value = Cudd_V(regular_node);
        if (Cudd_IsComplement(node_)) {
            value = !value;
        }
        return value;
    }

    /**
     * @brief Get the underlying DdNode pointer
     */
    DdNode* get_node() const {
        return node_;
    }

    /**
     * @brief Check if this iterator is valid
     */
    bool is_valid() const {
        return node_ != nullptr;
    }

    /**
     * @brief Get node type description
     */
    std::string get_type() const {
        if (is_terminal()) {
            return "Terminal(" + std::to_string(get_terminal_value()) + ")";
        } else {
            return "Variable";
        }
    }
};
