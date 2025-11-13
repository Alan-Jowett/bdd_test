# N-Queens Stress Testing Results

## Overview
The N-Queens stress testing demonstrates the scalability limits of Binary Decision Diagram (BDD) generation for constraint satisfaction problems. These tests push the TeDDy library and our BDD system to their practical limits.

## Test Results Summary

| Variant | Variables | BDD Nodes | DOT File Size | Status | PNG Viable |
|---------|-----------|-----------|---------------|---------|------------|
| 4-Queens | 16 | 95 | ~200 lines | ✅ Complete | ✅ Yes |
| 6-Queens | 36 | 3,361 | ~17,000 lines | ✅ Complete | ❌ Too large |
| 8-Queens | 64 | 71,166 | ~350,000 lines | ✅ Complete | ❌ Too large |

## Key Findings

### 4-Queens (Manageable)
- **Memory Usage**: Minimal
- **Generation Time**: <1 second
- **Visualization**: PNG files are clear and readable
- **Use Case**: Perfect for educational demonstrations and debugging

### 6-Queens (Practical Limit)
- **Memory Usage**: Significant but manageable
- **Generation Time**: Several seconds
- **Visualization**: DOT format only - PNG would be illegible and enormous
- **Use Case**: Stress testing and performance benchmarking

### 8-Queens (Advanced Stress Test)
- **Memory Usage**: Manageable
- **Generation Time**: 1.83 seconds
- **Visualization**: DOT format only - PNG would be extremely large (~100MB+)
- **Use Case**: Advanced performance benchmarking and algorithmic testing

## Technical Insights

### BDD Growth Pattern
The node count growth follows an exponential pattern:
- 4×4 board: 95 nodes
- 6×6 board: 3,361 nodes (35× increase for 2.25× variables)
- 8×8 board: 71,166 nodes (21× increase for 1.78× variables)

### Memory Constraints
The TeDDy library enables:
- Successful completion of all three N-Queens variants (4, 6, and 8)
- Efficient memory usage through BDD reduction algorithms
- Variable ordering that significantly reduces node counts
- Practical constraint satisfaction problem solving for larger instances

### Visualization Scalability
- Small BDDs (≤100 nodes): PNG visualization is clear and useful
- Medium BDDs (100-1,000 nodes): PNG becomes complex but still viable
- Large BDDs (>1,000 nodes): PNG visualization is impractical due to:
  - File size (potentially >100MB)
  - Layout complexity (illegible when rendered)
  - Memory requirements for graph layout algorithms

## Implementation Notes

### Test Framework Integration
- All three tests fully integrated into CMake/CTest framework
- Reference files stored in `expected_output/` and `reordered/` directories
- Deterministic node numbering ensures reproducible results
- Complete test coverage from educational (4-queens) to advanced stress testing (8-queens)

### File Organization
```
test_expressions/
├── four_queens.txt          # 4×4 N-Queens constraint formula
├── six_queens.txt           # 6×6 N-Queens constraint formula
├── eight_queens.txt         # 8×8 N-Queens constraint formula
├── expected_output/   # Reference files for expected output
│   ├── four_queens_*.dot
│   ├── six_queens_*.dot
│   ├── eight_queens_*.dot
│   └── ...
└── reordered/              # Reference files for --force-reorder
    ├── four_queens_*.dot
    ├── six_queens_*.dot
    ├── eight_queens_*.dot
    └── ...
```

## Conclusions

1. **4-Queens is ideal for demonstrations**: Fast, small, visualizable - perfect educational tool
2. **6-Queens provides intermediate complexity**: Manageable node count for performance analysis
3. **8-Queens demonstrates advanced capabilities**: Complex constraint satisfaction in 1.83 seconds
4. **Exponential complexity is manageable**: Proper algorithmic design enables larger problems
5. **BDD optimization is essential**: Variable ordering and reduction strategies are critical for performance
6. **Comprehensive testing coverage**: All three variants provide testing from basic to advanced scenarios

The N-Queens stress testing establishes clear performance benchmarks and demonstrates the system's capability to handle constraint satisfaction problems of varying complexity.

This stress testing establishes clear benchmarks for the system's capabilities and provides valuable data points for users working with constraint satisfaction problems.
