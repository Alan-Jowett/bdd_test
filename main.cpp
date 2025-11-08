/*
 * BDD Expression Converter
 * 
 * A comprehensive Binary Decision Diagram (BDD) conversion tool built with the TeDDy library.
 * Converts logical expressions from text files into BDDs and generates multiple output formats.
 * 
 * Copyright (c) 2025 Alan Jowett
 * Licensed under the MIT License - see LICENSE file for details
 */

#include <libteddy/core.hpp>
#include <array>
#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <iomanip>
#include <stack>
#include <algorithm>
#include <variant>
#include <memory>
#include <sstream>
#include <string>
#include <filesystem>

// Common function to format BDD node table to any output stream
void write_bdd_nodes_to_stream(teddy::bdd_manager& manager, teddy::bdd_manager::diagram_t diagram, const std::vector<std::string>& variable_names, std::ostream& out, bool include_headers = true) {
    using node_t = teddy::bdd_manager::diagram_t::node_t;
    std::unordered_set<node_t*> visited;
    std::unordered_map<node_t*, int> node_to_index;
    std::vector<node_t*> nodes_in_order;
    
    // Step 1: Gather nodes in post-order (children before parents)
    std::stack<std::pair<node_t*, bool>> stack;  // pair of (node, processed)
    stack.push({diagram.unsafe_get_root(), false});
    
    while (!stack.empty()) {
        auto [node, processed] = stack.top();
        
        if (!node || visited.find(node) != visited.end()) {
            stack.pop();
            continue;
        }
        
        if (!processed) {
            // Mark as being processed and push children first
            stack.top().second = true;
            
            if (!node->is_terminal()) {
                // Push children (they will be processed before parent)
                stack.push({node->get_son(1), false});  // true child
                stack.push({node->get_son(0), false});  // false child
            }
        } else {
            // Process the node (children are already processed)
            stack.pop();
            visited.insert(node);
            
            // Assign index and collect node
            int index = static_cast<int>(nodes_in_order.size());
            node_to_index[node] = index;
            nodes_in_order.push_back(node);
        }
    }
    
    // Step 2: Reverse the order (highest index becomes 0)
    std::reverse(nodes_in_order.begin(), nodes_in_order.end());
    
    // Step 3: Rebuild index mapping for the reversed order
    node_to_index.clear();
    for (int i = 0; i < nodes_in_order.size(); ++i) {
        node_to_index[nodes_in_order[i]] = i;
    }
    
    // Step 4: Write all nodes in the final order to stream
    if (include_headers) {
        out << "BDD Node Table (Post-order + reverse - correct ordering):\n";
    }
    out << "Index | Variable | False Child | True Child | Type\n";
    out << "------|----------|-------------|------------|----------\n";
    
    for (int i = 0; i < nodes_in_order.size(); ++i) {
        node_t* node = nodes_in_order[i];
        
        out << std::setw(5) << i << " | ";
        
        if (node->is_terminal()) {
            out << std::setw(8) << "-" << " | ";
            out << std::setw(11) << "-" << " | ";
            out << std::setw(10) << "-" << " | ";
            out << "Terminal(" << node->get_value() << ")";
        } else {
            int var_index = node->get_index();
            std::string var_name = (var_index < variable_names.size()) ? variable_names[var_index] : ("x" + std::to_string(var_index));
            out << std::setw(8) << var_name << " | ";
            
            node_t* false_child = node->get_son(0);
            node_t* true_child = node->get_son(1);
            
            // Use the final indices
            out << std::setw(11) << node_to_index[false_child] << " | ";
            out << std::setw(10) << node_to_index[true_child] << " | ";
            out << "Variable";
        }
        out << "\n";
    }
    
    if (include_headers) {
        out << "\nTotal nodes: " << nodes_in_order.size() << "\n";
        out << "Note: True post-order + reverse ordering.\n";
    }
}

// Function to print BDD nodes to console (wrapper for common implementation)
void print_bdd_nodes(teddy::bdd_manager& manager, teddy::bdd_manager::diagram_t diagram, const std::vector<std::string>& variable_names) {
    write_bdd_nodes_to_stream(manager, diagram, variable_names, std::cout, true);
}

// Function to write BDD node table to file (wrapper for common implementation)
void write_bdd_nodes_to_file(teddy::bdd_manager& manager, teddy::bdd_manager::diagram_t diagram, const std::vector<std::string>& variable_names, std::ostream& out) {
    write_bdd_nodes_to_stream(manager, diagram, variable_names, out, false);
}

