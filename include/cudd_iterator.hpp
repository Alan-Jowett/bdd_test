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
     * 
     * Properly handles CUDD's complement edges following the standard CUDD pattern:
     * - Get children from the regular (non-complemented) node
     * - If this node is accessed through a complement edge, negate the children
     */
    std::vector<cudd_iterator> get_children() const {
        std::vector<cudd_iterator> children;

        // Get the regular (physical) node
        DdNode* regular_node = Cudd_Regular(node_);

        // Only non-constant nodes have children
        if (!Cudd_IsConstant(regular_node)) {
            // Get children from the regular node
            // Cudd_E and Cudd_T return the actual child pointers, which may themselves be complemented
            DdNode* else_child = Cudd_E(regular_node);
            DdNode* then_child = Cudd_T(regular_node);

            // If we're accessing this node through a complement edge, negate the children
            // This implements the semantics: NOT(node) = node with negated children
            if (Cudd_IsComplement(node_)) {
                else_child = Cudd_Not(else_child);
                then_child = Cudd_Not(then_child);
            }

            // Return children in TeDDy convention order: FALSE (else) first, TRUE (then) second
            children.emplace_back(*cudd_manager_, else_child, variable_names_);
            children.emplace_back(*cudd_manager_, then_child, variable_names_);
        }

        return children;
    }

    /**
     * @brief Get unique address for this node (for equality comparisons)
     * @return Pointer value as const void* (complement bit removed for physical node uniqueness)
     * 
     * Returns the regular (non-complemented) node pointer to ensure that a node accessed through
     * a complement edge is recognized as the same physical node. This is necessary for proper
     * ROBDD comparison with TeDDy, which doesn't use complement edges.
     */
    const void* get_node_address() const {
        return reinterpret_cast<const void*>(Cudd_Regular(node_));
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
     * @brief Get DOT node label
     */
    std::string get_label() const {
        DdNode* regular_node = Cudd_Regular(node_);
        
        if (Cudd_IsConstant(regular_node)) {
            // Terminal node - return the logical value (accounting for complement edge)
            return get_terminal_value() ? "1" : "0";
        } else {
            // Internal node - show variable name (based on physical node, not complement)
            unsigned int var_index = Cudd_NodeReadIndex(regular_node);
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
     * @brief Check if this is a terminal node
     */
    bool is_terminal() const {
        return Cudd_IsConstant(Cudd_Regular(node_));
    }

    /**
     * @brief Get terminal value (only valid for terminal nodes)
     * 
     * Returns the logical terminal value, accounting for complement edges.
     */
    int get_terminal_value() const {
        if (!is_terminal()) {
            throw std::runtime_error("get_terminal_value called on non-terminal node");
        }

        DdNode* regular_node = Cudd_Regular(node_);
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
