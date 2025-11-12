# PowerShell script to regenerate all reference analysis files with updated styling
# This ensures all reference files have the dark-mode friendly text colors

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

Write-Host "Regenerating reference analysis files with updated dark-mode styling..." -ForegroundColor Green

# Main expressions
foreach ($expr in $expressions) {
    Write-Host "Processing: $expr" -ForegroundColor Yellow
    $exprPath = "test_expressions\$expr"

    if (Test-Path $exprPath) {
        & ".\build\bin\Release\bdd_demo.exe" --mermaid $exprPath

        $baseName = [System.IO.Path]::GetFileNameWithoutExtension($expr)
        $sourceFile = "test_expressions\${baseName}_analysis.md"
        $destFile = "test_expressions\teddy_reference_outputs\${baseName}_analysis.md"

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
        & ".\build\bin\Release\bdd_demo.exe" --mermaid $exprPath

        $baseName = [System.IO.Path]::GetFileNameWithoutExtension($expr)
        $sourceFile = "test_expressions\${baseName}_analysis.md"
        $destFile = "test_expressions\teddy_reference_outputs\${baseName}_analysis.md"

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

Write-Host "✅ Reference file regeneration complete!" -ForegroundColor Green
Write-Host "All Mermaid diagrams now include dark-mode friendly text colors (color:#000)" -ForegroundColor Cyan
