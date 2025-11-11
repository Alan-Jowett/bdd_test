#!/usr/bin/env pwsh
# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

<#
.SYNOPSIS
    Generate code coverage report locally for development

.DESCRIPTION
    This script builds the project with coverage flags, runs tests, and generates
    a local HTML coverage report for development purposes.

.PARAMETER BuildDir
    The build directory to use (default: build-coverage)

.PARAMETER OpenReport
    Whether to open the coverage report in the default browser

.EXAMPLE
    .\generate_coverage.ps1

.EXAMPLE
    .\generate_coverage.ps1 -BuildDir "my-build" -OpenReport
#>

param(
    [string]$BuildDir = "build-coverage",
    [switch]$OpenReport
)

# Ensure we're in the project root
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $scriptDir

Write-Host "🔄 Generating code coverage report..." -ForegroundColor Green

# Check if required tools are available
$requiredTools = @("cmake", "lcov", "genhtml")
foreach ($tool in $requiredTools) {
    if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
        Write-Error "❌ Required tool '$tool' not found. Please install it first."
        Write-Host "On Ubuntu/Debian: sudo apt-get install cmake lcov"
        Write-Host "On macOS: brew install cmake lcov"
        Write-Host "On Windows: Use WSL or install via vcpkg/package manager"
        exit 1
    }
}

try {
    # Create and enter build directory
    if (Test-Path $BuildDir) {
        Write-Host "🗑️  Cleaning existing build directory..."
        Remove-Item $BuildDir -Recurse -Force
    }

    New-Item -ItemType Directory -Path $BuildDir | Out-Null
    Set-Location $BuildDir

    # Configure with coverage enabled
    Write-Host "⚙️  Configuring CMake with coverage enabled..."
    cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON

    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }

    # Build the project
    Write-Host "🔨 Building project..."
    cmake --build . --config Debug --parallel

    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }

    # Run tests
    Write-Host "🧪 Running tests..."
    ctest --output-on-failure --verbose

    if ($LASTEXITCODE -ne 0) {
        Write-Warning "⚠️  Some tests failed, but continuing with coverage generation..."
    }

    # Generate coverage data
    Write-Host "📊 Generating coverage data..."

    # Capture coverage info
    lcov --capture --directory . --output-file coverage.info --rc branch_coverage=1 --ignore-errors graph,gcov,unused,mismatch

    if ($LASTEXITCODE -ne 0) {
        throw "Failed to capture coverage data"
    }

    # Filter out external dependencies and system files
    lcov --remove coverage.info '/usr/*' '*/_deps/*' '*/CMakeFiles/*' --output-file coverage_filtered.info --rc branch_coverage=1 --ignore-errors unused,mismatch

    if ($LASTEXITCODE -ne 0) {
        throw "Failed to filter coverage data"
    }

    # Generate HTML report
    Write-Host "🌐 Generating HTML coverage report..."
    genhtml coverage_filtered.info --output-directory coverage_html --branch-coverage --function-coverage --title "BDD Demo Coverage Report"

    if ($LASTEXITCODE -ne 0) {
        throw "Failed to generate HTML report"
    }

    # Display summary
    Write-Host ""
    Write-Host "✅ Coverage report generated successfully!" -ForegroundColor Green
    Write-Host "📁 Build directory: $BuildDir" -ForegroundColor Cyan
    Write-Host "📄 Coverage info: $BuildDir/coverage_filtered.info" -ForegroundColor Cyan
    Write-Host "🌐 HTML report: $BuildDir/coverage_html/index.html" -ForegroundColor Cyan

    # Show coverage summary
    Write-Host ""
    Write-Host "📈 Coverage Summary:" -ForegroundColor Yellow
    lcov --summary coverage_filtered.info

    # Optionally open the report
    if ($OpenReport) {
        $indexPath = Join-Path (Get-Location) "coverage_html/index.html"
        if (Test-Path $indexPath) {
            Write-Host "🌐 Opening coverage report in browser..."
            Start-Process $indexPath
        }
    } else {
        Write-Host ""
        Write-Host "💡 Tip: Add -OpenReport to automatically open the HTML report" -ForegroundColor Gray
    }

} catch {
    Write-Error "❌ Coverage generation failed: $_"
    exit 1
} finally {
    # Return to original directory
    Set-Location $scriptDir
}
