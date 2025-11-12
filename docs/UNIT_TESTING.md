# Unit Testing Integration

This project now includes a Catch2-based unit testing system alongside the existing integration tests.

## Test Types

### Integration Tests (Existing)
- Located in `test_expressions/`
- Test the complete application functionality
- Generate BDD outputs and compare against reference files
- Test command-line interface and error handling

### Unit Tests (New)
- Located in `tests/unit/`
- Test individual components and functions
- Fast-running, focused tests
- Built with Catch2 framework

## Running Tests

### All Tests
```powershell
# Run everything (integration + unit tests)
cd build
ctest

# With verbose output
ctest --verbose
```

### Integration Tests Only
```powershell
# Run traditional BDD expression tests
ctest -R "test_.*" -E "mermaid"

# Run command-line tests
ctest -R "(help|error|option|file)"

# Run mermaid generation tests
ctest -R "mermaid"
```

### Unit Tests Only
```powershell
# Run through CTest
ctest -R "ExpressionParser|TeddyGraph|CuddGraph|ExpressionGraph"

# Run directly
./build/bin/tests/unit_tests.exe

# Run with verbose output
./build/bin/tests/unit_tests.exe -s

# Using custom target
cmake --build build --target run_unit_tests
```

## Adding Unit Tests

1. Create test files in `tests/unit/` following the pattern `test_component_name.cpp`
2. Include `catch2/catch_test_macros.hpp` and your component headers
3. Add the test file to the `unit_tests` target in `tests/CMakeLists.txt`
4. Write tests using Catch2 macros (`TEST_CASE`, `SECTION`, `REQUIRE`, `CHECK`)

See `tests/README.md` for detailed instructions and examples.

## Build Configuration

Unit tests are enabled by default. To disable:
```powershell
cmake -B build -S . -DBUILD_UNIT_TESTS=OFF
```

The unit testing system:
- Automatically downloads and configures Catch2
- Creates a library (`bdd_lib`) from the main source files
- Compiles unit tests against this library
- Integrates with CTest for unified test execution
- Supports all the same compiler flags (sanitizers, coverage, etc.)

## Coverage and Analysis

Unit tests participate in code coverage analysis:
```powershell
cmake -B build -S . -DENABLE_COVERAGE=ON
cmake --build build --config Release
ctest  # Runs all tests including unit tests
./scripts/generate_coverage.ps1
```

For more details, see `tests/README.md`.