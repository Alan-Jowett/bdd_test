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