// Custom BDD DOT generation function with real variable names
void write_bdd_to_dot(teddy::bdd_manager& manager, teddy::bdd_manager::diagram_t diagram, const std::vector<std::string>& variable_names, std::ostream& out) {
    using node_t = teddy::bdd_manager::diagram_t::node_t;
    std::unordered_set<node_t*> visited;
    
    out << "digraph DD {\n";
    
    // First pass: identify terminal nodes for styling
    std::stack<node_t*> stack;
    stack.push(diagram.unsafe_get_root());
    
    std::unordered_set<node_t*> terminal_nodes;
    while (!stack.empty()) {
        node_t* node = stack.top();
        stack.pop();
        
        if (!node || visited.find(node) != visited.end()) {
            continue;
        }
        
        visited.insert(node);
        
        if (node->is_terminal()) {
            terminal_nodes.insert(node);
        } else {
            stack.push(node->get_son(0));  // false child
            stack.push(node->get_son(1));  // true child
        }
    }
    
    // Style terminal nodes differently
    out << "    node [shape = square]";
    for (node_t* terminal : terminal_nodes) {
        out << " " << reinterpret_cast<std::uintptr_t>(terminal);
    }
    out << ";\n";
    out << "    node [shape = circle];\n\n";
    
    // Second pass: generate node definitions and edges
    visited.clear();
    stack.push(diagram.unsafe_get_root());
    
    while (!stack.empty()) {
        node_t* node = stack.top();
        stack.pop();
        
        if (!node || visited.find(node) != visited.end()) {
            continue;
        }
        
        visited.insert(node);
        
        // Generate node definition
        std::uintptr_t node_ptr = reinterpret_cast<std::uintptr_t>(node);
        
        if (node->is_terminal()) {
            out << "    " << node_ptr << " [label = \"" << node->get_value() << "\", tooltip = \"" << node->get_value() << "\"];\n";
        } else {
            int var_index = node->get_index();
            std::string var_name = (var_index < variable_names.size()) ? variable_names[var_index] : ("x" + std::to_string(var_index));
            out << "    " << node_ptr << " [label = \"" << var_name << "\", tooltip = \"" << var_index << "\"];\n";
        }
        
        // Generate edges for non-terminal nodes
        if (!node->is_terminal()) {
            node_t* false_child = node->get_son(0);
            node_t* true_child = node->get_son(1);
            
            if (false_child) {
                out << "    " << node_ptr << " -> " << reinterpret_cast<std::uintptr_t>(false_child) << " [style = dashed];\n";
                stack.push(false_child);
            }
            
            if (true_child) {
                out << "    " << node_ptr << " -> " << reinterpret_cast<std::uintptr_t>(true_child) << " [style = solid];\n";
                stack.push(true_child);
            }
        }
    }
    
    out << "\n}";
}

struct my_and;
struct my_or;
struct my_not;
struct my_xor;
struct my_variable;

using my_expression = std::variant<my_and, my_or, my_not, my_xor, my_variable>;
using my_expression_ptr = std::unique_ptr<my_expression>;

struct my_variable {
    std::string variable_name;
};

struct my_and {
    my_expression_ptr left;
    my_expression_ptr right;
};

struct my_or {
    my_expression_ptr left;
    my_expression_ptr right;
};

struct my_not {
    my_expression_ptr expr;
};

struct my_xor {
    my_expression_ptr left;
    my_expression_ptr right;
};

// Function to collect all variable names from the expression tree
void collect_variables(const my_expression& expr, std::unordered_set<std::string>& variables) {
    std::visit([&](const auto& variant_expr) {
        using T = std::decay_t<decltype(variant_expr)>;
        
        if constexpr (std::is_same_v<T, my_variable>) {
            variables.insert(variant_expr.variable_name);
        }
        else if constexpr (std::is_same_v<T, my_and> || std::is_same_v<T, my_or> || std::is_same_v<T, my_xor>) {
            collect_variables(*variant_expr.left, variables);
            collect_variables(*variant_expr.right, variables);
        }
        else if constexpr (std::is_same_v<T, my_not>) {
            collect_variables(*variant_expr.expr, variables);
        }
    }, expr);
}

