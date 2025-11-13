#pragma once

#include <format>
#include <libteddy/core.hpp>
#include <string>
#include <vector>

// ============================================================================
// TeDDy BDD Iterator Class (Implementation Detail)
// ============================================================================

/**
 * @brief Iterator for traversing TeDDy BDD structures with generic template properties
 *
 * This iterator provides the interface needed for both DOT and Mermaid generation
 * using the generic template systems. It wraps TeDDy library BDD nodes and provides
 * all the required interface methods for both graph generators.
 */
class teddy_iterator {
   public:
    using node_t = teddy::bdd_manager::diagram_t::node_t;

   private:
    node_t* current_node_;                            ///< Current BDD node
    const std::vector<std::string>* variable_names_;  ///< Variable names for labels

   public:
    // Constructors and assignment operators
    explicit teddy_iterator(node_t* node = nullptr,
                            const std::vector<std::string>* variable_names = nullptr)
        : current_node_(node), variable_names_(variable_names) {}

    teddy_iterator(const teddy_iterator& other)
        : current_node_(other.current_node_), variable_names_(other.variable_names_) {}

    teddy_iterator(teddy_iterator&& other) noexcept
        : current_node_(other.current_node_), variable_names_(other.variable_names_) {
        other.current_node_ = nullptr;
    }

    teddy_iterator& operator=(const teddy_iterator& other) {
        if (this != &other) {
            current_node_ = other.current_node_;
            variable_names_ = other.variable_names_;
        }
        return *this;
    }

    teddy_iterator& operator=(teddy_iterator&& other) noexcept {
        if (this != &other) {
            current_node_ = other.current_node_;
            variable_names_ = other.variable_names_;
            other.current_node_ = nullptr;
        }
        return *this;
    }

    // Iterator interface
    bool is_valid() const {
        return current_node_ != nullptr;
    }

    bool operator==(const teddy_iterator& other) const {
        return current_node_ == other.current_node_;
    }

    bool operator!=(const teddy_iterator& other) const {
        return !(*this == other);
    }

    // Generic Template Property Methods (required by both template systems)
    const void* get_node_address() const {
        return static_cast<const void*>(current_node_);
    }

    // Type-safe node access
    node_t* get_node() const {
        return current_node_;
    }

    std::string get_label() const {
        if (!current_node_)
            return "";

        if (current_node_->is_terminal()) {
            return std::to_string(current_node_->get_value());
        } else {
            int var_index = current_node_->get_index();
            if (variable_names_ && var_index < variable_names_->size()) {
                return (*variable_names_)[var_index];
            } else {
                return std::format("x{}", var_index);
            }
        }
    }

    std::string get_shape() const {
        if (!current_node_)
            return "circle";

        return current_node_->is_terminal() ? "square" : "circle";
    }

    std::string get_tooltip() const {
        if (!current_node_)
            return "";

        if (current_node_->is_terminal()) {
            return std::to_string(current_node_->get_value());
        } else {
            return std::to_string(current_node_->get_index());
        }
    }

    std::string get_css_class() const {
        if (!current_node_)
            return "default";

        return current_node_->is_terminal() ? "terminal" : "bddVariable";
    }

    std::vector<teddy_iterator> get_children() const {
        std::vector<teddy_iterator> children;

        if (!current_node_ || current_node_->is_terminal()) {
            return children;
        }

        // Add false child (dashed edge)
        node_t* false_child = current_node_->get_son(0);
        if (false_child) {
            children.emplace_back(false_child, variable_names_);
        }

        // Add true child (solid edge)
        node_t* true_child = current_node_->get_son(1);
        if (true_child) {
            children.emplace_back(true_child, variable_names_);
        }

        return children;
    }

    std::string get_edge_style(const teddy_iterator& child, size_t child_index) const {
        if (child_index == 0) {
            return "dashed";  // False edge
        } else {
            return "solid";  // True edge
        }
    }

    // NodeTableIterator concept methods
    bool is_terminal() const {
        return current_node_ && current_node_->is_terminal();
    }

    std::string get_variable_name() const {
        if (!current_node_ || current_node_->is_terminal()) {
            return "-";
        }

        int var_index = current_node_->get_index();
        if (variable_names_ && var_index < variable_names_->size()) {
            return (*variable_names_)[var_index];
        } else {
            return std::format("x{}", var_index);
        }
    }

    std::string get_type() const {
        if (!current_node_) {
            return "Invalid";
        }

        if (current_node_->is_terminal()) {
            return std::format("Terminal({})", current_node_->get_value());
        } else {
            return "Variable";
        }
    }

    int get_terminal_value() const {
        if (current_node_ && current_node_->is_terminal()) {
            return current_node_->get_value();
        }
        return 0;
    }
};
