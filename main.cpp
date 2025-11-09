// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file main.cpp
 * @brief BDD Expression Converter - A comprehensive Binary Decision Diagram conversion tool
 *
 * A comprehensive Binary Decision Diagram (BDD) conversion tool built with the
 * TeDDy library. Converts logical expressions from text files into BDDs and
 * generates multiple output formats including DOT graphs and node tables.
 *
 * @author Alan Jowett
 * @date 2025
 * @copyright Copyright (c) 2025 Alan Jowett. Licensed under the MIT License - see LICENSE file for
 * details
 *
 * Features:
 * - Parse logical expressions with AND, OR, XOR, NOT operators
 * - Convert expressions to Binary Decision Diagrams (BDDs)
 * - Generate DOT graph representations for visualization
 * - Output detailed BDD node tables
 * - Support for variable names and complex nested expressions
 */

#include <algorithm>
#include <array>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <libteddy/core.hpp>
#include <memory>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "bdd_graph.hpp"
#include "dag_walker.hpp"
#include "expression_graph.hpp"

/**
 * @brief Writes BDD node table to any output stream
 *
 * This function generates a comprehensive table showing the structure of a BDD,
 * including node indices, variable assignments, and child relationships.
 * Nodes are ordered in post-order traversal and then reversed for proper display.
 *
 * @param manager Reference to the BDD manager
 * @param diagram The BDD diagram to analyze
 * @param variable_names Vector of variable names for display (indexed by variable index)
 * @param out Output stream to write the table to
 * @param include_headers Whether to include descriptive headers and footers (default: true)
 *
 * @note The function uses dag_walker's topological traversal to ensure
 *       that parent nodes have lower indices than their children
 */