// Function to write expression tree as DOT graph
void write_expression_to_dot(const my_expression& expr, std::ostream& out, int& node_id, int parent_id = -1, const std::string& edge_label = "") {
    static bool first_call = true;
    
    std::visit([&](const auto& variant_expr) {
        using T = std::decay_t<decltype(variant_expr)>;
        
        int current_node = node_id++;
        
        if constexpr (std::is_same_v<T, my_variable>) {
            out << "    node" << current_node << " [label=\"" << variant_expr.variable_name << "\", shape=ellipse, style=filled, fillcolor=lightblue];\n";
        }
        else if constexpr (std::is_same_v<T, my_and>) {
            out << "    node" << current_node << " [label=\"AND\", shape=box, style=filled, fillcolor=lightgreen];\n";
            write_expression_to_dot(*variant_expr.left, out, node_id, current_node, "L");
            write_expression_to_dot(*variant_expr.right, out, node_id, current_node, "R");
        }
        else if constexpr (std::is_same_v<T, my_or>) {
            out << "    node" << current_node << " [label=\"OR\", shape=box, style=filled, fillcolor=lightcoral];\n";
            write_expression_to_dot(*variant_expr.left, out, node_id, current_node, "L");
            write_expression_to_dot(*variant_expr.right, out, node_id, current_node, "R");
        }
        else if constexpr (std::is_same_v<T, my_not>) {
            out << "    node" << current_node << " [label=\"NOT\", shape=box, style=filled, fillcolor=yellow];\n";
            write_expression_to_dot(*variant_expr.expr, out, node_id, current_node, "");
        }
        else if constexpr (std::is_same_v<T, my_xor>) {
            out << "    node" << current_node << " [label=\"XOR\", shape=box, style=filled, fillcolor=lightpink];\n";
            write_expression_to_dot(*variant_expr.left, out, node_id, current_node, "L");
            write_expression_to_dot(*variant_expr.right, out, node_id, current_node, "R");
        }
        
        // Create edge from parent to current node
        if (parent_id != -1) {
            out << "    node" << parent_id << " -> node" << current_node;
            if (!edge_label.empty()) {
                out << " [label=\"" << edge_label << "\"]";
            }
            out << ";\n";
        }
    }, expr);
}

// Function to trim whitespace from string
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

// Function to parse expression from string
my_expression_ptr parse_expression(const std::string& expr_str) {
    std::string trimmed = trim(expr_str);
    
    // Remove outer parentheses if they exist and are balanced
    if (trimmed.front() == '(' && trimmed.back() == ')') {
        int depth = 0;
        bool valid_outer_parens = true;
        for (size_t i = 0; i < trimmed.length() - 1; ++i) {
            if (trimmed[i] == '(') depth++;
            else if (trimmed[i] == ')') depth--;
            if (depth == 0) {
                valid_outer_parens = false;
                break;
            }
        }
        if (valid_outer_parens) {
            trimmed = trimmed.substr(1, trimmed.length() - 2);
            trimmed = trim(trimmed);
        }
    }
    
    // Find the main operator (lowest precedence, rightmost)
    int depth = 0;
    int xor_pos = -1, or_pos = -1, and_pos = -1;
    
    for (int i = trimmed.length() - 1; i >= 0; --i) {
        char c = trimmed[i];
        if (c == ')') depth++;
        else if (c == '(') depth--;
        else if (depth == 0) {
            if (i >= 2 && trimmed.substr(i-2, 3) == "XOR") {
                if (xor_pos == -1) xor_pos = i-2;
            }
            else if (i >= 1 && trimmed.substr(i-1, 2) == "OR") {
                if (or_pos == -1) or_pos = i-1;
            }
            else if (i >= 2 && trimmed.substr(i-2, 3) == "AND") {
                if (and_pos == -1) and_pos = i-2;
            }
        }
    }
    
    // Parse based on operator precedence: XOR < OR < AND
    if (xor_pos != -1) {
        std::string left = trim(trimmed.substr(0, xor_pos));
        std::string right = trim(trimmed.substr(xor_pos + 3));
        return std::make_unique<my_expression>(my_xor{
            parse_expression(left),
            parse_expression(right)
        });
    }
    else if (or_pos != -1) {
        std::string left = trim(trimmed.substr(0, or_pos));
        std::string right = trim(trimmed.substr(or_pos + 2));
        return std::make_unique<my_expression>(my_or{
            parse_expression(left),
            parse_expression(right)
        });
    }
    else if (and_pos != -1) {
        std::string left = trim(trimmed.substr(0, and_pos));
        std::string right = trim(trimmed.substr(and_pos + 3));
        return std::make_unique<my_expression>(my_and{
            parse_expression(left),
            parse_expression(right)
        });
    }
    
    // Check for NOT
    if (trimmed.length() > 3 && trimmed.substr(0, 3) == "NOT") {
        std::string operand = trim(trimmed.substr(3));
        return std::make_unique<my_expression>(my_not{
            parse_expression(operand)
        });
    }
    
    // Must be a variable
    return std::make_unique<my_expression>(my_variable{trimmed});
}

