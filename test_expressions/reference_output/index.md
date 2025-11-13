# BDD Reference Analysis Files

This directory contains reference analysis files generated for BDD analysis, used for regression testing and documentation. Each file includes comprehensive BDD analysis with visual diagrams and structured data tables.

## 📊 Analysis File Structure

Each analysis file contains:
- **Original Expression**: The logical expression being analyzed
- **Expression Tree**: Mermaid flowchart showing the parse tree structure
- **Binary Decision Diagram (BDD)**: Optimized BDD representation with colored nodes
- **BDD Summary**: Statistics about the generated BDD
- **BDD Node Table**: Detailed node structure with variable mappings

## 🔄 Cross-Library Validation

These files enable validation across different BDD implementations:
- **Variable Ordering**: Different libraries may use different variable ordering strategies
- **Node Optimization**: Various optimization techniques may result in different node counts
- **Representation**: All libraries should produce logically equivalent BDDs for the same expression
- **Python Integration**: Cross-validation with Python's `dd` library for canonical BDD comparison

## 📁 Available Reference Files

### Basic Expressions
- **[simple_expression_analysis.md](simple_expression_analysis.md)** - `(a AND b) OR (c AND d)`
- **[my_test_expression_analysis.md](my_test_expression_analysis.md)** - `NOT (a OR b) AND c`
- **[single_variable_analysis.md](single_variable_analysis.md)** - Single variable expression
- **[single_variable_unusual_name_analysis.md](single_variable_unusual_name_analysis.md)** - Variable with special characters

### Complex Logical Operations
- **[all_operators_analysis.md](all_operators_analysis.md)** - Expression using AND, OR, NOT, XOR
- **[filter_expression_analysis.md](filter_expression_analysis.md)** - Complex filtering expression
- **[deeply_nested_analysis.md](deeply_nested_analysis.md)** - Multiple levels of nesting
- **[many_variables_analysis.md](many_variables_analysis.md)** - Expression with numerous variables
- **[xor_chain_analysis.md](xor_chain_analysis.md)** - Chain of XOR operations

### Special Cases and Edge Cases
- **[multiple_not_analysis.md](multiple_not_analysis.md)** - Multiple NOT operations
- **[same_variable_analysis.md](same_variable_analysis.md)** - Expression with repeated variables
- **[test_subdir_expression_analysis.md](test_subdir_expression_analysis.md)** - Test subdirectory expression
- **[underscore_vars_analysis.md](underscore_vars_analysis.md)** - Variables with underscores

### Edge Case Regressions
- **[boundary_detection_regression_analysis.md](boundary_detection_regression_analysis.md)** - Boundary detection edge cases
- **[complex_boundary_regression_analysis.md](complex_boundary_regression_analysis.md)** - Complex boundary conditions
- **[operator_keywords_in_vars_analysis.md](operator_keywords_in_vars_analysis.md)** - Variables containing operator keywords
- **[unicode_chars_regression_analysis.md](unicode_chars_regression_analysis.md)** - Unicode character handling

## 📖 Usage

These reference files are used by:
1. **Regression Testing**: Ensuring consistent BDD output format across builds
2. **Documentation**: Examples of BDD analysis output format and structure
3. **Cross-Library Validation**: Comparing outputs between different BDD implementations (TeDDy, Python's dd library)
4. **Debugging**: Reference for expected output when troubleshooting issues
5. **Mermaid Visualization**: Templates for generating consistent diagram formats

## ⚠️ Note

Edge case files may have different behavior across BDD libraries due to varying parser strictness and optimization strategies. The unified reference structure helps track these differences systematically.