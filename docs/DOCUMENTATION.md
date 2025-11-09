# API Documentation

## Auto-Generated Documentation

This project includes comprehensive Doxygen documentation that is automatically generated and published:

- **📖 Online Documentation**: Available at [GitHub Pages](https://Alan-Jowett.github.io/bdd_test/) (auto-updated on pushes)
- **🔄 Auto-Update**: Documentation is regenerated whenever code or comments are modified
- **📱 Multiple Formats**: HTML documentation with search functionality and cross-references

## Documentation Features

- **Function Documentation**: Complete parameter and return value descriptions
- **Code Examples**: Usage patterns and implementation details
- **Cross-References**: Automatic linking between related functions and types
- **Expression Tree Visualization**: Detailed explanation of AST structure
- **BDD Algorithm Details**: In-depth coverage of conversion processes

## Local Documentation Generation

To generate documentation locally, you have several options:

### Option 1: Use the Provided Scripts (Recommended)

The project includes convenient scripts that mirror the GitHub Actions workflow:

**PowerShell (Windows):**
```powershell
# Generate documentation
.\scripts\generate_docs.ps1

# Generate and automatically open
.\scripts\generate_docs.ps1 -Open
```

**Bash (Linux/macOS):**
```bash
# Generate documentation
./scripts/generate_docs.sh

# Make sure script is executable first
chmod +x scripts/generate_docs.sh
./scripts/generate_docs.sh
```

These scripts will:
- ✅ Check for Doxygen installation and provide install instructions if missing
- ✅ Handle version detection from git tags/branches
- ✅ Generate documentation with proper configuration
- ✅ Create GitHub Pages compatible output (with .nojekyll file)
- ✅ Provide helpful status messages and error reporting

### Option 2: Manual Generation

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

## Documentation Workflow

The documentation is automatically built and deployed using GitHub Actions:

1. **Triggers**: Pushes to main branch, version tags, or documentation files
2. **Generation**: Doxygen processes all source files and generates HTML
3. **Deployment**:
   - Published to GitHub Pages for public access
   - Committed to `documentation` branch for manual access
4. **Versioning**: Documentation version matches git tags or branch names

## Accessing Different Versions

- **Latest Stable**: Main branch documentation at GitHub Pages
- **Tagged Releases**: Available in the `documentation` branch history
- **Development**: Updated on every commit to main branch

## Doxygen Configuration

The project uses a carefully tuned `Doxyfile` configuration:

- **Input Sources**: All C++ source and header files
- **Output Format**: HTML with navigation and search
- **Graphs**: Call graphs and inheritance diagrams using Graphviz
- **Code Examples**: Extracted from source comments
- **Cross-Platform**: Works consistently across development environments

## Contributing to Documentation

When adding new functions or classes:

1. Use Doxygen-style comments (`///` or `/** */`)
2. Document all parameters with `@param`
3. Document return values with `@return`
4. Add usage examples with `@code` blocks
5. Include `@see` references to related functions

Example:
```cpp
/**
 * @brief Converts a logical expression to a BDD
 * @param expression The logical expression string
 * @param variables List of variable names to use
 * @return BDD structure representing the expression
 * @see generateDotFile(), saveNodeTable()
 *
 * @code
 * auto bdd = convertToBDD("a AND b", {"a", "b"});
 * @endcode
 */
```
