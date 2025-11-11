// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

/**
 * @file expression_types.hpp
 * @brief Expression tree data structures for logical expressions
 *
 * Defines the variant-based abstract syntax tree (AST) structures
 * for representing logical expressions with AND, OR, XOR, NOT operators
 * and variable references.
 *
 * @author Alan Jowett
 * @date 2025
 * @copyright Copyright (c) 2025 Alan Jowett. Licensed under the MIT License
 */

#pragma once

#include <memory>
#include <string>
#include <variant>

/// @name Expression Tree Data Structures
/// @{

/**
 * @brief Forward declarations for expression tree node types
 *
 * These structures form a variant-based abstract syntax tree (AST)
 * for representing logical expressions with AND, OR, XOR, NOT operators
 * and variable references.
 */
struct my_and;       ///< Binary AND operation
struct my_or;        ///< Binary OR operation
struct my_not;       ///< Unary NOT operation
struct my_xor;       ///< Binary XOR operation
struct my_variable;  ///< Variable reference

/// @brief Variant type representing any expression node
using my_expression = std::variant<my_and, my_or, my_not, my_xor, my_variable>;

/// @brief Smart pointer to an expression for memory management
using my_expression_ptr = std::unique_ptr<my_expression>;

/**
 * @brief Represents a variable reference in the expression tree
 */
struct my_variable {
    std::string variable_name;  ///< Name of the variable (e.g., "x0", "input_A")

    // C++20 spaceship operator for automatic comparison generation
    auto operator<=>(const my_variable& other) const = default;
};

/**
 * @brief Represents a logical AND operation between two sub-expressions
 */
struct my_and {
    my_expression_ptr left;   ///< Left operand
    my_expression_ptr right;  ///< Right operand
};

/**
 * @brief Represents a logical OR operation between two sub-expressions
 */
struct my_or {
    my_expression_ptr left;   ///< Left operand
    my_expression_ptr right;  ///< Right operand
};

/**
 * @brief Represents a logical NOT operation on a single sub-expression
 */
struct my_not {
    my_expression_ptr expr;  ///< The expression to negate
};

/**
 * @brief Represents a logical XOR (exclusive OR) operation between two sub-expressions
 */
struct my_xor {
    my_expression_ptr left;   ///< Left operand
    my_expression_ptr right;  ///< Right operand
};

/// @}
