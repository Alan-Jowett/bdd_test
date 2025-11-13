// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file node_table_generator.hpp
 * @brief Generic template for generating node tables from BDD structures
 *
 * This header provides a generic template-based approach to generating node tables
 * from BDD structures in multiple output formats. It uses concepts to define the
 * interface requirements for iterators and supports various output formats including
 * plain text tables and Markdown.
 *
 * The iterator represents the current node and provides methods to access node
 * properties such as variable names, child relationships, and terminal values.
 *
 * @author Alan Jowett
 * @date 2025
 * @copyright Copyright (c) 2025 Alan Jowett. Licensed under the MIT License
 */

#pragma once

#include <algorithm>
#include <concepts>
#include <format>
#include <iostream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "dag_walker.hpp"
#include "graph_iterator_concepts.hpp"

namespace node_table {

/// @name C++20 Concepts for node table generation
/// @{

/**
 * @brief Core concept for node table iterators
 *
 * Defines the interface required for generating node tables.
 * This extends the base graph iterator interface with additional
 * requirements specific to table generation, including node properties
 * and child relationship information.
 */
template <typename T>
concept NodeTableIterator = BaseGraphIterator<T> && requires(T t) {
    // Additional requirements for table generation
    { t.is_valid() } -> std::convertible_to<bool>;
    { t.is_terminal() } -> std::convertible_to<bool>;
    { t.get_variable_name() } -> std::convertible_to<std::string>;
    { t.get_type() } -> std::convertible_to<std::string>;

    // Terminal value (for terminal nodes)
    { t.get_terminal_value() } -> std::convertible_to<int>;
};

/// @}

/// @name Configuration structures for different table formats
/// @{

/**
 * @brief Configuration for plain text table output
 */
struct TextTableConfig {
    bool include_headers = true;  ///< Include descriptive headers and footers
    std::string table_title = "Node Table (topological ordering)";  ///< Title for the table
    int index_width = 5;                                            ///< Column width for index
    int variable_width = 8;         ///< Column width for variable name
    int false_child_width = 11;     ///< Column width for false child indices
    int true_child_width = 10;      ///< Column width for true child indices
    std::string separator = " | ";  ///< Column separator
    std::string header_separator = "------|----------|-------------|------------|----------";

    /**
     * @brief Constructor with default values
     * @param headers Whether to include headers and footers
     * @param title Custom title for the table
     */
    explicit TextTableConfig(bool headers = true,
                             const std::string& title = "Node Table (topological ordering)")
        : include_headers(headers), table_title(title) {}
};

/**
 * @brief Configuration for Markdown table output
 */
struct MarkdownTableConfig {
    std::string alignment = "left";  ///< Table alignment (left, center, right)
    bool use_bold_headers = false;   ///< Make headers bold

