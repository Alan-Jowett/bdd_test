# Code Coverage

## Overview

This project supports comprehensive code coverage analysis to ensure high-quality testing and code reliability.

## Automated Coverage (CI/CD)

- **Coveralls Integration**: Automatic coverage reporting on every push
- **GitHub Actions**: Dedicated coverage workflow with detailed reporting
- **Branch Coverage**: Tracks both line and branch coverage metrics
- **Pull Request Reports**: Coverage changes displayed in PR reviews

## Local Coverage Generation

Generate coverage reports for development:

```powershell
# Generate coverage report locally
./generate_coverage.ps1

# Generate and open report in browser
./generate_coverage.ps1 -OpenReport
```

## Coverage Configuration

Enable coverage in your build:

```powershell
mkdir build-coverage
cd build-coverage
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
cmake --build . --config Debug
ctest
```

## Coverage Reports

Coverage reports include:
- **Line Coverage**: Percentage of code lines executed
- **Branch Coverage**: Percentage of conditional branches tested
- **Function Coverage**: Percentage of functions called
- **HTML Reports**: Interactive coverage visualization

## Requirements

### For Linux/macOS
- `gcov` (usually included with GCC)
- `lcov` for HTML report generation:
  ```bash
  # Ubuntu/Debian
  sudo apt-get install lcov
  
  # macOS
  brew install lcov
  ```

### For Windows
- Visual Studio with coverage tools
- Or use GCC/MinGW with gcov support

## Interpreting Coverage Results

### Good Coverage Metrics
- **Line Coverage**: Aim for >90%
- **Branch Coverage**: Aim for >85%
- **Function Coverage**: Should be close to 100%

### Coverage Reports Location
- HTML reports: `coverage/html/index.html`
- Text summary: Displayed in console output
- CI reports: Available on Coveralls badge link

## Troubleshooting Coverage Issues

**No coverage data generated**
- Ensure you built with `-DENABLE_COVERAGE=ON`
- Run tests after building: `ctest`
- Check that gcov/lcov tools are installed

**Incomplete coverage**
- Verify all test cases are running successfully
- Check that source files are being instrumented
- Review compiler flags for coverage support