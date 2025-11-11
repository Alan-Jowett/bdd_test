# Building the BDD Expression Converter

## Prerequisites

- CMake 3.20 or higher
- C++20 compatible compiler (GCC 10+, Clang 12+, or MSVC 19.28+)
- Git (for fetching dependencies)
- Optional: Graphviz (for PNG visualization generation)

## Basic Build Instructions

1. Create a build directory:
   ```powershell
   mkdir build
   cd build
   ```

2. Configure the project:
   ```powershell
   cmake ..
   ```

3. Build the project:
   ```powershell
   cmake --build . --config Release
   ```

## Advanced Build Configuration

### Debug Build
```powershell
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

### Coverage Build
```powershell
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
cmake --build . --config Debug
```

### Sanitizer Builds

Sanitizers are powerful tools for detecting memory errors, undefined behavior, and other runtime issues. They are supported on Linux with GCC or Clang compilers.

#### AddressSanitizer (ASan)
Detects memory errors like buffer overflows, use-after-free, memory leaks:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON
cmake --build . --config Debug
```

#### UndefinedBehaviorSanitizer (UBSan)
Detects undefined behavior like integer overflow, null pointer dereference:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_UBSAN=ON
cmake --build . --config Debug
```

#### Combined Sanitizers
Run both ASan and UBSan together for comprehensive checking:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON -DENABLE_UBSAN=ON
cmake --build . --config Debug
```

#### Automated Sanitizer Testing
For convenience, use the provided scripts to run all sanitizer combinations:

**Linux/macOS:**
```bash
# Run all sanitizers with GCC (default)
./scripts/run-sanitizers.sh

# Run specific sanitizer with specific compiler
./scripts/run-sanitizers.sh asan clang
./scripts/run-sanitizers.sh ubsan gcc
./scripts/run-sanitizers.sh asan+ubsan clang
```

**Windows (requires WSL):**
```powershell
# Run all sanitizers with GCC
.\scripts\run-sanitizers.ps1

# Run specific combinations
.\scripts\run-sanitizers.ps1 -Sanitizer asan -Compiler clang
.\scripts\run-sanitizers.ps1 -Sanitizer ubsan -Compiler gcc
```

#### Sanitizer Environment Variables
Fine-tune sanitizer behavior with these environment variables:

**AddressSanitizer:**
```bash
export ASAN_OPTIONS="detect_leaks=1:abort_on_error=1:check_initialization_order=1"
```

**UndefinedBehaviorSanitizer:**
```bash
export UBSAN_OPTIONS="print_stacktrace=1:abort_on_error=1"
```

#### Important Notes
- Sanitizers significantly slow down execution (2-10x slower)
- They require debug builds for best results
- Windows MSVC builds don't support these sanitizers natively
- Use WSL on Windows for sanitizer support
- Sanitizer builds help catch bugs before they reach production

### Cross-Platform Notes

#### Windows
- Recommended to use Visual Studio 2019 or later
- PowerShell is the preferred terminal for commands
- Ensure Git is available in PATH for dependency fetching

#### Linux (Ubuntu/Debian)
```bash
# Install dependencies
sudo apt-get install cmake build-essential git

# Build process is the same as above
```

#### macOS
```bash
# Install dependencies using Homebrew
brew install cmake git

# Build process is the same as above
```

## Build Outputs

After a successful build, you'll find:
- **Executable**: `build/bin/Release/bdd_demo.exe` (Windows) or `build/bin/bdd_demo` (Unix)
- **Test outputs**: Generated in `test_expressions/` subdirectories during testing

## Common Build Issues

**C++20 not supported**
- Solution: Use a newer compiler (GCC 10+, Clang 12+, MSVC 19.28+)

**CMake version too old**
- Solution: Update CMake to version 3.20 or higher

**Git not found**
- Solution: Ensure Git is installed and available in your system PATH

**TeDDy dependency fetch failed**
- Solution: Check internet connection and Git access to GitHub
