# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

# Local Documentation Generation Script (PowerShell)
# This script mirrors the GitHub Actions workflow for local testing

param(
    [switch]$Open = $false
)

Write-Host "🔧 Local Documentation Generation" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan

# Check if Doxygen is available
try {
    $doxygenVersion = doxygen --version 2>&1
    Write-Host "✅ Found Doxygen version: $doxygenVersion" -ForegroundColor Green
} catch {
    Write-Host "❌ Error: Doxygen is not installed or not in PATH" -ForegroundColor Red
    Write-Host "   Please install Doxygen to generate documentation" -ForegroundColor Yellow
    Write-Host "   - Windows: Download from doxygen.org" -ForegroundColor Yellow
    Write-Host "   - Chocolatey: choco install doxygen.install" -ForegroundColor Yellow
    Write-Host "   - Scoop: scoop install doxygen" -ForegroundColor Yellow
    exit 1
}

# Get version information (similar to GitHub Actions)
try {
    $null = git describe --tags --exact-match 2>&1
    if ($LASTEXITCODE -eq 0) {
        $version = git describe --tags --exact-match
        Write-Host "📦 Building documentation for tag: $version" -ForegroundColor Blue
    } else {
        $version = git branch --show-current
        if ([string]::IsNullOrEmpty($version)) {
            $version = "unknown"
        }
        Write-Host "📦 Building documentation for branch: $version" -ForegroundColor Blue
    }
} catch {
    $version = "unknown"
    Write-Host "📦 Building documentation for version: $version" -ForegroundColor Blue
}

# Create a temporary Doxyfile with updated version
Write-Host "🔄 Updating Doxyfile with version: $version" -ForegroundColor Yellow

$doxyfileContent = Get-Content "Doxyfile" -Raw
$updatedContent = $doxyfileContent -replace "PROJECT_NUMBER.*=.*", "PROJECT_NUMBER = $version"
$updatedContent | Out-File "Doxyfile.tmp" -Encoding UTF8

# Generate documentation
Write-Host "📚 Generating documentation..." -ForegroundColor Yellow

try {
    doxygen Doxyfile.tmp
    if ($LASTEXITCODE -ne 0) {
        throw "Doxygen generation failed"
    }
} catch {
    Write-Host "❌ Error: Documentation generation failed" -ForegroundColor Red
    Remove-Item "Doxyfile.tmp" -ErrorAction SilentlyContinue
    exit 1
}

# Cleanup
Remove-Item "Doxyfile.tmp" -ErrorAction SilentlyContinue

# Verify output
if (-not (Test-Path "docs\html")) {
    Write-Host "❌ Error: Documentation generation failed - no docs\html directory found" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path "docs\html\index.html")) {
    Write-Host "❌ Error: No index.html generated" -ForegroundColor Red
    exit 1
}

# Add .nojekyll for GitHub Pages compatibility
New-Item "docs\html\.nojekyll" -ItemType File -Force | Out-Null

Write-Host "✅ Documentation generated successfully!" -ForegroundColor Green
Write-Host "📁 Output directory: docs\html\" -ForegroundColor Green
Write-Host "🌐 Open docs\html\index.html in your browser" -ForegroundColor Green

# Try to open documentation if requested
if ($Open) {
    Write-Host "🚀 Opening documentation..." -ForegroundColor Cyan
    try {
        Start-Process "docs\html\index.html"
    } catch {
        Write-Host "💡 Please manually open docs\html\index.html to view the documentation" -ForegroundColor Yellow
    }
} else {
    Write-Host "💡 Use -Open parameter to automatically open the documentation" -ForegroundColor Blue
}

Write-Host "✨ Done!" -ForegroundColor Green
