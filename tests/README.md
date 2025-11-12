# Unit Testing with Catch2

This directory contains the Catch2-based unit testing system for the BDD Demo project. The tests are designed to complement the existing integration tests by providing focused, fast-running unit tests for individual components.

## Structure

```
tests/
├── CMakeLists.txt          # CMake configuration for unit tests
├── README.md               # This file
└── unit/                   # Unit test files
    ├── test_main.cpp       # Test entry point (uses Catch2::Catch2WithMain)
    ├── test_expression_parser.cpp    # Tests for expression parsing
    ├── test_teddy_graph.cpp          # Tests for TeDDy BDD functionality
    ├── test_cudd_graph.cpp           # Tests for CUDD BDD functionality
    └── test_expression_graph.cpp     # Tests for expression graph functionality
```

## Building and Running Tests

### Prerequisites
- CMake 3.20 or higher
- C++20 compatible compiler
- Internet connection (for downloading Catch2 dependency)

### Build Configuration
Unit tests are enabled by default. To disable them:
```powershell
cmake -B build -S . -DBUILD_UNIT_TESTS=OFF
```

### Building Tests
```powershell
# Configure and build the entire project (includes unit tests)
cmake -B build -S .
cmake --build build --config Release

# Build only the unit test executable
cmake --build build --target unit_tests --config Release
```

### Running Unit Tests

#### Method 1: Using CTest (Recommended)
```powershell
# Run all tests (integration + unit tests)
cd build
ctest

# Run only unit tests
ctest -R "unit_"

# Run with verbose output
ctest -R "unit_" --verbose
```

#### Method 2: Direct Execution
```powershell
# Run all unit tests
./build/bin/tests/unit_tests.exe

# Run with verbose output (shows individual assertions)
./build/bin/tests/unit_tests.exe -s

# Run specific test cases
./build/bin/tests/unit_tests.exe -t "ExpressionParser*"

# List all available tests
./build/bin/tests/unit_tests.exe --list-tests
```

#### Method 3: Using Make Targets
```powershell
# Run unit tests using custom target
cmake --build build --target run_unit_tests

# Run unit tests with verbose output
cmake --build build --target run_unit_tests_verbose
```

## Writing Tests

### Test File Structure
Each test file should follow this structure:

```cpp
// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

#include <catch2/catch_test_macros.hpp>
#include "your_header.hpp"

TEST_CASE("Description of what you're testing", "[tag]") {
    SECTION("Specific scenario") {
        // Setup
        YourClass object;

        // Test
        auto result = object.someMethod();

        // Assertions
        REQUIRE(result.has_value());
        CHECK(result.value() > 0);
    }
}
```

### Key Catch2 Macros
- `TEST_CASE("name", "[tag]")` - Define a test case
- `SECTION("name")` - Create test sections within a test case
- `REQUIRE(condition)` - Hard assertion (stops test if fails)
- `CHECK(condition)` - Soft assertion (continues test if fails)
- `REQUIRE_THROWS(expression)` - Assert that expression throws
- `REQUIRE_NOTHROW(expression)` - Assert that expression doesn't throw

### Test Tags
Use tags to organize tests:
- `[expression_parser]` - Parser functionality
- `[teddy_graph]` - TeDDy BDD operations
- `[cudd_graph]` - CUDD BDD operations
- `[expression_graph]` - Expression graph functionality
- `[integration]` - Integration between components
- `[performance]` - Performance-related tests
- `[edge_case]` - Edge cases and error conditions

### Adding New Test Files
1. Create your test file in the `unit/` directory
2. Include the appropriate headers and `catch2/catch_test_macros.hpp`
3. Write your test cases using Catch2 macros
4. Add the file to the `unit_tests` target in `tests/CMakeLists.txt`

## Test Guidelines

### What to Test
- **Public API functionality** - Test all public methods and functions
- **Edge cases** - Test boundary conditions and invalid inputs
- **Error handling** - Verify proper exception handling
- **Integration points** - Test how components work together
- **Performance** - Add performance tests for critical paths

### What NOT to Test
- **Private implementation details** - Focus on public interfaces
- **Third-party library internals** - Trust TeDDy and CUDD implementations
- **File I/O extensively** - Integration tests cover this better

### Test Data
- Use simple, predictable test data
- Create helper functions for complex setup
- Consider using Catch2's generators for parameterized tests

## Current Test Status

All test files are currently placeholder implementations with TODO comments. As you investigate different components, implement the actual test cases by replacing the TODOs with real test code.

### Test Implementation Priorities
1. **Expression Parser** - Start here as it's the foundation
2. **Expression Graph** - Basic graph operations
3. **TeDDy Graph** - BDD construction and operations
4. **CUDD Graph** - Alternative BDD implementation
5. **Integration Tests** - Cross-component functionality

## Debugging Tests

### Running Individual Tests
```powershell
# Run a specific test case
./build/bin/tests/unit_tests.exe -t "ExpressionParser basic functionality"

# Run all tests with a specific tag
./build/bin/tests/unit_tests.exe "[expression_parser]"
```

### Debugging with Visual Studio
The unit test executable can be debugged directly in Visual Studio:
1. Set `unit_tests` as the startup project
2. Set breakpoints in your test code
3. Run with debugging (F5)

### Test Output
- Tests provide detailed output on failure
- Use `CHECK` instead of `REQUIRE` to see multiple assertion failures
- Use the `-s` flag for verbose output showing all sections

## Coverage Analysis

Unit tests work with the existing coverage system:

```powershell
# Build with coverage enabled
cmake -B build -S . -DENABLE_COVERAGE=ON
cmake --build build --config Release

# Run all tests (integration + unit)
cd build
ctest

# Generate coverage report
../scripts/generate_coverage.ps1
```

Unit tests should help increase code coverage by testing individual functions and edge cases that may not be covered by integration tests.

## Continuous Integration

Unit tests are automatically run by the CI system along with integration tests. They must:
- Pass on all supported platforms
- Complete within the timeout limits
- Not require external files or network access
- Be deterministic (no random failures)

## Best Practices

1. **Keep tests focused** - Each test should verify one specific behavior
2. **Make tests fast** - Unit tests should run in milliseconds
3. **Use descriptive names** - Test names should clearly state what's being tested
4. **Organize with sections** - Use sections to group related assertions
5. **Test edge cases** - Don't just test the happy path
6. **Mock dependencies** - Isolate the unit under test when possible
7. **Keep tests independent** - Tests should not depend on each other's state