// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

#include "expression_parser.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "expression_types.hpp"

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

    // Handle empty expression
    if (trimmed.empty()) {
        throw std::runtime_error("Empty expression encountered during parsing");
    }

    // Remove outer parentheses if they exist and are balanced
    if (trimmed.size() >= 2 && trimmed.front() == '(' && trimmed.back() == ')') {
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

    for (size_t i = trimmed.length(); i-- > 0;) {
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
    if (trimmed.length() >= 3 && trimmed.substr(0, 3) == "NOT") {
        std::string operand = trim(trimmed.substr(3));
        return std::make_unique<my_expression>(my_not{parse_expression(operand)});
    }

    // Must be a variable
    return std::make_unique<my_expression>(my_variable{trimmed});
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