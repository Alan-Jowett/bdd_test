# Contributing to BDD Expression Converter

Thank you for your interest in contributing to the BDD Expression Converter!

## 📚 Documentation Moved

The detailed contributing guidelines have been moved to provide better organization and more comprehensive information.

**Please see the complete contributing guide at: [docs/CONTRIBUTING.md](docs/CONTRIBUTING.md)**

## Quick Links

- **[Development Guidelines](docs/CONTRIBUTING.md#development-workflow)** - Code style, formatting requirements, and workflow
- **[Testing Requirements](docs/CONTRIBUTING.md#testing)** - How to add and run tests
- **[Code Style](docs/CONTRIBUTING.md#code-style-guidelines)** - Mandatory clang-format requirements
- **[Adding Test Expressions](docs/CONTRIBUTING.md#adding-new-test-expressions)** - Step-by-step guide

## Quick Start

1. **Fork and clone** the repository
2. **Create a feature branch**: `git checkout -b feature/your-feature-name`
3. **Follow the [detailed guidelines](docs/CONTRIBUTING.md)** for development
4. **Format your code**: `clang-format -i *.cpp *.h` (mandatory)
5. **Test your changes**: `cmake --build build && ctest`
6. **Submit a PR** with a clear description

For the complete contributing guide with detailed requirements, please visit **[docs/CONTRIBUTING.md](docs/CONTRIBUTING.md)**.

## Code of Conduct

This project follows a simple code of conduct: be respectful, constructive, and professional in all interactions.
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Running Tests
```bash
cd build
ctest -C Release
```

## Making Changes

### Code Style Guidelines
- Use consistent indentation (spaces preferred)
- Follow existing naming conventions
- Add comments for complex logic
- Keep functions focused and reasonably sized
- Use meaningful variable names

### Commit Message Format
Use clear, descriptive commit messages:
```
type(scope): brief description

Longer explanation if needed.

Closes #issue-number
```

Types:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `test`: Adding or updating tests
- `refactor`: Code refactoring
- `perf`: Performance improvements
- `build`: Build system changes

### Testing Requirements
- All new features must include appropriate tests
- Existing tests must continue to pass
- Test your changes with various expression types:
  - Simple expressions: `A & B`
  - Complex expressions: `(A | B) & (C | D)`
  - Edge cases: Single variables, nested parentheses

### Documentation Updates
When making changes that affect usage:
- Update the README.md if needed
- Add inline code comments for complex logic
- Update example expressions if adding new features

## Submitting Changes

### Pull Request Process
1. **Ensure tests pass**: Run `ctest -C Release` locally
2. **Update documentation**: If your changes affect usage
3. **Create a pull request** with:
   - Clear description of changes
   - Link to any related issues
   - Screenshots of output changes (if applicable)

### Pull Request Checklist
- [ ] Tests pass locally
- [ ] Code follows project style
- [ ] Documentation updated (if needed)
- [ ] Commit messages are clear
- [ ] No merge conflicts with main branch

## Types of Contributions

### Bug Reports
- Use the bug report template
- Include specific expression that caused the issue
- Provide system information (OS, compiler)
- Include expected vs actual behavior

### Feature Requests
- Use the feature request template
- Explain the use case and benefit
- Consider backward compatibility
- Discuss implementation approach

### Code Contributions
We welcome contributions in these areas:
- **Core BDD functionality**: Improvements to BDD operations
- **Expression parsing**: Support for new operators or syntax
- **Output formats**: New visualization or export options
- **Performance**: Optimizations for large expressions
- **Testing**: Additional test cases or testing tools
- **Documentation**: Improved examples or explanations

## Development Guidelines

### Adding New Expression Types
When adding support for new logical operators:
1. Update the `ExpressionNode` variant in `main.cpp`
2. Add parsing logic in the expression parser
3. Implement BDD conversion for the new operator
4. Add test cases in `test_expressions/` with expected results in `test_expressions/expected_output/`
5. Update documentation

### Modifying Output Formats
When changing output formats:
1. Ensure backward compatibility when possible
2. Update any dependent scripts or tools
3. Add tests that verify output format
4. Document any breaking changes

### Performance Considerations
- Profile changes that might affect performance
- Consider memory usage for large expressions
- Test with complex nested expressions
- Document any performance trade-offs

## Getting Help

If you have questions about contributing:
- Open an issue with the "question" label
- Review existing issues and pull requests
- Check the README for usage examples

## Recognition

Contributors will be acknowledged in release notes and project documentation. Thank you for helping make this project better!

## License

By contributing to this project, you agree that your contributions will be licensed under the MIT License.
