# BDD Analysis Report

## Original Expression

```
a XOR b XOR c XOR d XOR e
```

## Expression Tree

The following diagram shows the parse tree of the logical expression:

```mermaid
---
title: Expression Tree
---
flowchart TD
    N1(("a"))
    N2(("b"))
    N3["XOR"]
    N4(("c"))
    N5["XOR"]
    N6(("d"))
    N7["XOR"]
    N8(("e"))
    N9["XOR"]

    N3 --> N1
    N3 --> N2
    N5 --> N3
    N5 --> N4
    N7 --> N5
    N7 --> N6
    N9 --> N7
    N9 --> N8

    class N1 variable
    class N2 variable
    class N3 xorOp
    class N4 variable
    class N5 xorOp
    class N6 variable
    class N7 xorOp
    class N8 variable
    class N9 xorOp

    classDef variable fill:lightblue,stroke:#333,stroke-width:2px,color:#000
    classDef andOp fill:lightgreen,stroke:#333,stroke-width:2px,color:#000
    classDef orOp fill:lightcoral,stroke:#333,stroke-width:2px,color:#000
    classDef notOp fill:yellow,stroke:#333,stroke-width:2px,color:#000
    classDef xorOp fill:lightpink,stroke:#333,stroke-width:2px,color:#000
```

## Binary Decision Diagram (BDD)

The following diagram shows the optimized BDD representation:

```mermaid
---
title: BDD
---
flowchart TD
    N0["0"]
    N1["1"]
    N2(("e"))
    N3(("e"))
    N4(("d"))
    N5(("d"))
    N6(("c"))
    N7(("c"))
    N8(("b"))
    N9(("b"))
    N10(("a"))

    N2 -.-> N0
    N2 --> N1
    N3 -.-> N1
    N3 --> N0
    N4 -.-> N2
    N4 --> N3
    N5 -.-> N3
    N5 --> N2
    N6 -.-> N4
    N6 --> N5
    N7 -.-> N5
    N7 --> N4
    N8 -.-> N6
    N8 --> N7
    N9 -.-> N7
    N9 --> N6
    N10 -.-> N8
    N10 --> N9

    class N0 terminal
    class N1 terminal
    class N2 bddVariable
    class N3 bddVariable
    class N4 bddVariable
    class N5 bddVariable
    class N6 bddVariable
    class N7 bddVariable
    class N8 bddVariable
    class N9 bddVariable
    class N10 bddVariable

    classDef bddVariable fill:lightblue,stroke:#333,stroke-width:2px,color:#000
    classDef terminal fill:lightgray,stroke:#333,stroke-width:2px,color:#000
```

## Analysis Summary

- **Variables**: 5
- **BDD Nodes**: 11
- **Expression**: a XOR b XOR c XOR d XOR e

## BDD Node Table

The following table shows the internal structure of the BDD with node relationships:

| Index | Variable | False Child | True Child | Type |
|-------|----------|-------------|------------|------|
| 0 | a | 2 | 1 | Variable |
| 1 | b | 3 | 4 | Variable |
| 2 | b | 4 | 3 | Variable |
| 3 | c | 5 | 6 | Variable |
| 4 | c | 6 | 5 | Variable |
| 5 | d | 7 | 8 | Variable |
| 6 | d | 8 | 7 | Variable |
| 7 | e | 9 | 10 | Variable |
| 8 | e | 10 | 9 | Variable |
| 9 | - | - | - | Terminal(1) |
| 10 | - | - | - | Terminal(0) |

**Note**: Nodes are ordered topologically (parents before children) with terminal nodes at the end.
