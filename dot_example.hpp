/**
 * @file dot_example.hpp  
 * @brief Example implementation of the pure iterator DOT template for expression trees
 * 
 * This file demonstrates how to use the pure iterator DOT template system with
 * expression trees. The iterator represents both the current node and the traversal
 * logic, eliminating the need for separate data types.
 * 
 * @author BDD Expression Converter Project
 * @date November 8, 2025
 */

#pragma once

#include "dot_template.hpp"
#include <memory>
#include <variant>

// Forward declarations for the expression types (assuming they exist in main.cpp)
struct my_and;
struct my_or;
struct my_not;
struct my_xor;
struct my_variable;
using my_expression = std::variant<my_and, my_or, my_not, my_xor, my_variable>;
using my_expression_ptr = std::unique_ptr<my_expression>;

namespace dot_example {

/**
 * @brief Pure iterator for expression trees - represents both node and traversal logic
 * 
 * This iterator wraps an expression and provides the pure iterator interface needed
 * by the DOT template. The iterator IS the current node, eliminating the need for
 * separate data types.
 * 
 * Required interface (duck typing):
 * - std::vector<ExpressionIterator> get_children() const
 * - std::string get_node_id() const
 * 
 * Optional node property methods (auto-detected):
 * - std::string get_label() const
 * - std::string get_shape() const
 * - std::string get_style() const
 * - std::string get_fillcolor() const
 * - std::string get_fontcolor() const
 * - std::string get_tooltip() const
 * 
 * Optional edge property methods (auto-detected):
 * - std::string get_edge_label(const ExpressionIterator& child, size_t index) const
 * - std::string get_edge_style(const ExpressionIterator& child, size_t index) const
 * - std::string get_edge_color(const ExpressionIterator& child, size_t index) const
 * - std::string get_edge_fontcolor(const ExpressionIterator& child, size_t index) const
 * 
 * Optional filtering:
 * - bool should_process() const (optional)
 */
class ExpressionIterator {
private:
    std::shared_ptr<my_expression> expr_ptr;
    mutable int next_node_id = 0;
    
public:
    ExpressionIterator(std::shared_ptr<my_expression> expr) : expr_ptr(expr) {}
    
    /**
     * @brief Get the display label for this expression node
     */
    std::string get_label() const {
        if (!expr_ptr) {
            return "null";
        }
        
        return std::visit([](const auto& expr) -> std::string {
            using T = std::decay_t<decltype(expr)>;
            if constexpr (std::is_same_v<T, my_and>) {
                return "AND";
            } else if constexpr (std::is_same_v<T, my_or>) {
                return "OR";
            } else if constexpr (std::is_same_v<T, my_not>) {
                return "NOT";
            } else if constexpr (std::is_same_v<T, my_xor>) {
                return "XOR";
            } else if constexpr (std::is_same_v<T, my_variable>) {
                return expr.name;  // Assuming my_variable has a name field
            }
            return "unknown";
        }, *expr_ptr);
    }
    
    /**
     * @brief Get the shape for this expression node
     */
    std::string get_shape() const {
        if (!expr_ptr) {
            return "box";
        }
        
        return std::visit([](const auto& expr) -> std::string {
            using T = std::decay_t<decltype(expr)>;
            if constexpr (std::is_same_v<T, my_and> || std::is_same_v<T, my_or>) {
                return "box";
            } else if constexpr (std::is_same_v<T, my_not>) {
                return "invtriangle";
            } else if constexpr (std::is_same_v<T, my_xor>) {
                return "diamond";
            } else if constexpr (std::is_same_v<T, my_variable>) {
                return "ellipse";
            }
            return "box";
        }, *expr_ptr);
    }
    
    /**
     * @brief Get the fill color for this expression node
     */
    std::string get_fillcolor() const {
        if (!expr_ptr) {
            return "lightgray";
        }
        
        return std::visit([](const auto& expr) -> std::string {
            using T = std::decay_t<decltype(expr)>;
            if constexpr (std::is_same_v<T, my_and>) {
                return "lightgreen";
            } else if constexpr (std::is_same_v<T, my_or>) {
                return "lightcoral";
            } else if constexpr (std::is_same_v<T, my_not>) {
                return "lightyellow";
            } else if constexpr (std::is_same_v<T, my_xor>) {
                return "lightcyan";
            } else if constexpr (std::is_same_v<T, my_variable>) {
                return "lightblue";
            }
            return "white";
        }, *expr_ptr);
    }
    
