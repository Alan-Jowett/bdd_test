# Copyright (c) BDD Expression Converter contributors
# SPDX-License-Identifier: MIT

<#
.SYNOPSIS
    Check and fix code formatting with clang-format
.DESCRIPTION
    This script checks if all C++ source files comply with the project's clang-format style.
    It can be run manually or as part of pre-commit hooks.
.PARAMETER Fix
    If specified, automatically fix formatting issues instead of just checking
.PARAMETER Staged
    If specified, only check files that are staged for commit
.PARAMETER Verbose
    If specified, show detailed output
.PARAMETER Quiet
    If specified, suppress non-error output
.PARAMETER WhatIf
    If specified, show what would be changed without making changes
.EXAMPLE
    .\scripts\check-format.ps1
    Check formatting without fixing issues
.EXAMPLE
    .\scripts\check-format.ps1 -Fix
    Automatically fix formatting issues
.EXAMPLE
    .\scripts\check-format.ps1 -Staged -WhatIf
    Check what would be changed in staged files
#>

param(
    [switch]$Fix,
    [switch]$Staged,
    [switch]$Verbose,
    [switch]$Quiet,
    [switch]$WhatIf
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
    if (-not $Quiet) {
        Write-Host "${Color}${Message}${Reset}"
    }
}

function Write-VerboseOutput {
    param(
        [string]$Message,
        [string]$Color = $Reset
    )
    if ($Verbose -and -not $Quiet) {
        Write-Host "${Color}${Message}${Reset}"
    }
}

# Get the script directory and project root
$ScriptDir = Split-Path -Parent $PSCommandPath
$ProjectRoot = Split-Path -Parent $ScriptDir

# Change to project root
Push-Location $ProjectRoot

