#!/bin/bash

# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

# Script to run sanitizer builds and tests locally
# Usage: ./scripts/run-sanitizers.sh [asan|ubsan|asan+ubsan|all]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_BASE_DIR="$PROJECT_ROOT/build"

# Function to print colored output
print_info() {
    echo -e "\e[34m[INFO]\e[0m $1"
}

print_success() {
    echo -e "\e[32m[SUCCESS]\e[0m $1"
}

print_error() {
    echo -e "\e[31m[ERROR]\e[0m $1"
}

print_warning() {
    echo -e "\e[33m[WARNING]\e[0m $1"
}

# Function to run a specific sanitizer build
run_sanitizer_build() {
    local sanitizer="$1"
    local compiler="${2:-gcc}"

    print_info "Running $sanitizer build with $compiler"

    # Set up build directory
    local build_dir="$BUILD_BASE_DIR/sanitizer-$sanitizer-$compiler"
    rm -rf "$build_dir"
    mkdir -p "$build_dir"

    # Configure environment
    if [ "$compiler" = "gcc" ]; then
        export CC=gcc
        export CXX=g++
    elif [ "$compiler" = "clang" ]; then
        export CC=clang
        export CXX=clang++
    fi

    # Set sanitizer flags
    local cmake_flags=""
    local asan_options=""
    local ubsan_options=""

    case "$sanitizer" in
        "asan")
            cmake_flags="-DENABLE_ASAN=ON"
            asan_options="detect_leaks=1:abort_on_error=1:halt_on_error=1:check_initialization_order=1"
            ;;
        "ubsan")
            cmake_flags="-DENABLE_UBSAN=ON"
            ubsan_options="print_stacktrace=1:abort_on_error=1:halt_on_error=1"
            ;;
        "asan+ubsan")
            cmake_flags="-DENABLE_ASAN=ON -DENABLE_UBSAN=ON"
            asan_options="detect_leaks=1:abort_on_error=1:halt_on_error=1:check_initialization_order=1"
            ubsan_options="print_stacktrace=1:abort_on_error=1:halt_on_error=1"
            ;;
        *)
            print_error "Unknown sanitizer: $sanitizer"
            return 1
            ;;
    esac

    # Configure CMake
    print_info "Configuring CMake with sanitizer flags..."
    cd "$build_dir"
    cmake "$PROJECT_ROOT" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_C_COMPILER="$CC" \
        -DCMAKE_CXX_COMPILER="$CXX" \
        $cmake_flags

    # Build
    print_info "Building with sanitizers..."
    local jobs
    jobs="$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)"
    cmake --build . --config Debug --parallel "$jobs"

    # Set runtime environment for tests
    if [ -n "$asan_options" ]; then
        export ASAN_OPTIONS="$asan_options"
        print_info "ASAN_OPTIONS: $ASAN_OPTIONS"
    fi

    if [ -n "$ubsan_options" ]; then
        export UBSAN_OPTIONS="$ubsan_options"
        print_info "UBSAN_OPTIONS: $UBSAN_OPTIONS"
    fi

    # Run tests
    print_info "Running tests with sanitizers..."
    if ctest -C Debug --output-on-failure --verbose --timeout 300; then
        print_success "$sanitizer tests passed with $compiler!"
    else
        print_error "$sanitizer tests failed with $compiler!"
        return 1
    fi

    # Check for any sanitizer reports
    local reports_found=false
    while IFS= read -r file; do
        print_warning "Found sanitizer report: $file"
        cat "$file" || true
        reports_found=true
    done < <(find "$build_dir" \( -name "*.asan" -o -name "*.msan" -o -name "*.tsan" -o -name "*.ubsan" -o -name "core.*" \))

    if [ "$reports_found" = "false" ]; then
        print_success "No sanitizer reports found"
    fi

    cd "$PROJECT_ROOT"
}

# Main function
main() {
    local mode="${1:-all}"
    local compiler="${2:-gcc}"

    print_info "BDD Demo Sanitizer Test Runner"
    print_info "==============================="

    # Check if compiler is available
    if ! command -v "$compiler" >/dev/null 2>&1; then
        print_error "Compiler $compiler is not available"
        print_info "Available compilers:"
        command -v gcc >/dev/null 2>&1 && echo "  - gcc"
        command -v clang >/dev/null 2>&1 && echo "  - clang"
        exit 1
    fi

    case "$mode" in
        "asan")
            run_sanitizer_build "asan" "$compiler"
            ;;
        "ubsan")
            run_sanitizer_build "ubsan" "$compiler"
            ;;
        "asan+ubsan")
            run_sanitizer_build "asan+ubsan" "$compiler"
            ;;
        "all")
            print_info "Running all sanitizer combinations with $compiler"
            run_sanitizer_build "asan" "$compiler"
            run_sanitizer_build "ubsan" "$compiler"
            run_sanitizer_build "asan+ubsan" "$compiler"
            ;;
        *)
            print_error "Usage: $0 [asan|ubsan|asan+ubsan|all] [gcc|clang]"
            print_error "Examples:"
            print_error "  $0 asan gcc      # Run AddressSanitizer with GCC"
            print_error "  $0 ubsan clang   # Run UBSan with Clang"
            print_error "  $0 all gcc       # Run all sanitizers with GCC"
            print_error "  $0               # Run all sanitizers with GCC (default)"
            exit 1
            ;;
    esac

    print_success "All requested sanitizer builds completed!"
}

# Run main function with all arguments
main "$@"