    /**
     * @brief Get the style for this expression node
     */
    std::string get_style() const {
        return expr_ptr ? "filled" : "dashed";
    }
    
    /**
     * @brief Get the font color for this expression node
     */
    std::string get_fontcolor() const {
        return "black";
    }
    
    /**
     * @brief Get child iterators for this expression
     */
    std::vector<ExpressionIterator> get_children() const {
        std::vector<ExpressionIterator> children;
        
        if (!expr_ptr) {
            return children;
        }
        
        std::visit([&children](const auto& expr) {
            using T = std::decay_t<decltype(expr)>;
            if constexpr (std::is_same_v<T, my_and> || std::is_same_v<T, my_or> || std::is_same_v<T, my_xor>) {
                // Binary operators have left and right children
                if (expr.left) {
                    children.emplace_back(expr.left);
                }
                if (expr.right) {
                    children.emplace_back(expr.right);
                }
            } else if constexpr (std::is_same_v<T, my_not>) {
                // Unary operator has one child
                if (expr.operand) {
                    children.emplace_back(expr.operand);
                }
            }
            // Variables are leaves - no children
        }, *expr_ptr);
        
        return children;
    }
    
    /**
     * @brief Get the edge label for a specific child
     */
    std::string get_edge_label(const ExpressionIterator& child, size_t child_index) const {
        // Label edges based on child position
        if (child_index == 0) {
            return "L";  // Left child
        } else if (child_index == 1) {
            return "R";  // Right child  
        } else {
            return std::to_string(child_index);
        }
    }
    
    /**
     * @brief Get the edge style for a specific child
     */
    std::string get_edge_style(const ExpressionIterator& child, size_t child_index) const {
        return "solid";
    }
    
    /**
     * @brief Get the edge color for a specific child
     */
    std::string get_edge_color(const ExpressionIterator& child, size_t child_index) const {
        // Color edges based on child position
        if (child_index == 0) {
            return "blue";  // Left child
        } else if (child_index == 1) {
            return "red";   // Right child  
        } else {
            return "black";
        }
    }
    
    /**
     * @brief Get the edge font color for a specific child
     */
    std::string get_edge_fontcolor(const ExpressionIterator& child, size_t child_index) const {
        return get_edge_color(child, child_index); // Match the edge color
    }
    
    /**
     * @brief Get a unique identifier for this expression node
     */
    std::string get_node_id() const {
        return "node_" + std::to_string(reinterpret_cast<uintptr_t>(expr_ptr.get()));
    }
    
    /**
     * @brief Check if this expression should be included in the graph
     */
    bool should_process() const {
        return expr_ptr != nullptr;  // Only process non-null expressions
    }
};

/**
 * @brief Convenience function to generate DOT graph for expression trees
 */
void generate_expression_dot(std::shared_ptr<my_expression> root_expr,
                           std::ostream& out,
                           const std::string& graph_name = "ExpressionTree") {
    ExpressionIterator root_iterator(root_expr);
    dot_template::generate_dot_graph(root_iterator, out, graph_name);
}

/**
 * @brief Generate DOT graph with custom configuration
 */
void generate_expression_dot(std::shared_ptr<my_expression> root_expr,
                           std::ostream& out,
                           const dot_template::DotConfig& config) {
    ExpressionIterator root_iterator(root_expr);
    dot_template::generate_dot_graph(root_iterator, out, config);
}

} // namespace dot_example

/**
 * USAGE EXAMPLE:
 * 
 * #include "dot_example.hpp"
 * #include <fstream>
 * 
 * // Assuming you have an expression tree
 * auto expr = build_my_expression_tree();
 * 
 * // Generate DOT file - pure iterator approach!
 * std::ofstream file("expression.dot");
 * dot_example::generate_expression_dot(expr, file);
 * 
 * // Or with custom config
 * dot_template::DotConfig config("MyExpression");
 * config.rankdir = "LR";  // Left-to-right layout
 * dot_example::generate_expression_dot(expr, file, config);
 */