void write_bdd_nodes_to_stream(teddy::bdd_manager& manager, teddy::bdd_manager::diagram_t diagram,
                               const std::vector<std::string>& variable_names, std::ostream& out,
                               bool include_headers = true) {
    using node_t = teddy::bdd_manager::diagram_t::node_t;

    // Get nodes in topological order using dag_walker
    std::vector<node_t*> nodes_in_order = collect_bdd_nodes_topological(diagram, variable_names);

    // Reverse to match original ordering (parents before children, terminals at end)
    std::reverse(nodes_in_order.begin(), nodes_in_order.end());

    // Create node-to-index mapping
    std::unordered_map<node_t*, int> node_to_index;
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
            std::string var_name = (var_index < variable_names.size())
                                       ? variable_names[var_index]
                                       : ("x" + std::to_string(var_index));
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

/**
 * @brief Trims leading and trailing whitespace from a string
 *
 * Removes space, tab, newline, and carriage return characters from
 * the beginning and end of the input string.
 *
 * @param str Input string to trim
 * @return Trimmed string with whitespace removed
 */
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
        return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

/**
 * @brief Parses a logical expression string into an expression tree
 *
 * Recursive descent parser that handles operator precedence and parentheses.
 * Supports the following operators in order of precedence (lowest to highest):
 * 1. XOR (exclusive or)
 * 2. OR (logical or)
 * 3. AND (logical and)
 * 4. NOT (logical negation)
 *
 * The parser removes unnecessary outer parentheses and handles nested expressions.
 * Variable names can be any string not containing operators or parentheses.
 *
 * @param expr_str String representation of the logical expression
 * @return Smart pointer to the root of the parsed expression tree
 *
 * @note Operator precedence: NOT > AND > OR > XOR
 * @note Parsing is done right-to-left for left-associative operators
 *
 * Example expressions:
 * - "x AND y"
 * - "(a OR b) AND NOT c"
 * - "input1 XOR (input2 AND input3)"
 */
my_expression_ptr parse_expression(const std::string& expr_str) {
    std::string trimmed = trim(expr_str);

    // Remove outer parentheses if they exist and are balanced
    if (trimmed.front() == '(' && trimmed.back() == ')') {
        int depth = 0;
        bool valid_outer_parens = true;
        for (size_t i = 0; i < trimmed.length() - 1; ++i) {
            if (trimmed[i] == '(')
                depth++;
            else if (trimmed[i] == ')')
                depth--;
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
        if (c == ')')
            depth++;
        else if (c == '(')
            depth--;
        else if (depth == 0) {
            if (i >= 2 && trimmed.substr(i - 2, 3) == "XOR") {
                if (xor_pos == -1)
                    xor_pos = i - 2;
            } else if (i >= 1 && trimmed.substr(i - 1, 2) == "OR") {
                if (or_pos == -1)
                    or_pos = i - 1;
            } else if (i >= 2 && trimmed.substr(i - 2, 3) == "AND") {
                if (and_pos == -1)
                    and_pos = i - 2;
            }
        }
    }

    // Parse based on operator precedence: XOR < OR < AND
    if (xor_pos != -1) {
        std::string left = trim(trimmed.substr(0, xor_pos));
        std::string right = trim(trimmed.substr(xor_pos + 3));
        return std::make_unique<my_expression>(
            my_xor{parse_expression(left), parse_expression(right)});
    } else if (or_pos != -1) {
        std::string left = trim(trimmed.substr(0, or_pos));
        std::string right = trim(trimmed.substr(or_pos + 2));
        return std::make_unique<my_expression>(
            my_or{parse_expression(left), parse_expression(right)});
    } else if (and_pos != -1) {
        std::string left = trim(trimmed.substr(0, and_pos));
        std::string right = trim(trimmed.substr(and_pos + 3));
        return std::make_unique<my_expression>(
            my_and{parse_expression(left), parse_expression(right)});
    }

    // Check for NOT
    if (trimmed.length() > 3 && trimmed.substr(0, 3) == "NOT") {
        std::string operand = trim(trimmed.substr(3));
        return std::make_unique<my_expression>(my_not{parse_expression(operand)});
    }

    // Must be a variable
    return std::make_unique<my_expression>(my_variable{trimmed});
}

/**
 * @brief Constructs output file path with suffix in same directory as input file
 *
 * Takes an input file path and generates a new path in the same directory
 * with the same base name but with an added suffix before the extension.
 *
 * @param input_filepath Path to the input file
 * @param suffix String to append to the base filename
 * @return New filesystem path with the suffix added
 *
 * Example: input_filepath="test/expr.txt", suffix="_bdd" → "test/expr_bdd"
 */
std::filesystem::path get_output_path(const std::filesystem::path& input_filepath,
                                      const std::string& suffix) {
    std::filesystem::path dir = input_filepath.parent_path();
    std::string base_name = input_filepath.stem().string();
    return dir / (base_name + suffix);
}

/**
 * @brief Reads and parses a logical expression from a text file
 *
 * Reads all non-empty, non-comment lines from the specified file and
 * concatenates them into a single expression string. Lines starting with '#'
 * are treated as comments and ignored.
 *
 * @param filename Path to the file containing the expression
 * @return Smart pointer to the parsed expression tree
 *
 * @throws std::runtime_error If file cannot be opened or no valid expression is found
 *
 * File format:
 * - One or more lines containing parts of the expression
 * - Empty lines are ignored
 * - Lines starting with '#' are treated as comments
 * - Expression parts are concatenated with spaces
 *
 * Example file content:
 * ```
 * # This is a comment
 * (x0 AND x1) OR
 * (NOT x2) XOR (x3 AND (NOT x4))
 * ```
 */
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

/**
 * @brief Converts an expression tree to a Binary Decision Diagram (BDD)
 *
 * Performs recursive conversion of the custom expression tree into a BDD using
 * the TeDDy library. Uses dag_walker for efficient variable collection and
 * simple recursion for BDD construction since expressions form a tree structure.
 *
 * The conversion process:
 * 1. Uses dag_walker to collect all unique variable names efficiently
 * 2. Creates a sorted variable mapping for consistent BDD ordering
 * 3. Recursively converts each expression node to BDD operations
 *
 * @param expr The root expression to convert
 * @param mgr Reference to the BDD manager for creating BDD nodes
 * @return BDD diagram representing the logical function
 *
 * @throws std::runtime_error If a variable reference is not found during conversion
 *
 * Operator mappings:
 * - AND → BDD AND operation
 * - OR → BDD OR operation
 * - XOR → BDD XOR operation
 * - NOT → XOR with constant 1
 * - Variable → BDD variable node
 */
teddy::bdd_manager::diagram_t convert_to_bdd(const my_expression& expr, teddy::bdd_manager& mgr) {
    using bdd_t = teddy::bdd_manager::diagram_t;
    using namespace teddy::ops;

    // First pass: collect all unique variable names using dag_walker
    std::unordered_set<std::string> variable_names;
    collect_variables_with_dag_walker(expr, variable_names);

    // Build variable map with sorted variable names for consistent ordering
    std::vector<std::string> sorted_vars(variable_names.begin(), variable_names.end());
    std::sort(sorted_vars.begin(), sorted_vars.end());

    std::unordered_map<std::string, int> var_map;
    for (size_t i = 0; i < sorted_vars.size(); ++i) {
        var_map[sorted_vars[i]] = static_cast<int>(i);
    }

    // Helper function for recursive conversion (no memoization needed for tree structure)
    std::function<bdd_t(const my_expression&)> convert_recursive =
        [&](const my_expression& e) -> bdd_t {
        return std::visit(
            [&](const auto& variant_expr) -> bdd_t {
                using T = std::decay_t<decltype(variant_expr)>;

                if constexpr (std::is_same_v<T, my_variable>) {
                    auto it = var_map.find(variant_expr.variable_name);
                    if (it == var_map.end()) {
                        throw std::runtime_error("Variable not found: "
                                                 + variant_expr.variable_name);
                    }
                    return mgr.variable(it->second);
                } else if constexpr (std::is_same_v<T, my_and>) {
                    bdd_t left_bdd = convert_recursive(*variant_expr.left);
                    bdd_t right_bdd = convert_recursive(*variant_expr.right);
                    return mgr.apply<AND>(left_bdd, right_bdd);
                } else if constexpr (std::is_same_v<T, my_or>) {
                    bdd_t left_bdd = convert_recursive(*variant_expr.left);
                    bdd_t right_bdd = convert_recursive(*variant_expr.right);
                    return mgr.apply<OR>(left_bdd, right_bdd);
                } else if constexpr (std::is_same_v<T, my_not>) {
                    bdd_t expr_bdd = convert_recursive(*variant_expr.expr);
                    bdd_t one = mgr.constant(1);
                    return mgr.apply<XOR>(expr_bdd, one);
                } else if constexpr (std::is_same_v<T, my_xor>) {
                    bdd_t left_bdd = convert_recursive(*variant_expr.left);
                    bdd_t right_bdd = convert_recursive(*variant_expr.right);
                    return mgr.apply<XOR>(left_bdd, right_bdd);
                }
            },
            e);
    };

    return convert_recursive(expr);
}

/**
 * @brief Main function - BDD Expression Converter application entry point
 *
 * This application reads logical expressions from text files, converts them to
 * Binary Decision Diagrams (BDDs), and generates various output formats for
 * analysis and visualization.
 *
 * Command line usage:
 * - `program` : Uses default file "test_expressions/filter_expression.txt"
 * - `program <filename>` : Processes the specified expression file
 * - `program <filename> [options]` : Processes file with specified options
 *
 * Options:
 * - `--enable-reordering` : Enable automatic variable reordering for optimization
 * - `--disable-reordering` : Disable variable reordering (default)
 * - `--force-reorder` : Force immediate reordering after BDD construction
 * - `--help` or `-h` : Show help message
 *
 * Generated outputs (in same directory as input file):
 * - `*_expression_tree.dot` : DOT graph of the original expression tree
 * - `*_bdd.dot` : DOT graph of the resulting BDD
 * - `*_bdd_nodes.txt` : Detailed table of BDD nodes and structure
 *
 * The program automatically:
 * 1. Parses the input expression file
 * 2. Builds an abstract syntax tree (AST)
 * 3. Converts the AST to a BDD
 * 4. Applies variable reordering if requested
 * 5. Generates visualization and analysis outputs
 * 6. Displays results to console
 *
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return 0 on success, 1 on error
 *
 * @note Generated DOT files can be visualized using Graphviz tools:
 *       `dot -Tpng input.dot -o output.png`
 */
int main(int argc, const char* argv[]) {
    std::filesystem::path input_file = "";
    bool enable_auto_reordering = false;
    bool force_reorder_after_build = false;
    bool show_help = false;
    bool help_due_to_error = false;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--enable-reordering") {
            enable_auto_reordering = true;
        } else if (arg == "--disable-reordering") {
            enable_auto_reordering = false;
        } else if (arg == "--force-reorder") {
            force_reorder_after_build = true;
        } else if (arg == "--help" || arg == "-h") {
            show_help = true;
            break;
        } else if (arg.starts_with("--")) {
            std::cerr << "Unknown option: " << arg << "\n";
            show_help = true;
            help_due_to_error = true;
            break;
        } else if (input_file.empty()) {
            input_file = arg;
        } else {
            std::cerr << "Multiple input files specified. Only one file is allowed.\n";
            show_help = true;
            help_due_to_error = true;
            break;
        }
    }

    if (show_help) {
        std::cout << "TeDDy BDD Demo - Binary Decision Diagram Converter\n";
        std::cout << "==================================================\n\n";
        std::cout << "Usage: bdd_demo [filename] [options]\n\n";
        std::cout << "Arguments:\n";
        std::cout << "  filename              Input expression file (optional, defaults to "
                     "test_expressions/filter_expression.txt)\n\n";
        std::cout << "Options:\n";
        std::cout << "  --enable-reordering   Enable automatic variable reordering for BDD "
                     "optimization\n";
        std::cout << "  --disable-reordering  Disable variable reordering (default)\n";
        std::cout << "  --force-reorder       Force immediate reordering after BDD construction\n";
        std::cout << "  --help, -h            Show this help message\n\n";
        std::cout << "Example expression file format:\n";
        std::cout << "  # This is a comment\n";
        std::cout << "  (x0 AND x1) OR (NOT x2) XOR (x3 AND (NOT x4))\n\n";
        std::cout << "Supported operators: AND, OR, XOR, NOT\n";
        std::cout << "Use parentheses for grouping\n\n";
        std::cout << "Generated DOT files can be visualized using Graphviz tools:\n";
        std::cout << "  dot -Tpng input.dot -o output.png\n";
        return help_due_to_error ? 1 : 0;
    }

    // Use default file if none specified
    if (input_file.empty()) {
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
    collect_variables_with_dag_walker(*expr, variable_names);

    // Create sorted variable names for consistent ordering (same as in convert_to_bdd)
    std::vector<std::string> sorted_variable_names(variable_names.begin(), variable_names.end());
    std::sort(sorted_variable_names.begin(), sorted_variable_names.end());

    // Create a BDD manager with the appropriate number of variables
    teddy::bdd_manager manager(static_cast<int>(variable_names.size()), 1'000);

    // Configure variable reordering based on command line options
    if (enable_auto_reordering) {
        manager.set_auto_reorder(true);
        std::cout << "Automatic variable reordering enabled\n";
    } else {
        std::cout << "Automatic variable reordering disabled\n";
    }

    // Alias for the diagram type for convenience
    using bdd_t = teddy::bdd_manager::diagram_t;

    // Import operator namespace for cleaner syntax
    using namespace teddy::ops;

    // Convert the expression tree to BDD (variable map built dynamically)
    bdd_t f = convert_to_bdd(*expr, manager);

    // Force variable reordering if requested
    if (force_reorder_after_build) {
        std::cout << "Forcing variable reordering after BDD construction...\n";
        manager.force_reorder();
        std::cout << "Variable reordering completed\n";
    }

    std::cout << "Function created successfully!\n";
    std::cout << "Using " << variable_names.size() << " variables\n\n";

    // Generate output filenames in the same directory as the input file
    std::filesystem::path expr_dot_filename = get_output_path(input_file, "_expression_tree.dot");
    std::filesystem::path bdd_dot_filename = get_output_path(input_file, "_bdd.dot");
    std::filesystem::path bdd_nodes_filename = get_output_path(input_file, "_bdd_nodes.txt");

    // Output expression tree as DOT file using v2 function
    std::ofstream expr_dot_file(expr_dot_filename);
    if (expr_dot_file.is_open()) {
        // Generate complete DOT graph using updated function
        write_expression_to_dot(*expr, expr_dot_file, "ExpressionTree");

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
    write_bdd_nodes_to_stream(manager, f, sorted_variable_names, std::cout, true);
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
        write_bdd_nodes_to_stream(manager, f, sorted_variable_names, nodes_file, false);
        nodes_file.close();
        std::cout << "BDD node table saved to '" << bdd_nodes_filename << "'\n";
    } else {
        std::cerr << "Error: Could not create output file '" << bdd_nodes_filename << "'\n";
        return 1;
    }

    std::cout << "\nDemo completed successfully!\n";
    return 0;
}
