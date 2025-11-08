/**
 * @file dot_template_test.cpp
 * @brief Test program demonstrating the pure iterator DOT template system
 * 
 * This file shows how to use the pure iterator DOT template where the iterator
 * represents both the tree node and the traversal logic, eliminating the need
 * for separate data types.
 * 
 * Compile with: cl /std:c++17 /EHsc /I. dot_template_test.cpp /Fe:dot_template_test.exe
 * 
 * @author BDD Expression Converter Project
 * @date November 8, 2025
 */

#include "dot_template.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

/**
 * @brief Self-contained iterator that represents a tree node
 */
class TreeNodeIterator {
private:
    std::string name;
    std::string type;
    std::vector<std::shared_ptr<TreeNodeIterator>> child_iterators;
    
public:
    TreeNodeIterator(const std::string& n, const std::string& t = "default") 
        : name(n), type(t) {}
    
    void add_child(std::shared_ptr<TreeNodeIterator> child) {
        child_iterators.push_back(child);
    }
    
    // Pure iterator interface with individual property methods!
    std::string get_label() const {
        return name;
    }
    
    std::string get_shape() const {
        if (type == "operator") {
            return "box";
        } else if (type == "leaf") {
            return "ellipse";
        } else {
            return "diamond";
        }
    }
    
    std::string get_style() const {
        return "filled";
    }
    
    std::string get_fillcolor() const {
        if (type == "operator") {
            return "lightgreen";
        } else if (type == "leaf") {
            return "lightyellow";
        } else {
            return "lightblue";
        }
    }
    
    std::string get_fontcolor() const {
        return "black";
    }
    
    std::string get_tooltip() const {
        return type + ": " + name;
    }
    
    std::vector<TreeNodeIterator> get_children() const {
        std::vector<TreeNodeIterator> result;
        for (const auto& child_ptr : child_iterators) {
            result.push_back(*child_ptr);  // Copy the child iterator
        }
        return result;
    }
    
    std::string get_edge_label(const TreeNodeIterator& child, size_t index) const {
        return std::to_string(index);
    }
    
    std::string get_edge_style(const TreeNodeIterator& child, size_t index) const {
        return "solid";
    }
    
    std::string get_edge_color(const TreeNodeIterator& child, size_t index) const {
        return "black";
    }
    
    std::string get_edge_fontcolor(const TreeNodeIterator& child, size_t index) const {
        return "blue";
    }
    
    std::string get_node_id() const {
        return "node_" + std::to_string(reinterpret_cast<uintptr_t>(this));
    }
    
    // Optional method - automatically detected via SFINAE
    bool should_process() const {
        return true; // Process all nodes
    }
};

/**
 * @brief Create a test tree using the pure iterator approach
 */
std::shared_ptr<TreeNodeIterator> create_test_tree() {
    // Create the tree structure
    auto root = std::make_shared<TreeNodeIterator>("Expression", "root");
    auto left = std::make_shared<TreeNodeIterator>("AND", "operator");
    auto right = std::make_shared<TreeNodeIterator>("OR", "operator");
    
    // Add leaves to left subtree
    left->add_child(std::make_shared<TreeNodeIterator>("A", "leaf"));
    left->add_child(std::make_shared<TreeNodeIterator>("B", "leaf"));
    
    // Add leaves to right subtree  
    right->add_child(std::make_shared<TreeNodeIterator>("C", "leaf"));
    right->add_child(std::make_shared<TreeNodeIterator>("D", "leaf"));
    
    // Connect subtrees to root
    root->add_child(left);
    root->add_child(right);
    
    return root;
}

int main() {
    std::cout << "Pure Iterator DOT Template Test Program\n";
    std::cout << "======================================\n\n";
    
    // Create test tree
    auto tree = create_test_tree();
    
    // Generate DOT output to console
    std::cout << "Generated DOT graph:\n";
    std::cout << "-------------------\n";
    
    dot_template::DotConfig config("TestTree");
    config.rankdir = "TB";
    
    // Notice: No separate data type! Iterator IS the data.
    // Template signature: generate_dot_graph<Iterator>(iterator, out, config)
    dot_template::generate_dot_graph(*tree, std::cout, config);
    
    // Also write to file
    std::ofstream file("test_tree.dot");
    if (file.is_open()) {
        dot_template::generate_dot_graph(*tree, file, config);
        file.close();
        std::cout << "\nDOT graph also written to 'test_tree.dot'\n";
        std::cout << "Visualize with: dot -Tpng test_tree.dot -o test_tree.png\n";
    } else {
        std::cout << "\nCould not write to file.\n";
    }
    
    std::cout << "\nPure Iterator Approach Benefits:\n";
    std::cout << "- No value_type typedef needed\n";
    std::cout << "- Iterator represents the actual tree node\n";
    std::cout << "- get_children() returns iterators, not data objects\n";
    std::cout << "- Methods take no parameters (iterator knows its own state)\n";
    std::cout << "- Template signature: generate_dot_graph<Iterator>(iterator, out)\n";
    std::cout << "- Cleanest possible API!\n";
    
    return 0;
}