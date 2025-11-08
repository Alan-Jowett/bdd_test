# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

# PowerShell script to generate PNG visualizations from all DOT files
# Author: BDD Expression Converter
# Description: Automatically finds all DOT files and creates corresponding PNG images using Graphviz

Write-Host "BDD Visualization Generator" -ForegroundColor Green
Write-Host "==========================" -ForegroundColor Green
Write-Host ""

# Check if Graphviz is available
try {
    $null = Get-Command "dot" -ErrorAction Stop
    Write-Host "✓ Graphviz found" -ForegroundColor Green
} catch {
    Write-Host "✗ Graphviz not found. Please install Graphviz and ensure 'dot' is in your PATH." -ForegroundColor Red
    Write-Host "  Windows: choco install graphviz" -ForegroundColor Yellow
    Write-Host "  Windows: winget install Graphviz.Graphviz" -ForegroundColor Yellow
    exit 1
}

# Change to the script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

# Find all DOT files recursively
$DotFiles = Get-ChildItem -Recurse -Filter "*.dot" | Where-Object { $_.Name -notlike ".*" }

if ($DotFiles.Count -eq 0) {
    Write-Host "No DOT files found in the current directory and subdirectories." -ForegroundColor Yellow
    exit 0
}

Write-Host "Found $($DotFiles.Count) DOT file(s):" -ForegroundColor Cyan
$DotFiles | ForEach-Object { Write-Host "  - $($_.RelativePath ?? $_.Name)" -ForegroundColor Gray }
Write-Host ""

$SuccessCount = 0
$ErrorCount = 0

foreach ($DotFile in $DotFiles) {
    $InputPath = $DotFile.FullName
    $OutputPath = $InputPath -replace '\.dot$', '.png'
    $RelativePath = Resolve-Path -Path $InputPath -Relative

    Write-Host "Processing: $RelativePath" -ForegroundColor Cyan

    try {
        # Run Graphviz dot command
        $Process = Start-Process -FilePath "dot" -ArgumentList @("-Tpng", "`"$InputPath`"", "-o", "`"$OutputPath`"") -Wait -PassThru -NoNewWindow

        if ($Process.ExitCode -eq 0) {
            Write-Host "  ✓ Generated: $(Resolve-Path -Path $OutputPath -Relative)" -ForegroundColor Green
            $SuccessCount++
        } else {
            Write-Host "  ✗ Failed to generate PNG (exit code: $($Process.ExitCode))" -ForegroundColor Red
            $ErrorCount++
        }
    } catch {
        Write-Host "  ✗ Error: $($_.Exception.Message)" -ForegroundColor Red
        $ErrorCount++
    }
}

Write-Host ""
Write-Host "Summary:" -ForegroundColor White
Write-Host "  ✓ Success: $SuccessCount" -ForegroundColor Green
Write-Host "  ✗ Errors:  $ErrorCount" -ForegroundColor Red

if ($ErrorCount -eq 0) {
    Write-Host ""
    Write-Host "All visualizations generated successfully!" -ForegroundColor Green
    exit 0
} else {
    Write-Host ""
    Write-Host "Some visualizations failed to generate. Check the errors above." -ForegroundColor Yellow
    exit 1
}