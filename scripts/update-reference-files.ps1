# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

# PowerShell script to regenerate all reference analysis files for TeDDy and CUDD methods
# This ensures all reference files have the unified Mermaid format

param(
    [Parameter(Position=0)]
    [ValidateSet("teddy", "cudd", "both")]
    [string]$Method = "both"
)

$rootExprDir = "test_expressions"

# Collect top-level expression files (non-recursive) and sort for deterministic order
$expressions = @()
if (Test-Path $rootExprDir) {
    $expressions = Get-ChildItem -Path "$rootExprDir\*.txt" -File | Where-Object { $_.DirectoryName -eq (Resolve-Path $rootExprDir).Path } | Sort-Object Name | ForEach-Object { $_.Name }
}

# Collect edge-case expression files if directory exists
$edgeCaseDir = Join-Path $rootExprDir "edge_cases"
$edgeCaseExpressions = @()
if (Test-Path $edgeCaseDir) {
    $edgeCaseExpressions = Get-ChildItem -Path "$edgeCaseDir\*.txt" -File | Sort-Object Name | ForEach-Object { "edge_cases\" + $_.Name }
}

Write-Host "Regenerating reference analysis files for method: $Method" -ForegroundColor Green

function Update-ReferenceFiles {
    param(
        [string]$MethodName,
        [string]$MethodFlag,
        [string]$OutputDir
    )

    Write-Host "`n--- Processing $MethodName method ---" -ForegroundColor Cyan

    # Main expressions
    foreach ($expr in $expressions) {
        Write-Host "Processing: $expr" -ForegroundColor Yellow
        $exprPath = "test_expressions\$expr"

        if (Test-Path $exprPath) {
            & ".\build\bin\Release\bdd_demo.exe" --mermaid $MethodFlag $exprPath

            $baseName = [System.IO.Path]::GetFileNameWithoutExtension($expr)
            $sourceFile = "test_expressions\${baseName}_analysis.md"
            $destFile = "test_expressions\reference_output\${baseName}_analysis.md"

            if (Test-Path $sourceFile) {
                Copy-Item $sourceFile $destFile -Force
                Write-Host "  ✓ Updated: $destFile" -ForegroundColor Green
            } else {
                Write-Host "  ✗ Source file not found: $sourceFile" -ForegroundColor Red
            }

            # Also copy generated DOT files for visualization into expected_output
            $exprDot = "test_expressions\${baseName}_expression_tree.dot"
            $bddDot = "test_expressions\${baseName}_bdd.dot"
            $exprDotDest = "test_expressions\expected_output\${baseName}_expression_tree.dot"
            $bddDotDest = "test_expressions\expected_output\${baseName}_bdd.dot"

            if (Test-Path $exprDot) {
                Copy-Item $exprDot $exprDotDest -Force
                Write-Host "  ✓ Updated DOT: $exprDotDest" -ForegroundColor Green
            } else {
                Write-Host "  ✗ DOT not found: $exprDot" -ForegroundColor Yellow
            }

            if (Test-Path $bddDot) {
                Copy-Item $bddDot $bddDotDest -Force
                Write-Host "  ✓ Updated BDD DOT: $bddDotDest" -ForegroundColor Green
            } else {
                Write-Host "  ✗ BDD DOT not found: $bddDot" -ForegroundColor Yellow
            }

            # Generate reordered outputs using --force-reorder and copy DOTs to reordered dir
            Write-Host "  -> Generating reordered version (force reorder)" -ForegroundColor Cyan
            & ".\build\bin\Release\bdd_demo.exe" --mermaid $MethodFlag --force-reorder $exprPath

            $reorderedExprDot = "test_expressions\${baseName}_expression_tree.dot"
            $reorderedBddDot = "test_expressions\${baseName}_bdd.dot"
            $reorderedExprDotDest = "test_expressions\reordered\${baseName}_expression_tree.dot"
            $reorderedBddDotDest = "test_expressions\reordered\${baseName}_bdd.dot"

            if (Test-Path $reorderedExprDot) {
                Copy-Item $reorderedExprDot $reorderedExprDotDest -Force
                Write-Host "  ✓ Reordered DOT: $reorderedExprDotDest" -ForegroundColor Green
            } else {
                Write-Host "  ✗ Reordered DOT not found: $reorderedExprDot" -ForegroundColor Yellow
            }

            if (Test-Path $reorderedBddDot) {
                Copy-Item $reorderedBddDot $reorderedBddDotDest -Force
                Write-Host "  ✓ Reordered BDD DOT: $reorderedBddDotDest" -ForegroundColor Green
            } else {
                Write-Host "  ✗ Reordered BDD DOT not found: $reorderedBddDot" -ForegroundColor Yellow
            }
        } else {
            Write-Host "  ✗ Expression file not found: $exprPath" -ForegroundColor Red
        }
    }

    # Edge case expressions
    foreach ($expr in $edgeCaseExpressions) {
        Write-Host "Processing edge case: $expr" -ForegroundColor Yellow
        $exprPath = "test_expressions\$expr"

        if (Test-Path $exprPath) {
            & ".\build\bin\Release\bdd_demo.exe" --mermaid $MethodFlag $exprPath

            $baseName = [System.IO.Path]::GetFileNameWithoutExtension($expr)
            # Edge case files are generated in the edge_cases subdirectory
            $sourceFile = "test_expressions\edge_cases\${baseName}_analysis.md"
            $destFile = "test_expressions\reference_output\${baseName}_analysis.md"

            if (Test-Path $sourceFile) {
                Copy-Item $sourceFile $destFile -Force
                Write-Host "  ✓ Updated: $destFile" -ForegroundColor Green
            } else {
                Write-Host "  ✗ Source file not found: $sourceFile" -ForegroundColor Red
            }

            # Also copy generated DOT files for edge case visualization into expected_output
            $exprDot = "test_expressions\edge_cases\${baseName}_expression_tree.dot"
            $bddDot = "test_expressions\edge_cases\${baseName}_bdd.dot"
            $exprDotDest = "test_expressions\expected_output\${baseName}_expression_tree.dot"
            $bddDotDest = "test_expressions\expected_output\${baseName}_bdd.dot"

            if (Test-Path $exprDot) {
                Copy-Item $exprDot $exprDotDest -Force
                Write-Host "  ✓ Updated DOT: $exprDotDest" -ForegroundColor Green
            } else {
                Write-Host "  ✗ DOT not found: $exprDot" -ForegroundColor Yellow
            }

            if (Test-Path $bddDot) {
                Copy-Item $bddDot $bddDotDest -Force
                Write-Host "  ✓ Updated BDD DOT: $bddDotDest" -ForegroundColor Green
            } else {
                Write-Host "  ✗ BDD DOT not found: $bddDot" -ForegroundColor Yellow
            }

            # Generate reordered outputs using --force-reorder for edge-case and copy DOTs to reordered dir
            Write-Host "  -> Generating reordered version (force reorder) for edge case" -ForegroundColor Cyan
            & ".\build\bin\Release\bdd_demo.exe" --mermaid $MethodFlag --force-reorder $exprPath

            $reorderedExprDot = "test_expressions\edge_cases\${baseName}_expression_tree.dot"
            $reorderedBddDot = "test_expressions\edge_cases\${baseName}_bdd.dot"
            $reorderedExprDotDest = "test_expressions\reordered\${baseName}_expression_tree.dot"
            $reorderedBddDotDest = "test_expressions\reordered\${baseName}_bdd.dot"

            if (Test-Path $reorderedExprDot) {
                Copy-Item $reorderedExprDot $reorderedExprDotDest -Force
                Write-Host "  ✓ Reordered DOT: $reorderedExprDotDest" -ForegroundColor Green
            } else {
                Write-Host "  ✗ Reordered DOT not found: $reorderedExprDot" -ForegroundColor Yellow
            }

            if (Test-Path $reorderedBddDot) {
                Copy-Item $reorderedBddDot $reorderedBddDotDest -Force
                Write-Host "  ✓ Reordered BDD DOT: $reorderedBddDotDest" -ForegroundColor Green
            } else {
                Write-Host "  ✗ Reordered BDD DOT not found: $reorderedBddDot" -ForegroundColor Yellow
            }
        } else {
            Write-Host "  ✗ Expression file not found: $exprPath" -ForegroundColor Red
        }
    }
}

# Process based on selected method
# TeDDy outputs are the 'normal' expected outputs
if ($Method -eq "teddy" -or $Method -eq "both") {
    Update-ReferenceFiles -MethodName "TeDDy" -MethodFlag "--method=teddy" -OutputDir "test_expressions\expected_output"
}

# CUDD outputs are considered the 'reordered' reference outputs
if ($Method -eq "cudd" -or $Method -eq "both") {
    Update-ReferenceFiles -MethodName "CUDD" -MethodFlag "--method=cudd" -OutputDir "test_expressions\reordered"
}

Write-Host "`n✅ Reference file regeneration complete for method: $Method!" -ForegroundColor Green
Write-Host "All Mermaid diagrams now include unified format with CSS definitions" -ForegroundColor Cyan
