# Copyright (c) BDD Expression Converter contributors
# SPDX-License-Identifier: MIT

<#
.SYNOPSIS
    Setup development environment for BDD Expression Converter
.DESCRIPTION
    This script sets up the development environment including required tools,
    pre-commit hooks, and development dependencies.
.PARAMETER SkipPreCommit
    Skip installing pre-commit hooks
.PARAMETER UseFramework
    Use pre-commit framework instead of simple Git hooks
.PARAMETER SkipTools
    Skip installing development tools
.EXAMPLE
    .\scripts\setup-dev-env.ps1
    Full development environment setup
.EXAMPLE
    .\scripts\setup-dev-env.ps1 -SkipTools
    Setup only pre-commit hooks, skip tool installation
#>

param(
    [switch]$SkipPreCommit,
    [switch]$UseFramework,
    [switch]$SkipTools
)

# Set error action preference
$ErrorActionPreference = "Stop"

# Colors for output
$Red = "`e[31m"
$Green = "`e[32m"
$Yellow = "`e[33m"
$Blue = "`e[34m"
$Reset = "`e[0m"

function Write-ColorOutput {
    param(
        [string]$Message,
        [string]$Color = $Reset
    )
    Write-Host "${Color}${Message}${Reset}"
}

function Test-Command {
    param([string]$Command)
    try {
        Get-Command $Command -ErrorAction Stop | Out-Null
        return $true
    } catch {
        return $false
    }
}

# Get the script directory and project root
$ScriptDir = Split-Path -Parent $PSCommandPath
$ProjectRoot = Split-Path -Parent $ScriptDir

Push-Location $ProjectRoot