try {
    Write-ColorOutput "🔍 Checking clang-format availability..." $Blue

    # Check if clang-format is available
    $clangFormat = $null
    try {
        $clangFormat = Get-Command clang-format -ErrorAction Stop
        Write-VerboseOutput "✓ Found clang-format at: $($clangFormat.Source)" $Green

        # Check clang-format version
        $versionOutput = & clang-format --version 2>$null
        if ($versionOutput) {
            $version = ($versionOutput -split " ")[2]
            Write-VerboseOutput "✓ clang-format version: $version" $Green
        }
    } catch {
        Write-ColorOutput "❌ Error: clang-format not found in PATH" $Red
        Write-ColorOutput "Please install clang-format or add it to your PATH" $Yellow
        Write-ColorOutput "On Windows: Install Visual Studio with C++ tools or LLVM" $Yellow
        exit 1
    }

    # Find source files to check
    Write-ColorOutput "📁 Finding C++ source files..." $Blue
    $sourceFiles = @()

    if ($Staged) {
        # Get staged files
        $stagedFiles = git diff --cached --name-only --diff-filter=ACMR 2>$null
        if ($stagedFiles) {
            foreach ($file in $stagedFiles) {
                if ($file -match '\.(cpp|hpp|c|h|cc|cxx|hxx)$' -and (Test-Path $file)) {
                    $sourceFiles += Get-Item $file
                }
            }
        }
        Write-VerboseOutput "Found $($stagedFiles.Count) staged files, $($sourceFiles.Count) are C++ files" $Blue
    } else {
        # Look for common C++ file extensions
        $extensions = @("*.cpp", "*.hpp", "*.c", "*.h", "*.cc", "*.cxx", "*.hxx")

        foreach ($ext in $extensions) {
            $files = Get-ChildItem -Recurse -Include $ext | Where-Object {
                # Exclude build directory and dependencies
                $_.FullName -notmatch "\\build\\" -and
                $_.FullName -notmatch "\\_deps\\" -and
                $_.FullName -notmatch "\\CMakeFiles\\" -and
                $_.FullName -notmatch "\\\.git\\"
            }
            $sourceFiles += $files
        }
    }

    if ($sourceFiles.Count -eq 0) {
        if ($Staged) {
            Write-ColorOutput "ℹ️  No staged C++ files found to check" $Yellow
        } else {
            Write-ColorOutput "⚠️  No C++ source files found" $Yellow
        }
        exit 0
    }

    Write-ColorOutput "📋 Found $($sourceFiles.Count) source files to check" $Blue

    if ($Verbose) {
        foreach ($file in $sourceFiles) {
            $relativePath = Resolve-Path -Relative $file.FullName
            Write-VerboseOutput "  - $relativePath" $Blue
        }
    }

    $issuesFound = $false
    $filesWithIssues = @()
    $changeCount = 0

    Write-ColorOutput "🔧 Checking formatting..." $Blue

    foreach ($file in $sourceFiles) {
        $relativePath = Resolve-Path -Relative $file.FullName

        if ($Fix -and -not $WhatIf) {
            # Fix formatting
            Write-VerboseOutput "  Formatting: $relativePath" $Blue
            & clang-format -i $file.FullName
            if ($LASTEXITCODE -eq 0) {
                Write-VerboseOutput "  ✓ Formatted: $relativePath" $Green
            } else {
                Write-ColorOutput "  ❌ Failed to format: $relativePath" $Red
                $issuesFound = $true
            }
        } else {
            # Check formatting
            $formatted = & clang-format $file.FullName 2>$null
            $original = Get-Content $file.FullName -Raw

            if ($formatted -ne $original) {
                $issuesFound = $true
                $filesWithIssues += $relativePath
                $changeCount++

                if ($WhatIf) {
                    Write-ColorOutput "❌ Would change: $relativePath" $Yellow
                    if ($Verbose) {
                        # Show diff
                        $tempFile = New-TemporaryFile
                        $formatted | Set-Content $tempFile.FullName
                        Write-ColorOutput "--- Expected changes for $relativePath ---" $Yellow
                        try {
                            $diff = Compare-Object (Get-Content $file.FullName) (Get-Content $tempFile.FullName) -IncludeEqual | Where-Object { $_.SideIndicator -ne "==" }
                            foreach ($line in $diff[0..10]) { # Show first 10 diff lines
                                $indicator = if ($line.SideIndicator -eq "<=") { "-" } else { "+" }
                                $color = if ($line.SideIndicator -eq "<=") { $Red } else { $Green }
                                Write-ColorOutput "$indicator $($line.InputObject)" $color
                            }
                            if ($diff.Count -gt 10) {
                                Write-ColorOutput "... and $($diff.Count - 10) more changes" $Yellow
                            }
                        } finally {
                            Remove-Item $tempFile.FullName -Force -ErrorAction SilentlyContinue
                        }
                        Write-ColorOutput "--- End changes for $relativePath ---" $Yellow
                    }
                } else {
                    Write-ColorOutput "❌ $relativePath" $Red
                }
            } else {
                Write-VerboseOutput "✓ $relativePath" $Green
            }
        }
    }

    # Report results
    if ($Fix -and -not $WhatIf) {
        Write-ColorOutput "✅ Formatting complete!" $Green
    } elseif ($WhatIf) {
        if ($changeCount -gt 0) {
            Write-ColorOutput "⚠️  $changeCount files would be changed by formatting" $Yellow
        } else {
            Write-ColorOutput "All files are properly formatted!" $Green
        }
    } else {
        if ($issuesFound) {
            Write-ColorOutput "❌ Formatting issues found!" $Red

            if ($filesWithIssues.Count -gt 0) {
                Write-ColorOutput "`nFiles with formatting issues:" $Red
                foreach ($file in $filesWithIssues) {
                    Write-ColorOutput "  - $file" $Red
                }
            }

            Write-ColorOutput "`n💡 To fix these issues automatically, run:" $Yellow
            Write-ColorOutput "   .\scripts\check-format.ps1 -Fix" $Yellow
            Write-ColorOutput "`n💡 Or run clang-format manually:" $Yellow
            Write-ColorOutput "   clang-format -i filename" $Yellow

            exit 1
        } else {
            Write-ColorOutput "✅ All files are properly formatted!" $Green
        }
    }

    if ($WhatIf -and $changeCount -gt 0) {
        exit 1
    }

} finally {
    Pop-Location
}
