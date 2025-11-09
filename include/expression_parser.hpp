// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

#pragma once

/**
 * @file expression_parser.hpp
 * @brief Expression Parser Interface - Public interface for logical expression parsing
 *
 * This header provides the public interface for parsing logical expressions from text files.
 * It contains functions to read and parse logical expressions with support for AND, OR, XOR,
 * and NOT operators, with proper operator precedence and parenthetical grouping.
 *
 * @author Alan Jowett
 * @date 2025
 * @copyright Copyright (c) 2025 Alan Jowett. Licensed under the MIT License - see LICENSE file for
 * details
 *
 * Features:
 * - Parse logical expressions from text files
 * - Support for AND, OR, XOR, NOT operators
 * - Proper operator precedence handling
 * - Parenthetical grouping support
 * - Comment filtering (lines starting with #)
 * - Variable name extraction and validation
 */

#include <string>

#include "expression_types.hpp"

/**
 * @brief Reads and parses a logical expression from a text file
 *
 * This function reads a logical expression from the specified file, filters out comment lines
 * (starting with #), and parses the expression into an abstract syntax tree representation.
 * The parser supports standard logical operators with proper precedence and associativity.
 *
 * Supported operators (in order of precedence, highest to lowest):
 * - NOT: Unary negation operator
 * - AND: Binary conjunction operator (left-associative)
 * - OR: Binary disjunction operator (left-associative)
 * - XOR: Binary exclusive-or operator (left-associative)
 *
 * @param filename Path to the text file containing the logical expression
 * @return my_expression_ptr Unique pointer to the parsed expression tree
 *
 * @throws std::runtime_error If the file cannot be opened or read
 * @throws std::runtime_error If the expression contains syntax errors
 * @throws std::runtime_error If the file is empty or contains only comments
 *
 * @note The function automatically trims whitespace from the expression
 * @note Empty lines and lines starting with # are treated as comments and ignored
 *
 * Example file format:
 * @code
 * # This is a comment
 * (x0 AND x1) OR (NOT x2) XOR (x3 AND (NOT x4))
 * @endcode
 */
my_expression_ptr read_expression_from_file(const std::string& filename);