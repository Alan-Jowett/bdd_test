# Reference Analysis Files

This directory contains reference analysis files used for Mermaid regression testing. Each file includes comprehensive BDD analysis with visual diagrams and structured data tables.

## 📊 Analysis File Structure

Each analysis file contains:
- **Original Expression**: The logical expression being analyzed
- **Expression Tree**: Mermaid flowchart showing the parse tree structure
- **Binary Decision Diagram (BDD)**: Optimized BDD representation with colored nodes
- **BDD Summary**: Statistics about the generated BDD
- **BDD Node Table**: Detailed node structure with variable mappings

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
- **[same_variable_analysis.md](same_variable_analysis.md)** - Repeated variable usage
- **[underscore_vars_analysis.md](underscore_vars_analysis.md)** - Variables with underscores
- **[test_subdir_expression_analysis.md](test_subdir_expression_analysis.md)** - Subdirectory test case

### Regression Test Cases
- **[boundary_detection_regression_analysis.md](boundary_detection_regression_analysis.md)** - Boundary detection edge cases
- **[complex_boundary_regression_analysis.md](complex_boundary_regression_analysis.md)** - Complex boundary scenarios
- **[operator_keywords_in_vars_analysis.md](operator_keywords_in_vars_analysis.md)** - Variables containing operator keywords
- **[unicode_chars_regression_analysis.md](unicode_chars_regression_analysis.md)** - Unicode character handling

## 🎨 Visualization Features

The analysis files include **color-coded Mermaid diagrams** that match the PNG visualization outputs and are **compatible with both light and dark GitHub themes**:
- **Variables**: Light blue circles with black text
- **AND Operations**: Light green rectangles with black text
- **OR Operations**: Light coral rectangles with black text
- **NOT Operations**: Yellow rectangles with black text
- **XOR Operations**: Light pink rectangles with black text
- **BDD Variables**: Light blue rectangles with black text
- **Terminal Nodes**: Light gray rectangles with black text

All diagrams use explicit `color:#000` styling to ensure readability in GitHub's dark mode.

## 🧪 Usage in Testing

These reference files are used by the automated regression test suite:
- Tests verify that generated analysis files match these references exactly
- Ensures consistency of BDD generation and formatting
- Validates that Mermaid diagram syntax and styling remain correct
- Checks that BDD node tables maintain proper structure

## 📈 Adding New Reference Files

To add a new reference analysis file:

1. Create your logical expression in `test_expressions/your_expression.txt`
2. Generate the analysis: `.\build\bin\Release\bdd_demo.exe --mermaid test_expressions\your_expression.txt`
3. Copy the generated analysis file to this directory
4. The automated test discovery will detect it and create a regression test
5. Update this index to include the new reference file

## 🔗 Related Documentation

- **[Main README](../../README.md)** - Project overview and build instructions
- **[Visualization Guide](../../docs/VISUALIZATIONS.md)** - BDD visualization examples and setup
- **[Test Expressions](../README.md)** - Input expression files and test cases
- **[Contributing Guide](../../docs/CONTRIBUTING.md)** - Development guidelines

---

*This directory is part of the BDD Expression Converter project's comprehensive testing infrastructure.*