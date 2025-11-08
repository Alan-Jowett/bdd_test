# Contributing to BDD Expression Converter

## Getting Started

Thank you for your interest in contributing! This project welcomes contributions from the community.

## Adding New Test Expressions

To add new test expressions:

1. **Create Expression File**: Create a new `.txt` file in `test_expressions/` folder with meaningful variable names
2. **Generate Expected Output**: Run the tool to generate the BDD node file
3. **Save Expected Output**: Copy the generated `*_bdd_nodes.txt` file to `test_expressions/default_ordering/` with **real variable names** (not x0, x1, x2)
4. **Update Build Configuration**: Add the new test to `CMakeLists.txt` using the `add_bdd_test()` function
5. **Verify**: Run `ctest` to ensure the new test passes

**Important Note**: Expected output files should contain the actual variable names from your expression, not generic placeholders.

### Example Test Addition

1. Create `test_expressions/network_filter.txt`:
   ```
   (bandwidth_high AND latency_low) OR emergency_override
   ```

2. Run the tool to generate output:
   ```powershell
   .\build\bin\Release\bdd_demo.exe test_expressions\network_filter.txt
   ```

3. Copy the generated node file:
   ```powershell
   cp test_expressions\default_ordering\network_filter_bdd_nodes.txt test_expressions\default_ordering\
   ```

4. Add test to `CMakeLists.txt`:
   ```cmake
   add_bdd_test("network_filter" "test_expressions/network_filter.txt")
   ```

## Code Style Guidelines

### Code Formatting (MANDATORY)

**ALL code changes MUST be formatted with clang-format before committing.**

```bash
# Format all C++ files (REQUIRED)
find . -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i

# Verify formatting compliance (REQUIRED)
clang-format --dry-run --Werror *.cpp *.h
```

### Style Requirements

- Follow the established clang-format configuration (Google style with customizations)
- Use C++17 features appropriately
- Maintain clear variable naming with descriptive names
- Add comments for complex BDD operations
- Remove all trailing whitespace from files
- Use 4-space indentation, no tabs
- Maximum line length: 100 characters

## Development Workflow

### Before Committing

1. **🔥 MANDATORY: Format all code**: Run formatting commands on ALL files
2. **🔥 MANDATORY: Verify formatting**: Confirm no formatting issues exist
3. **Remove trailing whitespace**: Ensure no lines end with spaces or tabs
4. **Build successfully**: Ensure `cmake --build build` completes without errors
5. **Pass all tests**: Run `ctest` and verify all tests pass
6. **Include signed-off-by**: Use `git commit -s -m "your message"` to include the "Signed-off-by" line

**⚠️ WARNING: Any PR with formatting issues will be automatically rejected by CI.**

### For New Features

1. Add appropriate test cases
2. Update documentation if needed
3. Ensure BDD generation works correctly
4. Verify cross-platform compatibility
5. Add Doxygen comments for new functions

### For Bug Fixes

1. Add regression test if applicable
2. Verify fix doesn't break existing functionality
3. Test with various expression types
4. Document the fix in commit messages
5. Update troubleshooting docs if relevant

## Code Organization

### File Structure
- Keep source files at project root level
- Input expressions go in `test_expressions/`
- Generated outputs appear in `build/test_output/`
- Documentation goes in `docs/`
- Use descriptive filenames for test expressions

### Testing Requirements
- Add new test cases for any expression parsing changes
- Ensure all tests pass before committing
- Verify BDD generation works for edge cases
- Test with various logical operators (AND, OR, NOT, XOR)

## Documentation

### Code Documentation
- Use Doxygen-style comments for all public functions
- Include parameter descriptions with `@param`
- Document return values with `@return`
- Add usage examples with `@code` blocks
- Include cross-references with `@see`

### User Documentation
- Update relevant files in `docs/` for new features
- Add examples to `docs/VISUALIZATIONS.md` for new output formats
- Update troubleshooting guide for new known issues
- Keep README.md focused on core information

## Quality Standards

### Testing
- All new functionality must be tested
- Tests should be deterministic and repeatable
- Use meaningful test names and descriptions
- Verify cross-platform compatibility

### Performance
- Consider BDD size implications of changes
- Test with large expressions when applicable
- Document any performance considerations
- Optimize for common use cases

### Compatibility
- Maintain C++17 compatibility
- Test on multiple platforms when possible
- Preserve backward compatibility for existing expression files
- Document any breaking changes clearly

## Submitting Changes

### Pull Request Process
1. Fork the repository
2. Create a feature branch from `main`
3. Make your changes following the guidelines above
4. Ensure all tests pass and code is formatted
5. Submit a pull request with clear description

### Pull Request Requirements
- **Formatted Code**: All code must pass clang-format checks
- **Passing Tests**: All existing and new tests must pass
- **Clear Description**: Explain what changes were made and why
- **Documentation**: Update relevant documentation
- **Signed Commits**: Include Signed-off-by line in commits

### Review Process
- Code will be reviewed for style, functionality, and testing
- CI/CD pipeline must pass (including formatting checks)
- Reviewers may request changes or improvements
- Once approved, changes will be merged to main branch

## Getting Help

### Development Environment Setup
- See `docs/BUILDING.md` for detailed build instructions
- Review `docs/TROUBLESHOOTING.md` for common issues
- Check existing issues on GitHub for known problems

### Questions and Discussion
- Open an issue for bugs or feature requests
- Use discussions for questions about implementation
- Review existing documentation before asking questions

## Recognition

Contributors will be recognized in:
- Git commit history
- Release notes for significant contributions
- Project documentation for major features

Thank you for contributing to the BDD Expression Converter project!