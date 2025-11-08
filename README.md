# BDD Expression Converter

[![CI/CD Pipeline](https://github.com/Alan-Jowett/bdd_test/actions/workflows/ci.yml/badge.svg)](https://github.com/Alan-Jowett/bdd_test/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

This project provides a comprehensive Binary Decision Diagram (BDD) conversion tool built with the TeDDy (Templated Decision Diagrams) library. It converts logical expressions from text files into BDDs and generates multiple output formats for analysis and visualization.

## Features

### Core Functionality
- **Expression Parsing**: Reads logical expressions from text files with support for AND, OR, NOT, XOR operators
- **Variable Mapping**: Automatically maps expression variables to BDD variables with **real name preservation**
- **BDD Generation**: Converts parsed expressions into optimized Binary Decision Diagrams
- **Cross-Platform**: Uses C++17 std::filesystem for reliable file handling

### Output Formats
- **Console Display**: Human-readable output with descriptive headers and **real variable names**
- **DOT Graph Files**: Both expression tree and BDD structure exports with **semantic variable labels**
- **PNG Generation**: Automated visualization creation with **meaningful variable names**
- **Node Tables**: Clean, structured table output displaying **actual variable names** for analysis

### File Organization
- **Flexible Input**: Supports expressions from any text file location
- **Derived Output**: Automatically generates output files with descriptive names based on input
- **Test Suite**: Organized test expressions in dedicated `test_expressions/` folder

### Testing Framework
- **CMake Integration**: Comprehensive test suite with CTest framework
- **Deterministic Validation**: Node table comparison for reliable automated testing
- **Cross-Platform Testing**: Consistent results across different operating systems
- **100% Test Coverage**: All sample expressions validated automatically

## Building

### Prerequisites

- CMake 3.20 or higher
- C++17 compatible compiler (GCC 9+, Clang 10+, or MSVC 19.14+)
- Git (for fetching dependencies)
- Optional: Graphviz (for PNG visualization generation)

### Build Instructions

1. Create a build directory:
   ```powershell
   mkdir build
   cd build
   ```

2. Configure the project:
   ```powershell
   cmake ..
   ```

3. Build the project:
   ```powershell
   cmake --build . --config Release
   ```

## Usage

### Basic Usage

Run the BDD converter with an expression file:
```powershell
.\build\bin\Release\bdd_demo.exe path/to/expression.txt
```

### Expression File Format

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

### Output Files Generated

For input file `example.txt`, the following files are created:
- `example_expression_tree.dot` - Visual representation of parsed expression
- `example_bdd.dot` - BDD structure in DOT format
- `example_bdd_nodes.txt` - Structured node table for analysis
- `example_expression_tree.png` - PNG visualization (if Graphviz available)
- `example_bdd.png` - PNG visualization (if Graphviz available)

### Test Suite

Run the comprehensive test suite:
```powershell
cd build
ctest -C Release
```

Tests validate:
- Expression parsing accuracy
- BDD generation consistency
- Cross-platform file handling
- Output format correctness

## Sample Output

### Console Display
```
TeDDy BDD Demo - Building BDD from Filter Expression File
========================================================

Reading filter expression from: "test_expressions\simple_expression.txt"
Read expression from file: (a AND b) OR (c AND d)

Function created successfully!
Using 4 variables

BDD Node Structure:
==================
BDD Node Table (Post-order + reverse - correct ordering):
Index | Variable | False Child | True Child | Type
------|----------|-------------|------------|----------
    0 |        a |           2 |          1 | Variable
    1 |        b |           2 |          4 | Variable
    2 |        c |           5 |          3 | Variable
    3 |        d |           5 |          4 | Variable
    4 |        - |           - |          - | Terminal(1)
    5 |        - |           - |          - | Terminal(0)

Total nodes: 6
Note: True post-order + reverse ordering.

BDD node table saved to "test_expressions\simple_expression_bdd_nodes.txt"
Demo completed successfully!
```

### Node Table File (Clean Format)
```
Index | Variable | False Child | True Child | Type
------|----------|-------------|------------|----------
    0 |        a |           2 |          1 | Variable
    1 |        b |           2 |          4 | Variable
    2 |        c |           5 |          3 | Variable
    3 |        d |           5 |          4 | Variable
    4 |        - |           - |          - | Terminal(1)
    5 |        - |           - |          - | Terminal(0)
```

## Visualization Examples

The tool generates both expression tree and BDD visualizations. Here are examples from our test expressions:

### Simple Expression: `(a AND b) OR (c AND d)`

**Expression Tree:**
![Simple Expression Tree](test_expressions/simple_expression_expression_tree.png)

**Resulting BDD:**
![Simple Expression BDD](test_expressions/simple_expression_bdd.png)

This basic expression shows how AND/OR operations are structured in the BDD with 4 variables (a, b, c, d) displayed with their actual names.

---

### Complex Expression: `((x0 AND x1) OR (NOT x2)) XOR ((x3 AND (NOT x4)) OR (x5 XOR x6))`

**Expression Tree:**
![Complex Expression Tree](test_expressions/filter_expression_expression_tree.png)

**Resulting BDD:**
![Complex Expression BDD](test_expressions/filter_expression_bdd.png)

This complex expression demonstrates all supported operators (AND, OR, NOT, XOR) with 7 variables (x0 through x6), showing how the BDD optimization reduces the structure while preserving logical equivalence.

---

### Negated Expression: `NOT (a OR b) AND c`

**Expression Tree:**
![Negated Expression Tree](test_expressions/my_test_expression_expression_tree.png)

**Resulting BDD:**
![Negated Expression BDD](test_expressions/my_test_expression_bdd.png)

This example shows how NOT operations and De Morgan's laws are handled in the BDD representation with 3 variables (a, b, c) displayed with their actual names.

---

### Understanding the Visualizations

**Expression Trees** (left diagrams):
- Show the parsed structure of your logical expression
- Reveal operator precedence and grouping
- Help debug complex expressions before BDD conversion

**Binary Decision Diagrams** (right diagrams):
- Optimized representation for efficient logical operations
- Solid lines represent "true" paths, dashed lines represent "false" paths
- Terminal nodes show final boolean values (0 = false, 1 = true)
- Shared nodes indicate common subexpressions

### Real Variable Names

**Key Feature**: All outputs preserve the actual variable names from your expressions instead of using generic placeholders.

**Examples**:
- Expression `temperature AND humidity` displays variables as `temperature`, `humidity`
- Expression `(a AND b) OR c` shows variables as `a`, `b`, `c`
- Expression `x XOR y XOR z` uses variables `x`, `y`, `z`

**Benefits**:
- **Intuitive Debugging**: No mental mapping from x0→temperature required
- **Professional Visualizations**: Publication-ready graphs with meaningful labels
- **Consistent Output**: Same variable names across node tables, DOT files, and PNG images
- **Better Documentation**: Generated diagrams directly match your domain terminology

## Visualizing the BDD

### Using Graphviz (Recommended)

1. Install Graphviz:
   ```powershell
   # Windows (using chocolatey)
   choco install graphviz

   # Windows (using winget)
   winget install Graphviz.Graphviz

   # Linux (Ubuntu/Debian)
   sudo apt install graphviz

   # macOS (using homebrew)
   brew install graphviz
   ```

2. Generate PNG visualizations:
   ```powershell
   # For expression tree
   dot -Tpng example_expression_tree.dot -o example_expression_tree.png

   # For BDD structure
   dot -Tpng example_bdd.dot -o example_bdd.png
   ```

### Batch PNG Generation

Use the provided PowerShell script to generate all PNG files:
```powershell
# Generate PNGs for all test expressions and any additional DOT files
.\generate_all_visualizations.ps1
```

This script automatically:
- Finds all DOT files in the project directory and subdirectories
- Generates corresponding PNG visualizations using Graphviz
- Provides detailed progress output and error handling
- Works with both expression tree and BDD DOT files

## Project Structure

```
bdd_test/
├── main.cpp                           # Main BDD converter application
├── CMakeLists.txt                     # Build configuration with testing
├── README.md                          # This file
├── LICENSE                            # MIT License file
├── generate_all_visualizations.ps1    # Batch PNG generation script
├── cmake/                             # CMake configuration files
│   └── run_bdd_test.cmake             # Test framework script
├── test_expressions/                  # Test expression files
│   ├── simple_expression.txt          # Basic AND/OR expression
│   ├── filter_expression.txt          # Complex filter expression
│   ├── my_test_expression.txt         # Expression with underscores in name
│   ├── test_subdir_expression.txt     # Subdirectory test
│   └── *_bdd_nodes.txt                # Expected output files for testing
└── build/                             # Build artifacts and executables
```

## Advanced Features

### Real Variable Name Preservation
- **Semantic Variable Names**: All outputs use actual variable names from your expressions
- **Multi-Format Consistency**: Node tables, DOT files, and PNG visualizations all display the same meaningful names
- **Professional Output**: No generic x0, x1, x2 placeholders - direct mapping to your domain terminology
- **Enhanced Debugging**: Immediate recognition of variables without mental translation

### Expression Tree Analysis
- View the parsed expression structure before BDD conversion
- Understand operator precedence and grouping
- Debug complex expressions with tree visualization

### BDD Optimization
- Automatic variable ordering for optimal BDD size
- Node sharing and reduction for memory efficiency
- Terminal node consolidation

### Cross-Platform Compatibility
- Uses std::filesystem for robust path handling
- Consistent behavior across Windows, Linux, and macOS
- Unicode filename support

## Documentation

### Auto-Generated API Documentation

This project includes comprehensive Doxygen documentation that is automatically generated and published:

- **📖 Online Documentation**: Available at [GitHub Pages](https://Alan-Jowett.github.io/bdd_test/) (auto-updated on pushes)
- **🔄 Auto-Update**: Documentation is regenerated whenever code or comments are modified
- **📱 Multiple Formats**: HTML documentation with search functionality and cross-references

### Documentation Features

- **Function Documentation**: Complete parameter and return value descriptions
- **Code Examples**: Usage patterns and implementation details
- **Cross-References**: Automatic linking between related functions and types
- **Expression Tree Visualization**: Detailed explanation of AST structure
- **BDD Algorithm Details**: In-depth coverage of conversion processes

### Local Documentation Generation

To generate documentation locally:

```bash
# Install Doxygen (if not already installed)
# Ubuntu/Debian: sudo apt-get install doxygen graphviz
# Windows: Download from doxygen.org
# macOS: brew install doxygen graphviz

# Generate documentation
doxygen Doxyfile

# Open documentation
# Windows: start docs/html/index.html
# macOS: open docs/html/index.html  
# Linux: xdg-open docs/html/index.html
```

### Documentation Workflow

The documentation is automatically built and deployed using GitHub Actions:

1. **Triggers**: Pushes to main branch, version tags, or documentation files
2. **Generation**: Doxygen processes all source files and generates HTML
3. **Deployment**: 
   - Published to GitHub Pages for public access
   - Committed to `documentation` branch for manual access
4. **Versioning**: Documentation version matches git tags or branch names

### Accessing Different Versions

- **Latest Stable**: Main branch documentation at GitHub Pages
- **Tagged Releases**: Available in the `documentation` branch history
- **Development**: Updated on every commit to main branch

## About TeDDy

TeDDy is a C++ library for creation and manipulation of decision diagrams developed at the University of Žilina. It supports:

- Binary Decision Diagrams (BDDs)
- Multi-Valued Decision Diagrams (MDDs)
- Integer Multi-Valued Decision Diagrams (iMDDs)
- Reliability analysis functions
- High-performance algorithms for large-scale problems

**Version Used**: v4.1.0
**Repository**: https://github.com/MichalMrena/DecisionDiagrams

## Contributing

To add new test expressions:
1. Create a new `.txt` file in `test_expressions/` folder with meaningful variable names
2. Add the corresponding expected output file `*_bdd_nodes.txt` with **real variable names** (not x0, x1, x2)
3. Update `CMakeLists.txt` to include the new test case
4. Run `ctest` to verify the test passes

**Note**: Expected output files should contain the actual variable names from your expression, not generic placeholders.

## Troubleshooting

### Common Issues

**Build Error**: `C++17 not supported`
- Solution: Use a newer compiler (GCC 9+, Clang 10+, MSVC 19.14+)

**Test Failure**: `Generated BDD nodes file differs from expected`
- Solution: Regenerate expected output files by running the tool and copying the generated `*_bdd_nodes.txt` files

**Graphviz Not Found**: `'dot' is not recognized`
- Solution: Install Graphviz and ensure it's in your system PATH

### Performance Tips

- For large expressions, consider simplifying boolean logic first
- Variable ordering can significantly impact BDD size
- Use parentheses to make operator precedence explicit

## License

This BDD Expression Converter is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

**Third-Party Dependencies:**
- **TeDDy Library**: Used under its own license terms - see the [TeDDy repository](https://github.com/MichalMrena/DecisionDiagrams) for details

**MIT License Summary:**
- ✅ Commercial use allowed
- ✅ Modification allowed
- ✅ Distribution allowed
- ✅ Private use allowed
- ❗ License and copyright notice required