#!/bin/bash
# Verification script for CUDD canonical BDD output
# This script verifies that CUDD produces deterministic, canonical output

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BDD_DEMO="${REPO_ROOT}/build/bin/bdd_demo"
TEST_EXPRESSIONS="${REPO_ROOT}/test_expressions"

# Color codes for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo "========================================"
echo "CUDD Canonical Output Verification"
echo "========================================"
echo ""

# Test expressions
TEST_FILES=(
    "simple_expression.txt"
    "all_operators.txt"
    "filter_expression.txt"
)

all_passed=true

for test_file in "${TEST_FILES[@]}"; do
    echo "Testing: ${test_file}"

    full_path="${TEST_EXPRESSIONS}/${test_file}"

    if [ ! -f "$full_path" ]; then
        echo -e "${RED}✗ Test file not found: ${full_path}${NC}"
        all_passed=false
        continue
    fi

    # Generate BDD 3 times and check MD5 hashes
    hashes=()
    for i in 1 2 3; do
        "${BDD_DEMO}" "${full_path}" --method=cudd > /dev/null 2>&1
        base_name="${full_path%.txt}"
        hash=$(md5sum "${base_name}_bdd.dot" "${base_name}_bdd_nodes.txt" | md5sum | cut -d' ' -f1)
        hashes+=("$hash")
    done

    # Check if all hashes are identical
    if [ "${hashes[0]}" = "${hashes[1]}" ] && [ "${hashes[1]}" = "${hashes[2]}" ]; then
        echo -e "${GREEN}✓ PASS: Output is deterministic (hash: ${hashes[0]})${NC}"
    else
        echo -e "${RED}✗ FAIL: Output is non-deterministic${NC}"
        echo "  Hash 1: ${hashes[0]}"
        echo "  Hash 2: ${hashes[1]}"
        echo "  Hash 3: ${hashes[2]}"
        all_passed=false
    fi

    echo ""
done

echo "========================================"
if [ "$all_passed" = true ]; then
    echo -e "${GREEN}All tests PASSED${NC}"
    echo "CUDD produces canonical, deterministic output"
    exit 0
else
    echo -e "${RED}Some tests FAILED${NC}"
    echo "CUDD output may not be canonical"
    exit 1
fi
