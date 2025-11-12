# CUDD Reference Analysis Files

This directory contains reference analysis files generated using the CUDD BDD library, used for Mermaid regression testing and comparison with TeDDy outputs. Each file includes comprehensive BDD analysis with visual diagrams and structured data tables.

## 📊 Analysis File Structure

Each analysis file contains:
- **Original Expression**: The logical expression being analyzed
- **Expression Tree**: Mermaid flowchart showing the parse tree structure
- **Binary Decision Diagram (BDD)**: Optimized BDD representation with colored nodes (CUDD-generated)
- **BDD Summary**: Statistics about the CUDD-generated BDD
- **BDD Node Table**: Detailed node structure with CUDD variable mappings

## 🔄 CUDD vs TeDDy Comparison

These files enable comparison between CUDD and TeDDy BDD implementations:
- **Variable Ordering**: CUDD may use different variable ordering strategies
- **Node Optimization**: Different optimization techniques may result in varying node counts
- **Representation**: Both libraries should produce logically equivalent BDDs for the same expression
- **Visual Format**: Now uses identical Mermaid styling and CSS classes as TeDDy

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
- **[same_variable_analysis.md](same_variable_analysis.md)** - Expressions with repeated variable usage
- **[underscore_vars_analysis.md](underscore_vars_analysis.md)** - Variables with underscore naming

### Advanced Structures
- **[test_subdir_expression_analysis.md](test_subdir_expression_analysis.md)** - Subdirectory test expressions

## 🎯 Format Compatibility

As of the latest update, CUDD reference outputs now use the same visual format as TeDDy:
- ✅ Identical Mermaid diagram styling with proper CSS classes
- ✅ Consistent color schemes for BDD nodes and terminals
- ✅ Same graph title format ("BDD" instead of "CUDD BDD")
- ✅ Compatible node representation and edge styling

This ensures consistent documentation and testing across both BDD implementations while maintaining the mathematical correctness of each library's approach.

## 📋 Usage for Testing

These reference files serve multiple purposes:
- **Regression Testing**: Verify that CUDD output format remains consistent
- **Visual Comparison**: Compare BDD structures between CUDD and TeDDy
- **Documentation**: Provide examples of CUDD BDD analysis output
- **Format Validation**: Ensure Mermaid diagrams render correctly

Each file represents the canonical CUDD analysis output for its respective logical expression.