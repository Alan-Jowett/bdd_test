# TeDDy Library Information

## About TeDDy

TeDDy is a C++ library for creation and manipulation of decision diagrams developed at the University of Žilina. It supports:

- Binary Decision Diagrams (BDDs)
- Multi-Valued Decision Diagrams (MDDs)
- Integer Multi-Valued Decision Diagrams (iMDDs)
- Reliability analysis functions
- High-performance algorithms for large-scale problems

## Version Information

- **Version Used**: v4.1.0
- **Repository**: https://github.com/MichalMrena/DecisionDiagrams
- **License**: See the TeDDy repository for license details
- **Integration**: Automatically fetched via CMake FetchContent

## Key Features

### Binary Decision Diagrams (BDDs)
- Efficient representation of boolean functions
- Automatic reduction and optimization
- Support for various boolean operators
- Variable ordering optimization

### Performance Characteristics
- Memory-efficient shared node structure
- Fast boolean operations (AND, OR, NOT, XOR)
- Optimized algorithms for large-scale problems
- Cross-platform compatibility

### Integration with this Project
- Seamless CMake integration via FetchContent
- No manual installation required
- Automatic dependency resolution
- Version-locked for reproducible builds

## TeDDy vs Other BDD Libraries

### Advantages of TeDDy
- **Modern C++**: Uses C++17 features and STL containers (our project builds on this with C++20)
- **Template-Based**: Flexible and type-safe implementation
- **Educational**: Well-documented with clear examples
- **Lightweight**: Minimal external dependencies

### Use Cases
- **Logic Synthesis**: Boolean function manipulation and optimization
- **Model Checking**: State space representation and analysis
- **Reliability Analysis**: System reliability calculations
- **Decision Making**: Complex boolean decision modeling

## Academic Background

TeDDy was developed as part of research at the University of Žilina, Slovakia, focusing on:
- Decision diagram algorithms and data structures
- Reliability engineering applications
- Performance optimization for large-scale problems
- Educational tools for computer science curricula

## Attribution

When using this BDD Expression Converter in academic work, consider citing:
- The TeDDy library and its contributors
- The University of Žilina for the underlying algorithms
- This project for the expression parsing and visualization components

For detailed TeDDy documentation and examples, visit the official repository at:
https://github.com/MichalMrena/DecisionDiagrams