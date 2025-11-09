# Copilot Agent Directive for BDD Expression Converter

This project is a C++ Binary Decision Diagram (BDD) conversion tool that processes logical expressions and generates various output formats using the TeDDy (Templated Decision Diagrams) library.

## Project Overview

### Technology Stack
- **Language**: C++20
- **Build System**: CMake (minimum version 3.20)
- **Dependencies**: TeDDy library (fetched via FetchContent)
- **Testing**: CTest framework
- **Code Formatting**: clang-format (Google style with customizations)

### Project Structure
- `main.cpp` - Main application entry point
- `CMakeLists.txt` - Build configuration
- `test_expressions/` - Test input files for logical expressions
- `cmake/run_bdd_test.cmake` - Test execution helper
- `.clang-format` - Code formatting rules

## Build Instructions

### Prerequisites
- CMake 3.20 or higher
- C++20 compatible compiler (GCC, Clang, or MSVC)
- Git (for dependency fetching)

### Build Commands
```powershell
# Configure build (from project root)
cmake -B build -S .

# Build the project
cmake --build build --config Release

# Alternative: Build with coverage (for testing)
cmake -B build -S . -DENABLE_COVERAGE=ON
cmake --build build --config Release
```

### Build Outputs
- Executable: `build/bin/bdd_demo.exe` (Windows) or `build/bin/bdd_demo` (Unix)
- Test outputs: `build/test_output/`

## Testing

### Running Tests
```powershell
# Run all tests
cmake --build build --target test

# Alternative using ctest directly
cd build
ctest

# Run with verbose output
ctest --verbose

# Run specific test
ctest -R test_simple_expression
```

### Test Structure
- Tests are defined in `CMakeLists.txt` using the `add_bdd_test()` function
- Each test processes an expression file and generates:
  - Console output with variable mappings
  - DOT graph files (expression tree and BDD structure)
  - PNG visualizations (if Graphviz is available)
  - Node analysis tables

### Available Tests
- `test_filter_expression` - Complex filtering expressions
- `test_simple_expression` - Basic logical operations
- `test_my_test_expression` - Custom test expressions
- `test_subdir_expression` - Subdirectory test cases

## Code Formatting

### clang-format Configuration
The project uses a customized Google style with these key settings:
- **Indent**: 4 spaces, no tabs
- **Line Length**: 100 characters
- **Brace Style**: Attach (K&R style)
- **Include Sorting**: Case sensitive, regrouped
- **Trailing Whitespace**: Must be removed from all files

### Formatting Commands
```powershell
# Format all source files
clang-format -i *.cpp *.h

# Check formatting (dry run)
clang-format --dry-run --Werror *.cpp *.h

# Format specific file
clang-format -i main.cpp

# Remove trailing whitespace (PowerShell)
Get-ChildItem -Include "*.cpp","*.h","*.cmake","*.md" -Recurse | ForEach-Object { (Get-Content $_.FullName) | ForEach-Object { $_.TrimEnd() } | Set-Content $_.FullName }

# Check for trailing whitespace
Get-ChildItem -Include "*.cpp","*.h","*.cmake","*.md" -Recurse | Select-String "\s+$" | Select-Object Filename,LineNumber,Line
```

### Auto-formatting in VS Code
Ensure these settings are enabled:
```json
{
    "editor.formatOnSave": true,
    "C_Cpp.clang_format_path": "clang-format",
    "C_Cpp.clang_format_style": "file"
}
```

## Development Guidelines

### Code Style
- Follow the established clang-format configuration
- Use C++20 features appropriately
- Maintain clear variable naming with descriptive names
- Add comments for complex BDD operations
- Remove all trailing whitespace from files

### File Organization
- Input expressions go in `test_expressions/`
- Generated outputs appear in `build/test_output/`
- Keep source files at project root level
- Use descriptive filenames for test expressions

### Testing Requirements
- Add new test cases for any expression parsing changes
- Ensure all tests pass before committing
- Verify BDD generation works for edge cases
- Test with various logical operators (AND, OR, NOT, XOR)

### Dependencies
- TeDDy library is automatically fetched via CMake FetchContent
- No manual dependency installation required
- CMake handles all library linking

## Coverage Analysis

### Generating Coverage Reports
```powershell
# Build with coverage enabled
cmake -B build -S . -DENABLE_COVERAGE=ON
cmake --build build --config Release

# Run tests to generate coverage data
ctest

# Generate coverage report (requires gcov/lcov)
./scripts/generate_coverage.ps1
```

## GitHub Actions & Documentation

### Documentation Workflow
The project uses GitHub Actions to automatically generate and deploy Doxygen documentation:
- **Triggered by**: pushes to main/master branches, version tags, or manual dispatch
- **Generates**: Doxygen HTML documentation from source code comments
- **Deploys to**: GitHub Pages and a `documentation` branch

### Workflow Permissions
The documentation workflow requires `contents: write` permission to push to the documentation branch:
```yaml
permissions:
  contents: write  # Required for pushing to documentation branch
  pages: write     # Required for GitHub Pages deployment
  id-token: write  # Required for GitHub Pages authentication
```

### Troubleshooting Workflow Issues
- **Permission denied errors**: Ensure the workflow has `contents: write` permission
- **Documentation generation failures**: Check Doxyfile syntax and source code comments
- **Deployment failures**: Verify the `peaceiris/actions-gh-pages` action is up to date (v4+)

## Common Operations

### Adding New Test Cases
1. Create expression file in `test_expressions/`
2. Add test using `add_bdd_test()` in `CMakeLists.txt`
3. Run the new test to verify functionality

### Debugging BDD Generation
- Check console output for variable mappings
- Examine DOT files for graph structure
- Use PNG visualizations for complex expressions
- Review node tables for optimization analysis

### Performance Considerations
- BDD size grows with expression complexity
- Variable ordering affects BDD efficiency
- Test with large expressions to verify performance

## When Making Changes

### ⚠️ CRITICAL: Code Formatting is MANDATORY
**ALL code changes MUST be formatted with clang-format before committing.**
The CI/CD pipeline will FAIL if formatting issues are detected.

**Always run these commands before any commit:**
```bash
# Format all C++ files (REQUIRED)
find . -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i

# Verify formatting compliance (REQUIRED)
clang-format --dry-run --Werror *.cpp *.h
```

### Before Committing
1. **🔥 MANDATORY: Format all code**: Run `find . -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i` on ALL files
2. **🔥 MANDATORY: Verify formatting**: Run `clang-format --dry-run --Werror *.cpp *.h` to confirm no issues
3. **Remove trailing whitespace**: Ensure no lines end with spaces or tabs
4. **Build successfully**: Ensure `cmake --build build` completes without errors
5. **Pass all tests**: Run `ctest` and verify all tests pass
6. **Include signed-off-by**: Always use `git commit -s -m "your message"` to include the "Signed-off-by" line in commit messages. The `-s` flag adds the sign-off, and the `-m` flag specifies your commit message.

**⚠️ WARNING: Any PR with formatting issues will be automatically rejected by CI.**

### For New Features
1. Add appropriate test cases
2. Update documentation if needed
3. Ensure BDD generation works correctly
4. Verify cross-platform compatibility

### For Bug Fixes
1. Add regression test if applicable
2. Verify fix doesn't break existing functionality
3. Test with various expression types
4. Document the fix in commit messages

This directive helps Copilot understand the project structure, build process, testing methodology, and code formatting requirements to assist effectively with development tasks.
