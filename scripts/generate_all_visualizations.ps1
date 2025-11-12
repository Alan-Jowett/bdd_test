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

# Change to the project root directory (parent of scripts)
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir
Set-Location $ProjectRoot

# Find all DOT files recursively, excluding hidden files and very large files
$MaxSizeBytes = 100KB  # Skip files larger than 100KB as they take too long to convert
$DotFiles = Get-ChildItem -Recurse -Filter "*.dot" | Where-Object {
    $_.Name -notlike ".*" -and $_.Length -le $MaxSizeBytes
}

$LargeFiles = Get-ChildItem -Recurse -Filter "*.dot" | Where-Object {
    $_.Name -notlike ".*" -and $_.Length -gt $MaxSizeBytes
}

if ($LargeFiles.Count -gt 0) {
    Write-Host "Skipping $($LargeFiles.Count) large DOT file(s) (>100KB):" -ForegroundColor Yellow
    $LargeFiles | ForEach-Object {
        $sizeKB = [math]::Round($_.Length / 1KB, 1)
        Write-Host "  - $($_.Name) (${sizeKB}KB)" -ForegroundColor Gray
    }
    Write-Host ""
}

if ($DotFiles.Count -eq 0) {
    Write-Host "No DOT files found in the current directory and subdirectories." -ForegroundColor Yellow
    exit 0
}

Write-Host "Found $($DotFiles.Count) processable DOT file(s)" -ForegroundColor Cyan
Write-Host ""

$SuccessCount = 0
$ErrorCount = 0
$SkippedCount = 0

foreach ($DotFile in $DotFiles) {
    $InputPath = $DotFile.FullName
    $OutputPath = $InputPath -replace '\.dot$', '.png'
    $RelativePath = Resolve-Path -Path $InputPath -Relative
    $sizeKB = [math]::Round($DotFile.Length / 1KB, 1)
    $Progress = $SuccessCount + $ErrorCount + $SkippedCount + 1

    Write-Host "Processing ($Progress/$($DotFiles.Count)): $RelativePath (${sizeKB}KB)" -ForegroundColor Cyan

    # Skip if PNG already exists and is newer than DOT file
    if (Test-Path $OutputPath) {
        $DotTime = $DotFile.LastWriteTime
        $PngTime = (Get-Item $OutputPath).LastWriteTime
        if ($PngTime -ge $DotTime) {
            Write-Host "  ↻ PNG up-to-date, skipping" -ForegroundColor Yellow
            $SkippedCount++
            continue
        }
    }

    try {
        # Run Graphviz dot command with timeout
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
Write-Host "  ↻ Skipped: $SkippedCount (up-to-date)" -ForegroundColor Yellow
Write-Host "  📏 Large files skipped: $($LargeFiles.Count) (>100KB)" -ForegroundColor Magenta
Write-Host "  ✗ Errors:  $ErrorCount" -ForegroundColor Red

if ($ErrorCount -eq 0) {
    Write-Host ""
    if ($SuccessCount -gt 0) {
        Write-Host "All new visualizations generated successfully!" -ForegroundColor Green
    } else {
        Write-Host "All visualizations are up-to-date!" -ForegroundColor Green
    }
    exit 0
} else {
    Write-Host ""
    Write-Host "Some visualizations failed to generate. Check the errors above." -ForegroundColor Yellow
    exit 1
}
