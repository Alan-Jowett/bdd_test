# Advanced Features

## Real Variable Name Preservation
- **Semantic Variable Names**: All outputs use actual variable names from your expressions
- **Multi-Format Consistency**: Node tables, DOT files, and PNG visualizations all display the same meaningful names
- **Professional Output**: No generic x0, x1, x2 placeholders - direct mapping to your domain terminology
- **Enhanced Debugging**: Immediate recognition of variables without mental translation

## Expression Tree Analysis
- View the parsed expression structure before BDD conversion
- Understand operator precedence and grouping
- Debug complex expressions with tree visualization

## BDD Optimization
- Automatic variable ordering for optimal BDD size
- Node sharing and reduction for memory efficiency
- Terminal node consolidation

## Cross-Platform Compatibility
- Uses std::filesystem for robust path handling
- Consistent behavior across Windows, Linux, and macOS
- Unicode filename support

## Output File Organization

### Generated Files

For input file `example.txt`, the following files are created:
- `example_expression_tree.dot` - Visual representation of parsed expression
- `example_bdd.dot` - BDD structure in DOT format
- `example_bdd_nodes.txt` - Structured node table for analysis
- `example_expression_tree.png` - PNG visualization (if Graphviz available)
- `example_bdd.png` - PNG visualization (if Graphviz available)

### File Naming Convention

The tool automatically generates descriptive output file names:
- Base name derived from input file
- Descriptive suffixes for different output types
- Consistent naming across all generated files

## Expression File Format

### Supported Operators
- `AND` or `&` - Logical AND
- `OR` or `|` - Logical OR
- `NOT` or `!` - Logical NOT
- `XOR` or `^` - Logical XOR
- `()` - Parentheses for grouping

### Variable Naming
- Use descriptive variable names (temperature, humidity, pressure)
- Alphanumeric characters and underscores supported
- Case-sensitive variable recognition
- No reserved keywords (operators are recognized by syntax)

### Example Expressions
```
(a AND b) OR (c AND d)
x XOR y XOR z
NOT (p AND q) OR r
temperature > 25 AND humidity < 60
pressure_high OR temperature_low
```

## Performance Considerations

### BDD Size Optimization
- Variable ordering significantly impacts BDD size
- The tool automatically handles basic optimization
- Complex expressions may benefit from manual variable ordering

### Memory Usage
- BDD size grows with expression complexity
- Shared subexpressions reduce memory footprint
- Terminal node consolidation improves efficiency

### Processing Speed
- Linear time for most expressions
- Exponential worst-case for highly complex boolean functions
- Use parentheses to make precedence explicit for faster parsing