    /**
     * @brief Constructor with default values
     * @param align Table alignment style
     */
    explicit MarkdownTableConfig(const std::string& align = "left") : alignment(align) {}
};

/// @}

/// @name Core table generation functions
/// @{

/**
 * @brief Generate a plain text table of nodes
 *
 * Creates a formatted plain text table showing node structure, relationships,
 * and properties. Uses the TextTableConfig for formatting options.
 *
 * @tparam Iterator Type that satisfies NodeTableIterator concept
 * @param root_iterator Iterator positioned at the root of the structure
 * @param out Output stream to write the table to
 * @param config Configuration options for text table formatting
 *
 * @requires NodeTableIterator<Iterator>
 */
template <NodeTableIterator Iterator>
void generate_text_table(const Iterator& root_iterator, std::ostream& out,
                         const TextTableConfig& config = TextTableConfig()) {
    // Collect all nodes using DAG walker
    auto nodes = dag_walker::collect_unique_nodes_topological(root_iterator);

    // Reverse to achieve parents-before-children ordering with terminals at end
    // (matching legacy BDD function behavior)
    std::reverse(nodes.begin(), nodes.end());

    // Create index mapping for child references
    std::unordered_map<const void*, int> node_to_index;
    for (size_t i = 0; i < nodes.size(); ++i) {
        node_to_index[nodes[i].get_node_address()] = static_cast<int>(i);
    }

    // Write headers
    if (config.include_headers) {
        out << config.table_title << ":\n";
    }
    out << "Index" << config.separator << "Variable" << config.separator << "False Child"
        << config.separator << "True Child" << config.separator << "Type\n";
    out << config.header_separator << "\n";

    // Write table rows
    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& iter = nodes[i];

        out << std::format("{:>{}}", static_cast<int>(i), config.index_width) << config.separator;
        out << std::format("{:>{}}", iter.get_variable_name(), config.variable_width)
            << config.separator;

        if (iter.is_terminal()) {
            out << std::format("{:>{}}", "-", config.false_child_width) << config.separator;
            out << std::format("{:>{}}", "-", config.true_child_width) << config.separator;
        } else {
            // Find child indices
            auto children = iter.get_children();
            int false_child_idx = -1;
            int true_child_idx = -1;

            if (children.size() >= 2) {
                auto false_addr = children[0].get_node_address();
                auto true_addr = children[1].get_node_address();

                auto false_it = node_to_index.find(false_addr);
                auto true_it = node_to_index.find(true_addr);

                if (false_it != node_to_index.end())
                    false_child_idx = false_it->second;
                if (true_it != node_to_index.end())
                    true_child_idx = true_it->second;
            }

            out << std::format("{:>{}}", false_child_idx, config.false_child_width)
                << config.separator;
            out << std::format("{:>{}}", true_child_idx, config.true_child_width)
                << config.separator;
        }
        out << iter.get_type() << "\n";
    }

    if (config.include_headers) {
        out << "\nTotal nodes: " << nodes.size() << "\n";
        out << "Note: Topological order ensures parents appear before children.\n";
    }
}

/**
 * @brief Generate a Markdown table of nodes
 *
 * Creates a Markdown-formatted table showing node structure and relationships.
 * Compatible with GitHub, GitLab, and other Markdown renderers.
 *
 * @tparam Iterator Type that satisfies NodeTableIterator concept
 * @param root_iterator Iterator positioned at the root of the structure
 * @param out Output stream to write the Markdown table to
 * @param config Configuration options for Markdown table formatting
 *
 * @requires NodeTableIterator<Iterator>
 */
template <NodeTableIterator Iterator>
void generate_markdown_table(const Iterator& root_iterator, std::ostream& out,
                             const MarkdownTableConfig& config = MarkdownTableConfig()) {
    // Collect all nodes using DAG walker
    auto nodes = dag_walker::collect_unique_nodes_topological(root_iterator);

    // Reverse to achieve parents-before-children ordering with terminals at end
    // (matching legacy BDD function behavior)
    std::reverse(nodes.begin(), nodes.end());

    // Create index mapping for child references
    std::unordered_map<const void*, int> node_to_index;
    for (size_t i = 0; i < nodes.size(); ++i) {
        node_to_index[nodes[i].get_node_address()] = static_cast<int>(i);
    }

    // Write Markdown table header
    const char* header_prefix = config.use_bold_headers ? "**" : "";
    const char* header_suffix = config.use_bold_headers ? "**" : "";

    out << "| " << header_prefix << "Index" << header_suffix << " | " << header_prefix << "Variable"
        << header_suffix << " | " << header_prefix << "False Child" << header_suffix << " | "
        << header_prefix << "True Child" << header_suffix << " | " << header_prefix << "Type"
        << header_suffix << " |\n";

    // Table alignment row
    if (config.alignment == "center") {
        out << "|:-----:|:--------:|:-----------:|:----------:|:----:|\n";
    } else if (config.alignment == "right") {
        out << "|------:|--------:|-------------|----------:|-----:|\n";
    } else {  // left alignment (default)
        out << "|-------|----------|-------------|------------|------|\n";
    }

    // Write table rows
    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& iter = nodes[i];

        out << "| " << static_cast<int>(i) << " | ";

        if (iter.is_terminal()) {
            out << "- | - | - | " << iter.get_type() << " |\n";
        } else {
            // Find child indices
            auto children = iter.get_children();
            int false_child_idx = -1;
            int true_child_idx = -1;

            if (children.size() >= 2) {
                auto false_addr = children[0].get_node_address();
                auto true_addr = children[1].get_node_address();

                auto false_it = node_to_index.find(false_addr);
                auto true_it = node_to_index.find(true_addr);

                if (false_it != node_to_index.end())
                    false_child_idx = false_it->second;
                if (true_it != node_to_index.end())
                    true_child_idx = true_it->second;
            }

            out << iter.get_variable_name() << " | ";
            out << false_child_idx << " | ";
            out << true_child_idx << " | ";
            out << iter.get_type() << " |\n";
        }
    }
}