// Function to get output path with suffix in same directory as input file
std::filesystem::path get_output_path(const std::filesystem::path& input_filepath, const std::string& suffix) {
    std::filesystem::path dir = input_filepath.parent_path();
    std::string base_name = input_filepath.stem().string();
    return dir / (base_name + suffix);
}

// Function to read expression from file
my_expression_ptr read_expression_from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    
    std::string line;
    std::string expression_str;
    
    // Read all non-empty lines and concatenate them
    while (std::getline(file, line)) {
        line = trim(line);
        if (!line.empty() && line[0] != '#') {  // Skip empty lines and comments
            expression_str += " " + line;
        }
    }
    
    expression_str = trim(expression_str);
    if (expression_str.empty()) {
        throw std::runtime_error("No expression found in file: " + filename);
    }
    
    std::cout << "Read expression from file: " << expression_str << "\n\n";
    
    return parse_expression(expression_str);
}

// Function to convert custom expression tree to BDD
teddy::bdd_manager::diagram_t convert_to_bdd(
    const my_expression& expr, 
    teddy::bdd_manager& mgr
) {
    using bdd_t = teddy::bdd_manager::diagram_t;
    using namespace teddy::ops;
    
    // First pass: collect all unique variable names
    std::unordered_set<std::string> variable_names;
    collect_variables(expr, variable_names);
    
    // Build variable map with sorted variable names for consistent ordering
    std::vector<std::string> sorted_vars(variable_names.begin(), variable_names.end());
    std::sort(sorted_vars.begin(), sorted_vars.end());
    
    std::unordered_map<std::string, int> var_map;
    for (size_t i = 0; i < sorted_vars.size(); ++i) {
        var_map[sorted_vars[i]] = static_cast<int>(i);
    }
    
    // Helper function for recursive conversion
    std::function<bdd_t(const my_expression&)> convert_recursive = [&](const my_expression& e) -> bdd_t {
        return std::visit([&](const auto& variant_expr) -> bdd_t {
            using T = std::decay_t<decltype(variant_expr)>;
            
            if constexpr (std::is_same_v<T, my_variable>) {
                auto it = var_map.find(variant_expr.variable_name);
                if (it == var_map.end()) {
                    throw std::runtime_error("Variable not found: " + variant_expr.variable_name);
                }
                return mgr.variable(it->second);
            }
            else if constexpr (std::is_same_v<T, my_and>) {
                bdd_t left_bdd = convert_recursive(*variant_expr.left);
                bdd_t right_bdd = convert_recursive(*variant_expr.right);
                return mgr.apply<AND>(left_bdd, right_bdd);
            }
            else if constexpr (std::is_same_v<T, my_or>) {
                bdd_t left_bdd = convert_recursive(*variant_expr.left);
                bdd_t right_bdd = convert_recursive(*variant_expr.right);
                return mgr.apply<OR>(left_bdd, right_bdd);
            }
            else if constexpr (std::is_same_v<T, my_not>) {
                bdd_t expr_bdd = convert_recursive(*variant_expr.expr);
                bdd_t one = mgr.constant(1);
                return mgr.apply<XOR>(expr_bdd, one);
            }
            else if constexpr (std::is_same_v<T, my_xor>) {
                bdd_t left_bdd = convert_recursive(*variant_expr.left);
                bdd_t right_bdd = convert_recursive(*variant_expr.right);
                return mgr.apply<XOR>(left_bdd, right_bdd);
            }
        }, e);
    };
    
    return convert_recursive(expr);
}

