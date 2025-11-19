# NOTE: This project is being retired in favor of [DagIR](https://github.com/Alan-Jowett/dagir) - Traverse external DAGs without copying and render them anywhere: DagIR builds a lightweight IR for DOT, Mermaid, or JSON — header-only, C++20, cross-platform.

This repo is being made read-only and kept for historical sake.

# BDD Expression Converter

[![CI/CD Pipeline](https://github.com/Alan-Jowett/bdd_test/actions/workflows/ci.yml/badge.svg)](https://github.com/Alan-Jowett/bdd_test/actions/workflows/ci.yml)
[![Code Coverage](https://github.com/Alan-Jowett/bdd_test/actions/workflows/coverage.yml/badge.svg)](https://github.com/Alan-Jowett/bdd_test/actions/workflows/coverage.yml)
[![Coverage Status](https://coveralls.io/repos/github/Alan-Jowett/bdd_test/badge.svg?branch=main)](https://coveralls.io/github/Alan-Jowett/bdd_test?branch=main)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

This project provides a comprehensive Binary Decision Diagram (BDD) conversion tool built with the TeDDy (Templated Decision Diagrams) library. It converts logical expressions from text files into BDDs and generates multiple output formats for analysis and visualization.

## 📚 Documentation

For detailed documentation, see the [docs/](docs/) directory:
- **[Lessons Learned](docs/LESSONS_LEARNED.md)** - Comprehensive analysis of TeDDy BDD PoC insights and production recommendations
- **[Building Guide](docs/BUILDING.md)** - Detailed build instructions and prerequisites
- **[Unit Testing Guide](docs/UNIT_TESTING.md)** - Catch2-based unit testing system setup and usage
- **[Visualization Examples](docs/VISUALIZATIONS.md)** - BDD visualization examples and Graphviz setup
- **[Advanced Features](docs/ADVANCED.md)** - Performance optimization and advanced usage
- **[Troubleshooting](docs/TROUBLESHOOTING.md)** - Common issues and solutions
- **[Contributing](docs/CONTRIBUTING.md)** - Development guidelines and code style
- **[API Documentation](docs/DOCUMENTATION.md)** - Online docs and Doxygen setup
- **[Reference Analysis Files](test_expressions/reference_output/index.md)** - BDD analysis examples with visual diagrams
- **[Complete Documentation Index](docs/README.md)** - Full documentation overview

## Features

### Core Functionality
- **Expression Parsing**: Reads logical expressions from text files with support for AND, OR, NOT, XOR operators
- **Variable Mapping**: Automatically maps expression variables to BDD variables with **real name preservation**
- **BDD Generation**: Converts parsed expressions into optimized Binary Decision Diagrams
- **Cross-Platform**: Uses C++20 std::filesystem for reliable file handling

### Output Formats
- **Console Display**: Human-readable output with descriptive headers and **real variable names**
- **DOT Graph Files**: Both expression tree and BDD structure exports with **semantic variable labels**
- **PNG Generation**: Automated visualization creation with **meaningful variable names**
- **Node Tables**: Clean, structured table output displaying **actual variable names** for analysis

## Building

### Prerequisites

- CMake 3.20 or higher
- C++20 compatible compiler (GCC 10+, Clang 12+, or MSVC 19.28+)
- Git (for fetching dependencies)
- Optional: Graphviz (for PNG visualization generation)

### Build Instructions

```powershell
# Build the project
cmake -B build -S .
cmake --build build --config Release

# Run with an expression file
.\build\bin\Release\bdd_demo.exe test_expressions\simple_expression.txt

# Run tests
cd build && ctest -C Release
```

For detailed build instructions, see [docs/BUILDING.md](docs/BUILDING.md).

## Usage

Run the BDD converter with an expression file:
```powershell
.\build\bin\Release\bdd_demo.exe path/to/expression.txt
```

### Expression Format

Create a text file containing a logical expression using these operators:
- `AND` or `&` - Logical AND
- `OR` or `|` - Logical OR
- `NOT` or `!` - Logical NOT
- `XOR` or `^` - Logical XOR
- `()` - Parentheses for grouping

**Example expressions:**
```
(a AND b) OR (c AND d)
x XOR y XOR z
NOT (p AND q) OR r
temperature > 25 AND humidity < 60
```

### Output Files

For input file `example.txt`, the tool generates:
- `example_expression_tree.dot` - Visual representation of parsed expression
- `example_bdd.dot` - BDD structure in DOT format
- `example_bdd_nodes.txt` - Structured node table for analysis
- `example_expression_tree.png` - PNG visualization (if Graphviz available)
- `example_bdd.png` - PNG visualization (if Graphviz available)

### Running Tests

```powershell
cd build
ctest -C Release
```

### Advanced Testing

For comprehensive testing including memory safety checks:

**Sanitizer builds** (Linux/WSL only):
```bash
# Run all sanitizer combinations
./scripts/run-sanitizers.sh

# Or on Windows with WSL:
.\scripts\run-sanitizers.ps1
```

For visualization examples and advanced usage, see [docs/VISUALIZATIONS.md](docs/VISUALIZATIONS.md).

## Sample Output

```
TeDDy BDD Demo - Building BDD from Filter Expression File
========================================================

Reading filter expression from: "test_expressions\simple_expression.txt"
Read expression from file: (a AND b) OR (c AND d)

Function created successfully!
Using 4 variables

BDD Node Structure:
==================
Index | Variable | False Child | True Child | Type
------|----------|-------------|------------|----------
    0 |        a |           2 |          1 | Variable
    1 |        b |           2 |          4 | Variable
    2 |        c |           5 |          3 | Variable
    3 |        d |           5 |          4 | Variable
    4 |        - |           - |          - | Terminal(1)
    5 |        - |           - |          - | Terminal(0)

Total nodes: 6
BDD node table saved to "test_expressions\expected_output\simple_expression_bdd_nodes.txt"
Demo completed successfully!
```

For more visualization examples, see [docs/VISUALIZATIONS.md](docs/VISUALIZATIONS.md).

## Python BDD Comparison Tool

This project includes a **Python-based equivalent of bdd_demo.exe** for cross-validation of BDD implementations. The Python implementation uses the trusted `dd` (Decision Diagrams) library to produce canonical ROBDDs for comparison and verification.

### Quick Start
```bash
# Setup Python BDD tools
cd python_bdd
python setup_python_bdd.py

# Compare implementations
python bdd_demo_python.py ../test_expressions/simple_expression.txt --verbose
./build/bin/Release/bdd_demo.exe test_expressions/simple_expression.txt --method=teddy --verbose
./build/bin/Release/bdd_demo.exe test_expressions/simple_expression.txt --method=cudd --verbose
```

### CMake Integration
```bash
# Enable Python BDD testing
cmake -DENABLE_PYTHON_BDD=ON -B build -S .
ctest -C Release -L python
```

### Key Features
- **✅ Identical Output Formats** - Node tables and DOT graphs match C++ exactly
- **✅ Canonical Reference** - Uses academically trusted `dd` library
- **✅ CUDD Issue Detection** - Reveals non-canonical BDD problems
- **✅ Cross-Platform** - Pure Python implementation

**Example Canonicality Issue Detection:**
```
Expression: (a AND b) OR (c AND d)
TeDDy:   6 nodes (canonical) ✅
CUDD:    5 nodes (non-canonical) ❌
Python:  6 nodes (canonical reference) ✅
```

For complete documentation, see **[python_bdd/README.md](python_bdd/README.md)**.

## Project Structure

```
bdd_test/
├── main.cpp                           # Main BDD converter application
├── CMakeLists.txt                     # Build configuration with testing
├── README.md                          # This file
├── docs/                              # Detailed documentation
│   ├── BUILDING.md                    # Build instructions
│   ├── VISUALIZATIONS.md              # Visualization examples
│   ├── ADVANCED.md                    # Advanced features
│   ├── TROUBLESHOOTING.md             # Common issues and solutions
│   └── CONTRIBUTING.md                # Development guidelines
├── python_bdd/                       # Python BDD comparison tools
│   ├── README.md                      # Python BDD overview
│   ├── bdd_demo_python.py            # Python BDD demo script
│   ├── requirements.txt              # Python dependencies
│   └── setup_python_bdd.py          # Setup and verification
├── test_expressions/                  # Test expression files and outputs
│   ├── expected_output/               # Reference output files for validation
│   ├── edge_cases/                    # Edge case test files
│   ├── reference_output/              # BDD analysis examples (see index.md)
│   ├── reordered/                     # Variable reordering test results
│   └── default_ordering/             # Default ordering test results
└── build/                             # Build artifacts and executables
```

## About TeDDy

TeDDy is a C++ library for creation and manipulation of decision diagrams developed at the University of Žilina. This project uses TeDDy v4.1.0 to provide high-performance BDD operations with support for various decision diagram types.

**Version Used**: v4.1.0
**Repository**: https://github.com/MichalMrena/DecisionDiagrams
**Dependency Management**: See [documentation/dependency-management.md](documentation/dependency-management.md) for information on keeping dependencies up to date
For more information about TeDDy, see [docs/TEDDY.md](docs/TEDDY.md).

## Contributing

We welcome contributions! For quick setup:

```powershell
# One-command development environment setup
.\scripts\setup-dev-env.ps1

# Quick verification
.\scripts\check-format.ps1 -WhatIf
```

See [docs/CONTRIBUTING.md](docs/CONTRIBUTING.md) for complete details on:
- Development environment setup
- Pre-commit hooks and code formatting
- Adding new test expressions
- Code style guidelines
- Testing requirements

**✅ Pro tip:** The setup script installs pre-commit hooks that automatically check your code before commits!

## Troubleshooting

For common issues and solutions, see [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md).

**Quick fixes:**
- **Build Error**: Use C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 19.28+)
- **Test Failure**: Regenerate expected output files with real variable names
- **Missing Graphviz**: Install Graphviz and ensure it's in system PATH

## License

This BDD Expression Converter is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

**Third-Party Dependencies:**
- **TeDDy Library**: Used under its own license terms - see the [TeDDy repository](https://github.com/MichalMrena/DecisionDiagrams) for details
