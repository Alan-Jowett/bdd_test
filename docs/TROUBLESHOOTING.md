# Troubleshooting

## Common Issues

### Build Errors

**C++20 not supported**
- **Problem**: Compiler doesn't support C++20 features
- **Solution**: Use a newer compiler (GCC 10+, Clang 12+, MSVC 19.28+)

**CMake version too old**
- **Problem**: CMake version is below 3.20
- **Solution**: Update CMake to version 3.20 or higher

**TeDDy dependency fetch failed**
- **Problem**: Cannot download TeDDy library from GitHub
- **Solution**: Check internet connection and Git access to GitHub
- **Alternative**: Verify firewall/proxy settings allow Git operations

### Test Failures

**Test Failure**: `Generated BDD nodes file differs from expected`
- **Problem**: Generated output doesn't match expected test files
- **Solution**: Regenerate expected output files by running the tool and copying the generated `*_bdd_nodes.txt` files to `test_expressions/default_ordering/`
- **Note**: Ensure expected files use real variable names, not x0, x1, x2

**CTest not finding tests**
- **Problem**: Tests not discovered or executed
- **Solution**: Ensure you're running `ctest` from the build directory
- **Check**: Verify CMakeLists.txt contains proper test definitions

### Runtime Issues

**Expression parsing failed**
- **Problem**: Input expression cannot be parsed
- **Solutions**:
  - Check operator spelling (AND, OR, NOT, XOR)
  - Verify parentheses are balanced
  - Ensure variables use valid characters (alphanumeric + underscore)
  - Check file encoding (should be UTF-8 or ASCII)

**File not found errors**
- **Problem**: Cannot read input expression file
- **Solutions**:
  - Use absolute file paths
  - Verify file exists and is readable
  - Check working directory
  - Ensure proper file permissions

### Visualization Issues

**Graphviz Not Found**: `'dot' is not recognized`
- **Problem**: Graphviz is not installed or not in PATH
- **Solutions**:
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
- **Verify**: Run `dot -V` to confirm installation

**PNG files not generated**
- **Problem**: DOT files exist but PNG files are missing
- **Solution**: Use the batch script `.\scripts\generate_all_visualizations.ps1`
- **Manual**: Run `dot -Tpng input.dot -o output.png`

### Performance Issues

**Large expressions cause slowdown**
- **Problem**: Complex boolean expressions take too long to process
- **Solutions**:
  - Simplify boolean logic first
  - Use parentheses to make operator precedence explicit
  - Consider breaking complex expressions into smaller parts
  - Check variable ordering (affects BDD size)

**Memory usage too high**
- **Problem**: BDD requires excessive memory
- **Solutions**:
  - Verify expression is minimized
  - Check for redundant subexpressions
  - Consider alternative variable ordering

## Debug Strategies

### Enabling Verbose Output
- Build in Debug mode for additional logging
- Use console output to trace expression parsing
- Check generated node tables for unexpected structure

### Validating Expressions
1. Test with simple expressions first
2. Gradually increase complexity
3. Verify each operator type works individually
4. Check variable name consistency across outputs

### File System Issues
- Use absolute paths to avoid working directory confusion
- Check file permissions (read/write access)
- Verify disk space for output files
- Test with files containing various character encodings

## Getting Help

### Information to Provide
When reporting issues, include:
- Operating system and version
- Compiler type and version
- CMake version
- Full error message or output
- Input expression that causes the problem
- Steps to reproduce

### Useful Commands for Diagnostics
```powershell
# Check compiler version
gcc --version          # or clang --version, cl
cmake --version
git --version

# Check Graphviz installation
dot -V

# Run tests with verbose output
cd build
ctest --verbose

# Check generated files
ls test_expressions/default_ordering/
```

## Known Limitations

### Expression Complexity
- Very large expressions (>50 variables) may require significant memory
- Deeply nested expressions can slow parsing
- Some operator combinations may not be optimal for BDD representation

### File Handling
- Unicode variable names may cause issues on some systems
- Very long file paths might be problematic on Windows
- Network drives may cause permission issues

### Platform-Specific Notes
- **Windows**: PowerShell recommended over Command Prompt
- **Linux**: Requires recent GCC for C++20 support
- **macOS**: May need Xcode command line tools for compilation
