# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

<#
.SYNOPSIS
    Script to run sanitizer builds and tests locally on Windows with WSL

.DESCRIPTION
    This script sets up and runs AddressSanitizer (ASan) and UndefinedBehaviorSanitizer (UBSan)
    builds using Windows Subsystem for Linux (WSL). Note that sanitizers are primarily
    supported on Linux, so this script requires WSL to be installed.

.PARAMETER Sanitizer
    The sanitizer to run: asan, ubsan, asan+ubsan, or all (default: all)

.PARAMETER Compiler
    The compiler to use: gcc or clang (default: gcc)

.EXAMPLE
    .\scripts\run-sanitizers.ps1
    Runs all sanitizer combinations with GCC

.EXAMPLE
    .\scripts\run-sanitizers.ps1 -Sanitizer asan -Compiler clang
    Runs AddressSanitizer with Clang

.NOTES
    Requires Windows Subsystem for Linux (WSL) to be installed and configured.
    Sanitizers are not natively supported on Windows MSVC builds.
#>

param(
    [ValidateSet("asan", "ubsan", "asan+ubsan", "all")]
    [string]$Sanitizer = "all",

    [ValidateSet("gcc", "clang")]
    [string]$Compiler = "gcc"
)

function Write-Info {
    param([string]$Message)
    Write-Host "[INFO] $Message" -ForegroundColor Blue
}

function Write-Success {
    param([string]$Message)
    Write-Host "[SUCCESS] $Message" -ForegroundColor Green
}

function Write-Error {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor Red
}

function Write-Warning {
    param([string]$Message)
    Write-Host "[WARNING] $Message" -ForegroundColor Yellow
}

function Test-WSL {
    try {
        wsl --list --quiet 2>&1 | Out-Null
        if ($LASTEXITCODE -eq 0) {
            return $true
        }
    }
    catch {
        return $false
    }
    return $false
}

function Test-LinuxCommand {
    param([string]$Command)

    try {
        wsl bash -c "command -v $Command" 2>$null | Out-Null
        return ($LASTEXITCODE -eq 0)
    }
    catch {
        return $false
    }
}

# Main script
Write-Info "BDD Demo Sanitizer Test Runner (Windows WSL)"
Write-Info "============================================="

# Check if WSL is available
if (-not (Test-WSL)) {
    Write-Error "Windows Subsystem for Linux (WSL) is not installed or configured."
    Write-Info "To install WSL, run the following commands as Administrator:"
    Write-Info "  wsl --install"
    Write-Info "  wsl --install -d Ubuntu"
    Write-Info ""
    Write-Info "After installation, you'll need to install build tools in WSL:"
    Write-Info "  wsl sudo apt-get update"
    Write-Info "  wsl sudo apt-get install -y build-essential cmake gcc g++ clang"
    exit 1
}

Write-Success "WSL is available"

# Check if required tools are installed in WSL
$missingTools = @()

if (-not (Test-LinuxCommand "cmake")) {
    $missingTools += "cmake"
}

if ($Compiler -eq "gcc") {
    if (-not (Test-LinuxCommand "gcc") -or -not (Test-LinuxCommand "g++")) {
        $missingTools += "gcc/g++"
    }
} elseif ($Compiler -eq "clang") {
    if (-not (Test-LinuxCommand "clang") -or -not (Test-LinuxCommand "clang++")) {
        $missingTools += "clang/clang++"
    }
}

if ($missingTools.Count -gt 0) {
    Write-Error "Missing required tools in WSL: $($missingTools -join ', ')"
    Write-Info "Install them with:"
    Write-Info "  wsl sudo apt-get update"
    Write-Info "  wsl sudo apt-get install -y build-essential cmake gcc g++ clang"
    exit 1
}

Write-Success "Required build tools are available in WSL"

# Get the project root directory
$projectRoot = Split-Path $PSScriptRoot -Parent

# Convert Windows path to WSL path
$wslProjectRoot = wsl wslpath "'$projectRoot'"

if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to convert Windows path to WSL path"
    exit 1
}

Write-Info "Project root (WSL): $wslProjectRoot"

# Check if the sanitizer script exists
$sanitizerScript = Join-Path $PSScriptRoot "run-sanitizers.sh"
if (-not (Test-Path $sanitizerScript)) {
    Write-Error "Sanitizer script not found: $sanitizerScript"
    exit 1
}

# Convert script path to WSL
$wslScriptPath = wsl wslpath "'$sanitizerScript'"

# Make sure the script is executable
wsl chmod +x "'$wslScriptPath'"

# Run the sanitizer script in WSL
Write-Info "Running sanitizer tests in WSL..."
Write-Info "Sanitizer: $Sanitizer"
Write-Info "Compiler: $Compiler"
Write-Info ""

# Change to project directory in WSL and run the script
$wslCommand = "cd '$wslProjectRoot' && '$wslScriptPath' '$Sanitizer' '$Compiler'"

try {
    wsl bash -c $wslCommand

    if ($LASTEXITCODE -eq 0) {
        Write-Success "Sanitizer tests completed successfully!"
    } else {
        Write-Error "Sanitizer tests failed with exit code $LASTEXITCODE"
        exit $LASTEXITCODE
    }
} catch {
    Write-Error "Failed to run sanitizer tests: $($_.Exception.Message)"
    exit 1
}

Write-Info ""
Write-Info "Note: Sanitizer builds are only available on Linux. This script uses WSL to provide"
Write-Info "sanitizer functionality on Windows. For native Windows testing, use the regular"
Write-Info "build process without sanitizers."
