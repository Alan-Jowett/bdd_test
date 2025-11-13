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
#include <format>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <libteddy/core.hpp>
#include <memory>
#include <ranges>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

// CUDD headers for BDD comparison
#include <cudd/cudd.h>

#include <cudd/cuddObj.hh>

#include "cudd_convert.hpp"
#include "cudd_graph.hpp"
#include "dag_walker.hpp"
#include "expression_adapter.hpp"
#include "expression_graph.hpp"
#include "expression_parser.hpp"
#include "node_table_generator.hpp"
#include "teddy_convert.hpp"
#include "teddy_graph.hpp"

// ============================================================================
// Anonymous namespace for implementation details
// ============================================================================

namespace {

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
 * Example: input_filepath="test/expr.txt", suffix="_bdd" -> "test/expr_bdd"
 */
std::filesystem::path get_output_path(const std::filesystem::path& input_filepath,
                                      const std::string& suffix) {
    std::filesystem::path dir = input_filepath.parent_path();
    std::string base_name = input_filepath.stem().string();
    return dir / (base_name + suffix);
}

}  // end anonymous namespace

// ============================================================================
// Exported functions (global namespace)
// ============================================================================

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
 * Options:
 * - `--enable-reordering` : Enable automatic variable reordering for optimization
 * - `--disable-reordering` : Disable variable reordering (default)
 * - `--force-reorder` : Force immediate reordering and reduce after BDD construction
 * - `--method=custom` : Use custom recursive conversion method (default)
 * - `--method=teddy` : Use TeDDy's from_expression_tree method
 * - `--quiet` or `-q` : Suppress console output of BDD structure and DOT graph (default)
 * - `--verbose` or `-v` : Show detailed console output of BDD structure and DOT graph
 * - `--mermaid` or `-m` : Generate Mermaid format graphs for Markdown embedding
 * - `--help` or `-h` : Show help message
 *
 * Generated outputs (in same directory as input file):
 * - `*_expression_tree.dot` : DOT graph of the original expression tree
 * - `*_bdd.dot` : DOT graph of the resulting BDD
 * - `*_bdd_nodes.txt` : Detailed table of BDD nodes and structure
 * - `*_expression_tree.md` : Mermaid graph of expression tree (with --mermaid)
 * - `*_bdd.md` : Mermaid graph of BDD structure (with --mermaid)
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
    enum class ConversionMethod {
        Custom,
        TeDDy,
        CUDD
    } conversion_method = ConversionMethod::Custom;
    bool quiet_mode = true;
    bool show_help = false;
    bool help_due_to_error = false;
    bool generate_mermaid = false;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--enable-reordering") {
            enable_auto_reordering = true;
        } else if (arg == "--disable-reordering") {
            enable_auto_reordering = false;
        } else if (arg == "--force-reorder") {
            force_reorder_after_build = true;
        } else if (arg == "--method=custom") {
            conversion_method = ConversionMethod::Custom;
        } else if (arg == "--method=teddy") {
            conversion_method = ConversionMethod::TeDDy;
        } else if (arg == "--method=cudd") {
            conversion_method = ConversionMethod::CUDD;
        } else if (arg == "--quiet" || arg == "-q") {
            quiet_mode = true;
        } else if (arg == "--verbose" || arg == "-v") {
            quiet_mode = false;
        } else if (arg == "--mermaid" || arg == "-m") {
            generate_mermaid = true;
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
        std::cout << "  --force-reorder       Force immediate reordering and reduce after BDD "
                     "construction\n";
        std::cout << "  --method=custom       Use custom recursive conversion method (default)\n";
        std::cout << "  --method=teddy        Use TeDDy's from_expression_tree method\n";
        std::cout << "  --method=cudd         Use CUDD library for BDD conversion\n";
        std::cout << "  --quiet, -q           Suppress console output of BDD structure and DOT "
                     "graph (default)\n";
        std::cout << "  --verbose, -v         Show detailed console output of BDD structure and "
                     "DOT graph\n";
        std::cout
            << "  --mermaid, -m         Generate Mermaid format graphs for Markdown embedding\n";
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

    // Read the original expression text first for later use in Mermaid output
    std::string original_expression;
    {
        std::ifstream file(input_file);
        if (!file.is_open()) {
            std::cerr << "Error reading expression file: Could not open file: "
                      << input_file.string() << "\n";
            return 1;
        }

        std::string line;
        while (std::getline(file, line)) {
            line.erase(line.find_last_not_of(" \t\r\n") + 1);  // rtrim
            line.erase(0, line.find_first_not_of(" \t\r\n"));  // ltrim
            if (!line.empty() && line[0] != '#') {
                if (!original_expression.empty()) {
                    original_expression += " ";
                }
                original_expression += line;
            }
        }
    }

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
    std::ranges::sort(sorted_variable_names);

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

    // Convert the expression tree to BDD using selected method
    bdd_t f;
    std::unique_ptr<Cudd> cudd_mgr_ptr;
    BDD cudd_bdd;
    bool using_cudd = false;

    switch (conversion_method) {
        case ConversionMethod::Custom:
            std::cout << "Converting expression to BDD using custom recursive method...\n";
            f = convert_to_bdd(*expr, manager);
            break;
        case ConversionMethod::TeDDy:
            std::cout
                << "Converting expression to BDD using TeDDy's from_expression_tree method...\n";
            f = convert_to_bdd_with_teddy_adapter(*expr, manager);
            break;
        case ConversionMethod::CUDD:
            std::cout << "Converting expression to BDD using CUDD library...\n";
            std::tie(cudd_mgr_ptr, cudd_bdd) = convert_to_cudd_bdd(*expr, variable_names);
            using_cudd = true;
            std::cout << "CUDD BDD conversion completed successfully\n";
            std::cout << "CUDD BDD node count: " << cudd_bdd.nodeCount() << "\n";
            std::cout << "Note: CUDD BDDs are handled separately from TeDDy BDDs\n";
            break;
    }

    // Force variable reordering if requested (only for TeDDy)
    if (force_reorder_after_build && !using_cudd) {
        std::cout << "Forcing variable reordering after BDD construction...\n";
        manager.force_reorder();
        std::cout << "Variable reordering completed\n";

        // Apply reduce() method after reordering for additional optimization
        std::cout << "Applying reduce() method after reordering...\n";
        f = manager.reduce(f);  // Call reduce() with the diagram
        std::cout << "Reduce method completed successfully\n";
    } else if (force_reorder_after_build && using_cudd) {
        std::cout << "Variable reordering is not supported for CUDD in this implementation\n";
    }

    std::cout << "Function created successfully!\n";
    std::cout << "Using " << variable_names.size() << " variables\n\n";

    // Handle CUDD-specific output
    if (using_cudd) {
        std::cout << "\n=== CUDD BDD Analysis ===\n";
        std::cout << "CUDD BDD Statistics:\n";
        std::cout << "- Node count: " << cudd_bdd.nodeCount() << "\n";
        std::cout << "- Support size: " << cudd_bdd.SupportSize() << "\n";
        std::cout << "- Is constant: " << (cudd_bdd.IsZero() || cudd_bdd.IsOne() ? "Yes" : "No")
                  << "\n";

        if (cudd_bdd.IsZero() || cudd_bdd.IsOne()) {
            std::cout << "- Constant value: " << (cudd_bdd.IsOne() ? "True" : "False") << "\n";
        }

        std::cout << "\nCUDD conversion completed successfully.\n";
    }

    // Generate output filenames in the same directory as the input file
    std::filesystem::path expr_dot_filename = get_output_path(input_file, "_expression_tree.dot");
    std::filesystem::path bdd_dot_filename = get_output_path(input_file, "_bdd.dot");
    std::filesystem::path bdd_nodes_filename = get_output_path(input_file, "_bdd_nodes.txt");

    // Output expression tree as DOT file
    std::ofstream expr_dot_file(expr_dot_filename);
    if (expr_dot_file.is_open()) {
        // Generate complete DOT graph representation
        write_expression_to_dot(*expr, expr_dot_file, "ExpressionTree");

        expr_dot_file.close();
        std::cout << "Expression tree DOT representation saved to '" << expr_dot_filename << "'\n";
        std::cout << "You can visualize it using Graphviz with: dot -Tpng " << expr_dot_filename
                  << " -o " << get_output_path(input_file, "_expression_tree.png") << "\n\n";
    } else {
        std::cerr << "Error: Could not create output file '" << expr_dot_filename << "'\n";
    }

    // Output expression tree as Mermaid file if requested
    if (generate_mermaid) {
        std::string combined_mermaid_filename =
            get_output_path(input_file, "_analysis.md").string();
        std::ofstream combined_file(combined_mermaid_filename);
        if (combined_file.is_open()) {
            // Write comprehensive Markdown document with original expression and diagrams
            combined_file << "# BDD Analysis Report\n\n";
            combined_file << "## Original Expression\n\n";
            combined_file << "```\n" << original_expression << "\n```\n\n";

            combined_file << "## Expression Tree\n\n";
            combined_file
                << "The following diagram shows the parse tree of the logical expression:\n\n";
            combined_file << "```mermaid\n";
            write_expression_to_mermaid(*expr, combined_file);  // No title
            combined_file << "```\n\n";

            combined_file << "## Binary Decision Diagram (BDD)\n\n";
            combined_file << "The following diagram shows the optimized BDD representation:\n\n";
            combined_file << "```mermaid\n";
            if (using_cudd) {
                write_cudd_to_mermaid(*cudd_mgr_ptr, cudd_bdd, sorted_variable_names,
                                      combined_file);  // No title
            } else {
                write_teddy_to_mermaid(manager, f, sorted_variable_names,
                                       combined_file);  // No title
            }
            combined_file << "```\n\n";

            combined_file << "## Analysis Summary\n\n";
            combined_file << "- **Variables**: " << sorted_variable_names.size() << "\n";

            // Calculate BDD node count (from actual table size)
            size_t node_count;
            if (using_cudd) {
                auto cudd_nodes =
                    collect_cudd_nodes_topological(*cudd_mgr_ptr, cudd_bdd, sorted_variable_names);
                node_count = cudd_nodes.size();
            } else {
                auto nodes_in_order = collect_teddy_nodes_topological(f, sorted_variable_names);
                node_count = nodes_in_order.size();
            }
            combined_file << "- **BDD Nodes**: " << node_count << "\n";
            combined_file << "- **Expression**: " << original_expression << "\n\n";

            // Add BDD Node Table
            combined_file << "## BDD Node Table\n\n";
            combined_file << "The following table shows the internal structure of the BDD with "
                             "node relationships:\n\n";
            if (using_cudd) {
                write_cudd_nodes_to_markdown(*cudd_mgr_ptr, cudd_bdd, sorted_variable_names,
                                             combined_file);
            } else {
                write_teddy_nodes_to_markdown(manager, f, sorted_variable_names, combined_file);
            }
            combined_file << "\n";
            combined_file << "**Note**: Nodes are ordered topologically (parents before children) "
                             "with terminal nodes at the end.\n";

            combined_file.close();
            std::cout << "Combined BDD analysis saved to '" << combined_mermaid_filename << "'\n";
            std::cout << "You can view this Markdown file with embedded Mermaid diagrams on "
                         "GitHub/GitLab\n\n";
        } else {
            std::cerr << "Error: Could not create output file '" << combined_mermaid_filename
                      << "'\n";
        }
    }

    if (!quiet_mode) {
        // Print BDD node structure
        std::cout << "BDD Node Structure:\n";
        std::cout << "==================\n";
        if (using_cudd) {
            write_cudd_nodes_to_stream(*cudd_mgr_ptr, cudd_bdd, sorted_variable_names, std::cout,
                                       true);
        } else {
            write_teddy_nodes_to_stream(manager, f, sorted_variable_names, std::cout, true);
        }
        std::cout << "\n";

        // Output DOT representation to console
        std::cout << "DOT representation of the BDD:\n";
        std::cout << "==============================\n";
        if (using_cudd) {
            write_cudd_to_dot(*cudd_mgr_ptr, cudd_bdd, sorted_variable_names, std::cout, "DD");
        } else {
            write_teddy_to_dot(manager, f, sorted_variable_names, std::cout);
        }
        std::cout << "\n\n";
    }

    // Output DOT representation to file
    std::ofstream dot_file(bdd_dot_filename);
    if (dot_file.is_open()) {
        if (using_cudd) {
            write_cudd_to_dot(*cudd_mgr_ptr, cudd_bdd, sorted_variable_names, dot_file, "DD");
        } else {
            write_teddy_to_dot(manager, f, sorted_variable_names, dot_file);
        }
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
        if (using_cudd) {
            write_cudd_nodes_to_stream(*cudd_mgr_ptr, cudd_bdd, sorted_variable_names, nodes_file,
                                       false);
        } else {
            write_teddy_nodes_to_stream(manager, f, sorted_variable_names, nodes_file, false);
        }
        nodes_file.close();
        std::cout << "BDD node table saved to '" << bdd_nodes_filename << "'\n";
    } else {
        std::cerr << "Error: Could not create output file '" << bdd_nodes_filename << "'\n";
        return 1;
    }

    std::cout << "\nDemo completed successfully!\n";
    return 0;
}