int main(int argc, char* argv[])
{
    std::filesystem::path input_file = "";
    
    // Check if a filename was provided as command line argument
    if (argc > 1) {
        input_file = argv[1];
    } else {
        // Use default test expression file
        input_file = "test_expressions/filter_expression.txt";
    }
    
    std::cout << "TeDDy BDD Demo - Building BDD from Filter Expression File\n";
    std::cout << "========================================================\n\n";
    std::cout << "Reading filter expression from: " << input_file << "\n";

    my_expression_ptr expr;
    try {
        expr = read_expression_from_file(input_file.string());
    } catch (const std::exception& e) {
        std::cerr << "Error reading expression file: " << e.what() << "\n";
        std::cerr << "\nExample expression file format:\n";
        std::cerr << "# This is a comment\n";
        std::cerr << "(x0 AND x1) OR (NOT x2) XOR (x3 AND (NOT x4))\n";
        std::cerr << "\nSupported operators: AND, OR, XOR, NOT\n";
        std::cerr << "Use parentheses for grouping\n";
        return 1;
    }

    // Dynamically determine the number of variables needed
    std::unordered_set<std::string> variable_names;
    collect_variables(*expr, variable_names);
    
    // Create sorted variable names for consistent ordering (same as in convert_to_bdd)
    std::vector<std::string> sorted_variable_names(variable_names.begin(), variable_names.end());
    std::sort(sorted_variable_names.begin(), sorted_variable_names.end());
    
    // Create a BDD manager with the appropriate number of variables
    teddy::bdd_manager manager(static_cast<int>(variable_names.size()), 1'000);

    // Alias for the diagram type for convenience
    using bdd_t = teddy::bdd_manager::diagram_t;

    // Import operator namespace for cleaner syntax
    using namespace teddy::ops;
    
    // Convert the expression tree to BDD (variable map built dynamically)
    bdd_t f = convert_to_bdd(*expr, manager);

    std::cout << "Function created successfully!\n";
    std::cout << "Using " << variable_names.size() << " variables\n\n";

    // Generate output filenames in the same directory as the input file
    std::filesystem::path expr_dot_filename = get_output_path(input_file, "_expression_tree.dot");
    std::filesystem::path bdd_dot_filename = get_output_path(input_file, "_bdd.dot");
    std::filesystem::path bdd_nodes_filename = get_output_path(input_file, "_bdd_nodes.txt");

    // Output expression tree as DOT file
    std::ofstream expr_dot_file(expr_dot_filename);
    if (expr_dot_file.is_open()) {
        expr_dot_file << "digraph ExpressionTree {\n";
        expr_dot_file << "    rankdir=TB;\n";
        expr_dot_file << "    node [fontname=\"Arial\"];\n";
        expr_dot_file << "    edge [fontname=\"Arial\"];\n";
        expr_dot_file << "\n";
        
        int node_id = 0;
        write_expression_to_dot(*expr, expr_dot_file, node_id);
        
        expr_dot_file << "}\n";
        expr_dot_file.close();
        std::cout << "Expression tree DOT representation saved to '" << expr_dot_filename << "'\n";
        std::cout << "You can visualize it using Graphviz with: dot -Tpng " << expr_dot_filename 
                  << " -o " << get_output_path(input_file, "_expression_tree.png") << "\n\n";
    } else {
        std::cerr << "Error: Could not create output file '" << expr_dot_filename << "'\n";
    }

    // Print BDD node structure
    std::cout << "BDD Node Structure:\n";
    std::cout << "==================\n";
    print_bdd_nodes(manager, f, sorted_variable_names);
    std::cout << "\n";

    // Output DOT representation to console
    std::cout << "DOT representation of the BDD:\n";
    std::cout << "==============================\n";
    write_bdd_to_dot(manager, f, sorted_variable_names, std::cout);
    std::cout << "\n\n";

    // Output DOT representation to file
    std::ofstream dot_file(bdd_dot_filename);
    if (dot_file.is_open()) {
        write_bdd_to_dot(manager, f, sorted_variable_names, dot_file);
        dot_file.close();
        std::cout << "BDD DOT representation saved to '" << bdd_dot_filename << "'\n";
        std::cout << "You can visualize it using Graphviz with: dot -Tpng " << bdd_dot_filename 
                  << " -o " << get_output_path(input_file, "_bdd.png") << "\n";
    } else {
        std::cerr << "Error: Could not create output file '" << bdd_dot_filename << "'\n";
        return 1;
    }

    // Output BDD node table to file
    std::ofstream nodes_file(bdd_nodes_filename);
    if (nodes_file.is_open()) {
        write_bdd_nodes_to_file(manager, f, sorted_variable_names, nodes_file);
        nodes_file.close();
        std::cout << "BDD node table saved to '" << bdd_nodes_filename << "'\n";
    } else {
        std::cerr << "Error: Could not create output file '" << bdd_nodes_filename << "'\n";
        return 1;
    }

    std::cout << "\nDemo completed successfully!\n";
    return 0;
}