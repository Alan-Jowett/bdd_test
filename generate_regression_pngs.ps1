# SPDX-License-Identifier: MIT# SPDX-License-Identifier: MIT

# SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett# SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett



# PowerShell script to generate PNG visualizations for all regression test DOT files# PowerShell script to generate PNG visualizations for all regression test DOT files



Write-Host "Generating PNG visualizations for regression test DOT files..."Write-Host "Generating PNG visualizations for regression test DOT files..."



# Check if dot command is available# Check if dot command is available

try {try {

    $null = Get-Command dot -ErrorAction Stop    $null = Get-Command dot -ErrorAction Stop

    Write-Host "Graphviz 'dot' command found."    Write-Host "Graphviz 'dot' command found."

} catch {} catch {

    Write-Error "Graphviz 'dot' command not found. Please install Graphviz."    Write-Error "Graphviz 'dot' command not found. Please install Graphviz."

    exit 1    exit 1

}}



# Generate PNGs for default_ordering reference files# Generate PNGs for default_ordering reference files

Write-Host "Generating PNGs for default_ordering reference files..."Write-Host "Generating PNGs for default_ordering reference files..."

Set-Location test_expressions/default_ordering/Set-Location test_expressions/default_ordering/



Get-ChildItem "*_regression_*.dot" | ForEach-Object {Get-ChildItem "*_regression_*.dot" | ForEach-Object {

    $dotFile = $_.Name    $dotFile = $_.Name

    $pngFile = $_.BaseName + ".png"    $pngFile = $_.BaseName + ".png"

    Write-Host "  $dotFile -> $pngFile"    Write-Host "  $dotFile -> $pngFile"

    & dot -Tpng $dotFile -o $pngFile    & dot -Tpng $dotFile -o $pngFile

}}



# Generate PNGs for edge_cases test files# Generate PNGs for edge_cases test files

Write-Host "Generating PNGs for edge_cases test files..."Write-Host "Generating PNGs for edge_cases test files..."

Set-Location ../edge_cases/Set-Location ../edge_cases/



Get-ChildItem "*_regression_*.dot" | ForEach-Object {Get-ChildItem "*_regression_*.dot" | ForEach-Object {

    $dotFile = $_.Name    $dotFile = $_.Name

    $pngFile = $_.BaseName + ".png"    $pngFile = $_.BaseName + ".png"

    Write-Host "  $dotFile -> $pngFile"    Write-Host "  $dotFile -> $pngFile"

    & dot -Tpng $dotFile -o $pngFile    & dot -Tpng $dotFile -o $pngFile

}}



Set-Location ../../Set-Location ../../



Write-Host "PNG generation complete!"Write-Host "PNG generation complete!"
Write-Host ""
Write-Host "Generated visualizations for the following regression tests:"
Write-Host "- boundary_detection_regression: Tests variable names containing operator keywords"
Write-Host "- complex_boundary_regression: Tests complex variable naming with numbers and operators"
Write-Host "- operator_keywords_in_vars: Tests operator keywords at various positions in variables"
Write-Host "- unicode_chars_regression: Tests extended ASCII character handling"
Write-Host ""
Write-Host "View the PNG files to visualize the BDD structures and expression trees."