/**
 * @brief Generate a CSV table of nodes
 *
 * Creates a comma-separated values table for importing into spreadsheet
 * applications or data analysis tools.
 *
 * @tparam Iterator Type that satisfies NodeTableIterator concept
 * @param root_iterator Iterator positioned at the root of the structure
 * @param out Output stream to write the CSV table to
 * @param include_headers Whether to include column headers
 *
 * @requires NodeTableIterator<Iterator>
 */
template <NodeTableIterator Iterator>
void generate_csv_table(const Iterator& root_iterator, std::ostream& out,
                        bool include_headers = true) {
    // Collect all nodes using DAG walker
    auto nodes = dag_walker::collect_unique_nodes_topological(root_iterator);

    // Create index mapping for child references
    std::unordered_map<const void*, int> node_to_index;
    for (size_t i = 0; i < nodes.size(); ++i) {
        node_to_index[nodes[i].get_node_address()] = static_cast<int>(i);
    }

    // Write CSV header
    if (include_headers) {
        out << "Index,Variable,False Child,True Child,Type\n";
    }

    // Write CSV rows
    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& iter = nodes[i];

        out << static_cast<int>(i) << ",";
        out << iter.get_variable_name() << ",";

        if (iter.is_terminal()) {
            out << ",,";
        } else {
            // Find child indices
            auto children = iter.get_children();
            int false_child_idx = -1;
            int true_child_idx = -1;

            if (children.size() >= 2) {
                auto false_addr = children[0].get_node_address();
                auto true_addr = children[1].get_node_address();

                auto false_it = node_to_index.find(false_addr);
                auto true_it = node_to_index.find(true_addr);

                if (false_it != node_to_index.end())
                    false_child_idx = false_it->second;
                if (true_it != node_to_index.end())
                    true_child_idx = true_it->second;
            }

            out << false_child_idx << ",";
            out << true_child_idx << ",";
        }
        out << iter.get_type() << "\n";
    }
}

/// @}

/// @name Utility functions for different output formats
/// @{

/**
 * @brief Generate table in the specified format
 *
 * Unified interface for generating tables in different formats based on
 * a format string parameter.
 *
 * @tparam Iterator Type that satisfies NodeTableIterator concept
 * @param root_iterator Iterator positioned at the root of the structure
 * @param out Output stream to write the table to
 * @param format Output format ("text", "markdown", "csv")
 * @param include_headers Whether to include headers (applies to text and CSV)
 *
 * @throws std::invalid_argument If format is not recognized
 * @requires NodeTableIterator<Iterator>
 */
template <NodeTableIterator Iterator>
void generate_table(const Iterator& root_iterator, std::ostream& out,
                    const std::string& format = "text", bool include_headers = true) {
    if (format == "text") {
        generate_text_table(root_iterator, out, TextTableConfig(include_headers));
    } else if (format == "markdown") {
        generate_markdown_table(root_iterator, out, MarkdownTableConfig());
    } else if (format == "csv") {
        generate_csv_table(root_iterator, out, include_headers);
    } else {
        throw std::invalid_argument("Unsupported table format: " + format);
    }
}

/// @}

}  // namespace node_table