try {
    Write-ColorOutput "🚀 Setting up BDD Expression Converter development environment..." $Blue
    Write-ColorOutput ""

    # Check if we're in a Git repository
    try {
        git rev-parse --git-dir | Out-Null
        Write-ColorOutput "✓ Git repository detected" $Green
    } catch {
        Write-ColorOutput "❌ Error: Not in a git repository" $Red
        exit 1
    }

    # Tool installation
    if (-not $SkipTools) {
        Write-ColorOutput "🔧 Checking development tools..." $Blue

        # Check CMake
        if (Test-Command "cmake") {
            $cmakeVersion = (cmake --version | Select-Object -First 1) -replace "cmake version ", ""
            Write-ColorOutput "✓ CMake found: $cmakeVersion" $Green
        } else {
            Write-ColorOutput "⚠️  CMake not found" $Yellow
            Write-ColorOutput "Install CMake: https://cmake.org/download/" $Yellow
        }

        # Check C++ compiler
        $compilerFound = $false
        if (Test-Command "cl") {
            try {
                $msvcVersion = cl 2>&1 | Select-String "Microsoft.*Compiler" | Select-Object -First 1
                Write-ColorOutput "✓ MSVC found: $($msvcVersion.Line)" $Green
                $compilerFound = $true
            } catch {
                # MSVC might be available but not in PATH
            }
        }

        if (-not $compilerFound -and (Test-Command "g++")) {
            $gccVersion = g++ --version | Select-Object -First 1
            Write-ColorOutput "✓ GCC found: $gccVersion" $Green
            $compilerFound = $true
        }

        if (-not $compilerFound -and (Test-Command "clang++")) {
            $clangVersion = clang++ --version | Select-Object -First 1
            Write-ColorOutput "✓ Clang found: $clangVersion" $Green
            $compilerFound = $true
        }

        if (-not $compilerFound) {
            Write-ColorOutput "⚠️  No C++ compiler found" $Yellow
            Write-ColorOutput "Install one of:" $Yellow
            Write-ColorOutput "  • Visual Studio with C++ tools (Windows)" $Yellow
            Write-ColorOutput "  • GCC: sudo apt install build-essential (Linux)" $Yellow
            Write-ColorOutput "  • Clang: xcode-select --install (macOS)" $Yellow
        }

        # Check clang-format
        if (Test-Command "clang-format") {
            $clangFormatVersion = clang-format --version | Select-String "version" | Select-Object -First 1
            Write-ColorOutput "✓ clang-format found: $($clangFormatVersion.Line)" $Green
        } else {
            Write-ColorOutput "⚠️  clang-format not found" $Yellow
            Write-ColorOutput "Install clang-format for code formatting:" $Yellow
            Write-ColorOutput "  • Windows: Part of Visual Studio or install LLVM" $Yellow
            Write-ColorOutput "  • Linux: sudo apt install clang-format" $Yellow
            Write-ColorOutput "  • macOS: brew install clang-format" $Yellow
        }

        # Check Git
        if (Test-Command "git") {
            $gitVersion = git --version
            Write-ColorOutput "✓ Git found: $gitVersion" $Green
        } else {
            Write-ColorOutput "❌ Git not found (required)" $Red
            exit 1
        }

        # Check optional tools
        if (Test-Command "graphviz" -or Test-Command "dot") {
            Write-ColorOutput "✓ Graphviz found (for visualizations)" $Green
        } else {
            Write-ColorOutput "ℹ️  Graphviz not found (optional, needed for PNG generation)" $Blue
            Write-ColorOutput "Install: https://graphviz.org/download/" $Blue
        }

        if (Test-Command "doxygen") {
            $doxygenVersion = doxygen --version
            Write-ColorOutput "✓ Doxygen found: $doxygenVersion (for documentation)" $Green
        } else {
            Write-ColorOutput "ℹ️  Doxygen not found (optional, needed for API docs)" $Blue
        }
    }

    # Pre-commit hooks setup
    if (-not $SkipPreCommit) {
        Write-ColorOutput ""
        Write-ColorOutput "🪝 Setting up pre-commit hooks..." $Blue

        $installArgs = @()
        if ($UseFramework) {
            $installArgs += "-Framework"
        } else {
            $installArgs += "-GitHook"
        }

        $installScript = Join-Path $ScriptDir "install-hooks.ps1"
        if (Test-Path $installScript) {
            & $installScript @installArgs -Force
        } else {
            Write-ColorOutput "❌ Error: install-hooks.ps1 not found" $Red
            exit 1
        }
    }

    # Project-specific setup
    Write-ColorOutput ""
    Write-ColorOutput "📦 Project setup..." $Blue

    # Check if build directory exists
    $buildDir = Join-Path $ProjectRoot "build"
    if (-not (Test-Path $buildDir)) {
        Write-ColorOutput "📁 Creating build directory..." $Blue
        New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
        Write-ColorOutput "✓ Build directory created" $Green
    } else {
        Write-ColorOutput "✓ Build directory exists" $Green
    }

    # Verify clang-format config exists
    $clangFormatConfig = Join-Path $ProjectRoot ".clang-format"
    if (Test-Path $clangFormatConfig) {
        Write-ColorOutput "✓ .clang-format config found" $Green
    } else {
        Write-ColorOutput "⚠️  .clang-format config not found" $Yellow
    }

    # Success summary
    Write-ColorOutput ""
    Write-ColorOutput "🎉 Development environment setup complete!" $Green
    Write-ColorOutput ""
    Write-ColorOutput "📋 Next steps:" $Blue
    Write-ColorOutput "  1. Build the project:" $Blue
    Write-ColorOutput "     cmake -B build -S ." $Blue
    Write-ColorOutput "     cmake --build build --config Release" $Blue
    Write-ColorOutput ""
    Write-ColorOutput "  2. Run tests:" $Blue
    Write-ColorOutput "     cd build && ctest -C Release" $Blue
    Write-ColorOutput ""
    Write-ColorOutput "  3. Check code formatting:" $Blue
    Write-ColorOutput "     .\scripts\check-format.ps1" $Blue
    Write-ColorOutput ""
    Write-ColorOutput "  4. Generate documentation:" $Blue
    Write-ColorOutput "     .\scripts\generate_docs.ps1" $Blue
    Write-ColorOutput ""
    Write-ColorOutput "💡 The pre-commit hooks will automatically check your code before commits!" $Yellow

} catch {
    Write-ColorOutput "❌ Error during setup: $_" $Red
    exit 1
} finally {
    Pop-Location
}
