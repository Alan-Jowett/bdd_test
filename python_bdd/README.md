# Python BDD Implementation

This folder contains a Python-based equivalent of the C++ `bdd_demo.exe` that produces identical output formats for comparison with TeDDy and CUDD implementations.

## 📁 **Folder Structure**

```
python_bdd/
├── README.md                      # This file - Python BDD overview
├── bdd_demo_python.py            # Main Python BDD demo script
├── requirements.txt              # Python dependencies (dd library)
├── setup_python_bdd.py          # Automated setup and verification
├── PYTHON_BDD_INTEGRATION.md    # Detailed integration guide
└── PYTHON_BDD_SUCCESS_SUMMARY.md # Implementation success summary
```

## 🚀 **Quick Start**

### 1. Setup Dependencies
```bash
# Navigate to Python BDD folder
cd python_bdd

# Install dependencies
python setup_python_bdd.py
# OR manually: pip install -r requirements.txt
```

### 2. Run Python BDD Demo
```bash
# Basic usage
python bdd_demo_python.py ../test_expressions/simple_expression.txt

# Verbose output (shows node table)
python bdd_demo_python.py ../test_expressions/simple_expression.txt --verbose

# Test with all operators
python bdd_demo_python.py ../test_expressions/all_operators.txt --verbose
```

### 3. Enable CMake Integration
```bash
# From project root directory
cd ..
cmake -DENABLE_PYTHON_BDD=ON -B build -S .
ctest -C Release -L python
```

## 🎯 **Purpose**

This Python implementation serves as a **canonical reference** for cross-library BDD validation:

- **✅ Trusted Reference**: Uses the academically-backed `dd` library
- **✅ Canonical ROBDDs**: Produces proper reduced, ordered BDDs
- **✅ Identical Output**: Same formats as C++ implementation
- **✅ Cross-Validation**: Compare TeDDy vs CUDD vs Python

## 🔬 **Cross-Library Validation**

The Python implementation enables comprehensive BDD verification:

**Example: `(a AND b) OR (c AND d)`**
- **✅ TeDDy**: 6 nodes (canonical)
- **✅ CUDD**: 6 nodes (canonical)
- **✅ Python**: 6 nodes (canonical reference)

## 📖 **Documentation**

- **[PYTHON_BDD_INTEGRATION.md](PYTHON_BDD_INTEGRATION.md)** - Comprehensive integration guide
- **[PYTHON_BDD_SUCCESS_SUMMARY.md](PYTHON_BDD_SUCCESS_SUMMARY.md)** - Implementation details and results

## 🔧 **Key Features**

- **Identical Interface**: Same command-line options as `bdd_demo.exe`
- **Compatible Output**: Node tables and DOT graphs match C++ format exactly
- **Expression Parsing**: Supports AND, OR, XOR, NOT with parentheses
- **CMake Integration**: Automated testing via `ENABLE_PYTHON_BDD=ON`
- **Cross-Platform**: Works on Windows, macOS, and Linux

## ⚡ **Usage Examples**

### Compare All Three Implementations
```bash
# From project root
./build/bin/Release/bdd_demo.exe test_expressions/simple_expression.txt --method=teddy --verbose
./build/bin/Release/bdd_demo.exe test_expressions/simple_expression.txt --method=cudd --verbose
python python_bdd/bdd_demo_python.py test_expressions/simple_expression.txt --verbose
```

### Validate CUDD Fixes
```bash
# Test multiple expressions
for expr in test_expressions/*.txt; do
    echo "Testing: $expr"
    python python_bdd/bdd_demo_python.py "$expr" > python_output.txt
    ./build/bin/Release/bdd_demo.exe "$expr" --method=cudd > cudd_output.txt
    # Compare node counts and structures
done
```

---

**Related Issues:**
- [CUDD Canonical Output Fix](../docs/CUDD_CANONICAL_OUTPUT_FIX.md) - Documents resolution of previous canonicality issues
- Original request for Python BDD comparison tool

**Dependencies:**
- Python 3.8+
- `dd` library (Decision Diagrams)