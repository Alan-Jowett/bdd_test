# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

# PowerShell script to regenerate all reference analysis files for TeDDy and CUDD methods
# This ensures all reference files have the unified Mermaid format

param(
    [Parameter(Position=0)]
    [ValidateSet("teddy", "cudd", "both")]
    [string]$Method = "both"
)

$expressions = @(
    "all_operators.txt",
    "deeply_nested.txt",
    "filter_expression.txt",
    "many_variables.txt",
    "multiple_not.txt",
    "my_test_expression.txt",
    "same_variable.txt",
    "simple_expression.txt",
    "single_variable.txt",
    "single_variable_unusual_name.txt",
    "test_subdir_expression.txt",
    "underscore_vars.txt",
    "xor_chain.txt"
)

$edgeCaseExpressions = @(
    "edge_cases\boundary_detection_regression.txt",
    "edge_cases\complex_boundary_regression.txt",
    "edge_cases\operator_keywords_in_vars.txt",
    "edge_cases\unicode_chars_regression.txt"
)

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
            $destFile = "$OutputDir\${baseName}_analysis.md"

            if (Test-Path $sourceFile) {
                Copy-Item $sourceFile $destFile -Force
                Write-Host "  ✓ Updated: $destFile" -ForegroundColor Green
            } else {
                Write-Host "  ✗ Source file not found: $sourceFile" -ForegroundColor Red
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
            $destFile = "$OutputDir\${baseName}_analysis.md"

            if (Test-Path $sourceFile) {
                Copy-Item $sourceFile $destFile -Force
                Write-Host "  ✓ Updated: $destFile" -ForegroundColor Green
            } else {
                Write-Host "  ✗ Source file not found: $sourceFile" -ForegroundColor Red
            }
        } else {
            Write-Host "  ✗ Expression file not found: $exprPath" -ForegroundColor Red
        }
    }
}

# Process based on selected method - now using unified reference output directory
if ($Method -eq "teddy" -or $Method -eq "both") {
    Update-ReferenceFiles -MethodName "TeDDy" -MethodFlag "--method=teddy" -OutputDir "test_expressions\reference_output"
}

if ($Method -eq "cudd" -or $Method -eq "both") {
    Update-ReferenceFiles -MethodName "CUDD" -MethodFlag "--method=cudd" -OutputDir "test_expressions\reference_output"
}

Write-Host "`n✅ Reference file regeneration complete for method: $Method!" -ForegroundColor Green
Write-Host "All Mermaid diagrams now include unified format with CSS definitions" -ForegroundColor Cyan
