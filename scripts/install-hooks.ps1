# Copyright (c) BDD Expression Converter contributors
# SPDX-License-Identifier: MIT

<#
.SYNOPSIS
    Install pre-commit hooks for the BDD Expression Converter project
.DESCRIPTION
    This script sets up the pre-commit hooks to automatically check code formatting
    and other issues before each commit. It supports both the pre-commit framework
    and a fallback Git hook.
.PARAMETER Framework
    Use the pre-commit framework (requires Python and pre-commit package)
.PARAMETER GitHook
    Use simple Git hook (no dependencies)
.PARAMETER Force
    If specified, overwrites existing pre-commit hook
.EXAMPLE
    .\scripts\install-hooks.ps1
    Install the Git hook (default, no dependencies)
.EXAMPLE
    .\scripts\install-hooks.ps1 -Framework
    Install using pre-commit framework (requires Python)
.EXAMPLE
    .\scripts\install-hooks.ps1 -Force
    Install the hook, overwriting any existing one
#>

param(
    [switch]$Framework,
    [switch]$GitHook,
    [switch]$Force
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

# Get the script directory and project root
$ScriptDir = Split-Path -Parent $PSCommandPath
$ProjectRoot = Split-Path -Parent $ScriptDir

# Ensure we're in a git repository
Push-Location $ProjectRoot

try {
    $gitRoot = git rev-parse --show-toplevel 2>$null
    if (!$gitRoot) {
        Write-ColorOutput "❌ Error: Not in a git repository" $Red
        exit 1
    }

    Write-ColorOutput "🔧 Installing pre-commit hooks..." $Blue

    # Default to GitHook if neither is specified
    if (-not $Framework -and -not $GitHook) {
        $GitHook = $true
    }

    if ($Framework) {
        Write-ColorOutput "📦 Installing using pre-commit framework..." $Blue

        # Check if Python is available
        try {
            $pythonVersion = python --version 2>$null
            Write-ColorOutput "✓ Found Python: $pythonVersion" $Green
        } catch {
            Write-ColorOutput "❌ Error: Python not found. Install Python to use the pre-commit framework." $Red
            Write-ColorOutput "Alternative: Use -GitHook for a simple hook with no dependencies" $Yellow
            exit 1
        }

        # Check if pre-commit is installed
        try {
            $precommitVersion = pre-commit --version 2>$null
            Write-ColorOutput "✓ Found pre-commit: $precommitVersion" $Green
        } catch {
            Write-ColorOutput "⚠️  pre-commit not found, installing..." $Yellow
            try {
                pip install pre-commit
                Write-ColorOutput "✓ Installed pre-commit" $Green
            } catch {
                Write-ColorOutput "❌ Error: Failed to install pre-commit" $Red
                Write-ColorOutput "Install manually with: pip install pre-commit" $Yellow
                exit 1
            }
        }

        # Install pre-commit hooks
        try {
            pre-commit install
            Write-ColorOutput "✅ Pre-commit framework installed successfully!" $Green
        } catch {
            Write-ColorOutput "❌ Error: Failed to install pre-commit hooks" $Red
            exit 1
        }

        Write-ColorOutput ""
        Write-ColorOutput "📋 Pre-commit framework setup complete!" $Green
        Write-ColorOutput "  • Hooks will run automatically on git commit" $Blue
        Write-ColorOutput "  • Run 'pre-commit run --all-files' to check all files" $Blue
        Write-ColorOutput "  • Run 'pre-commit autoupdate' to update hook versions" $Blue

    } elseif ($GitHook) {
        Write-ColorOutput "🔗 Installing simple Git hook..." $Blue

        $hooksDir = Join-Path $ProjectRoot ".git\hooks"
        $preCommitHook = Join-Path $hooksDir "pre-commit"
        $sourceHook = Join-Path $ScriptDir "pre-commit"

        # Check if hooks already exist
        $hookExists = Test-Path $preCommitHook

        if ($hookExists -and -not $Force) {
            Write-ColorOutput "⚠️  Pre-commit hook already exists!" $Yellow
            $response = Read-Host "Do you want to overwrite it? (y/N)"
            if ($response -notmatch '^[yY]') {
                Write-ColorOutput "❌ Installation cancelled" $Red
                exit 1
            }
        }

        # Ensure hooks directory exists
        if (!(Test-Path $hooksDir)) {
            New-Item -ItemType Directory -Path $hooksDir -Force | Out-Null
        }

        # Copy pre-commit hook
        if (Test-Path $sourceHook) {
            Copy-Item $sourceHook $preCommitHook -Force
            Write-ColorOutput "✓ Pre-commit hook installed" $Green
        } else {
            Write-ColorOutput "❌ Error: Pre-commit hook source not found at $sourceHook" $Red
            exit 1
        }

        # Make the hook executable on Unix-like systems
        if ($IsLinux -or $IsMacOS) {
            chmod +x $preCommitHook
        }

        Write-ColorOutput "✅ Git hook installed successfully!" $Green
        Write-ColorOutput ""
        Write-ColorOutput "📋 What happens now:" $Blue
        Write-ColorOutput "  • Before each commit, code formatting will be checked" $Blue
        Write-ColorOutput "  • If issues are found, the commit will be blocked" $Blue
        Write-ColorOutput "  • Use './scripts/check-format.ps1 -Fix' to automatically fix issues" $Blue
        Write-ColorOutput "  • Use 'git commit --no-verify' to skip checks if needed" $Blue
    }

    # Verify the format checker script exists
    $formatChecker = Join-Path $ProjectRoot "scripts\check-format.ps1"
    if (!(Test-Path $formatChecker)) {
        Write-ColorOutput "❌ Error: Format checker script not found" $Red
        Write-ColorOutput "Expected location: $formatChecker" $Red
        exit 1
    }

    # Test clang-format availability
    Write-ColorOutput ""
    Write-ColorOutput "🧪 Testing clang-format availability..." $Blue
    try {
        $clangFormat = Get-Command clang-format -ErrorAction Stop
        Write-ColorOutput "✓ clang-format found at: $($clangFormat.Source)" $Green

        # Test the format checker
        Write-ColorOutput "🧪 Testing format checker..." $Blue
        & $formatChecker -WhatIf -Quiet
        if ($LASTEXITCODE -eq 0) {
            Write-ColorOutput "✓ Format checker working correctly" $Green
        } else {
            Write-ColorOutput "⚠️  Format checker found issues (this is normal)" $Yellow
        }

    } catch {
        Write-ColorOutput "⚠️  Warning: clang-format not found in PATH" $Yellow
        Write-ColorOutput "Install clang-format for formatting checks to work:" $Yellow
        Write-ColorOutput "  • Windows: Install Visual Studio with C++ tools or LLVM" $Yellow
        Write-ColorOutput "  • Linux: sudo apt install clang-format" $Yellow
        Write-ColorOutput "  • macOS: brew install clang-format" $Yellow
    }

    Write-ColorOutput ""
    Write-ColorOutput "💡 Quick test:" $Yellow
    Write-ColorOutput "  Try making a commit to see the hooks in action!" $Yellow

} finally {
    Pop-Location
}
