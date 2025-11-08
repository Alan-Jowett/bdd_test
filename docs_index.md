# BDD Expression Converter Documentation

Welcome to the comprehensive documentation for the BDD Expression Converter project.

## Quick Navigation

### 🚀 Getting Started
- [Building the Project](building.html) - Step-by-step build instructions
- [Usage Examples](examples.html) - How to use the tool effectively
- [Expression Format](expression-format.html) - Supported syntax and operators

### 📚 API Reference
- [Functions](functions.html) - Complete function documentation
- [Data Structures](annotated.html) - Expression tree and BDD types
- [File I/O](files.html) - Input and output file handling

### 🔧 Implementation Details
- [BDD Conversion](conversion.html) - How expressions become BDDs
- [Expression Parsing](parsing.html) - Parser implementation
- [Visualization](visualization.html) - DOT graph generation

## Key Features

This tool provides:
- **Expression Parsing**: Support for AND, OR, XOR, NOT operators with proper precedence
- **BDD Generation**: Efficient conversion to Binary Decision Diagrams using TeDDy library
- **Multiple Outputs**: Console display, DOT graphs, PNG images, and structured tables
- **Real Variable Names**: Preserves meaningful variable names throughout the process

## Expression Syntax

The tool supports logical expressions with these operators:
- `AND` - Logical conjunction
- `OR` - Logical disjunction  
- `XOR` - Exclusive or
- `NOT` - Logical negation
- `()` - Grouping parentheses

Example: `(input_A AND input_B) OR (NOT sensor_1) XOR (gate_C AND (NOT enable))`

## Output Formats

### 1. Console Display
Human-readable BDD structure with variable names and node relationships.

### 2. DOT Graph Files
- Expression tree visualization showing the abstract syntax tree
- BDD structure diagram with decision nodes and terminal values

### 3. Node Tables
Structured tables showing:
- Node indices and relationships
- Variable assignments
- Terminal node values
- Complete BDD structure

## Getting Help

- Check the [troubleshooting section](troubleshooting.html) for common issues
- Review [usage examples](examples.html) for typical workflows
- Examine [test expressions](test-expressions.html) for syntax examples

---

*This documentation is automatically generated from the source code using Doxygen.*