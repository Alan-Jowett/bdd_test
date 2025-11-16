# BDD Analysis Report

## Original Expression

```
((x0 AND x1) OR (NOT x2)) XOR ((x3 AND (NOT x4)) OR (x5 XOR x6))
```

## Expression Tree

The following diagram shows the parse tree of the logical expression:

```mermaid
---
title: Expression Tree
---
flowchart TD
    N1(("x0"))
    N10["AND"]
    N11(("x5"))
    N12(("x6"))
    N13["XOR"]
    N14["OR"]
    N15["XOR"]
    N2(("x1"))
    N3["AND"]
    N4(("x2"))
    N5["NOT"]
    N6["OR"]
    N7(("x3"))
    N8(("x4"))
    N9["NOT"]

    N10 --> N7
    N10 --> N9
    N13 --> N11
    N13 --> N12
    N14 --> N10
    N14 --> N13
    N15 --> N14
    N15 --> N6
    N3 --> N1
    N3 --> N2
    N5 --> N4
    N6 --> N3
    N6 --> N5
    N9 --> N8

    class N1 variable
    class N2 variable
    class N3 andOp
    class N4 variable
    class N5 notOp
    class N6 orOp
    class N7 variable
    class N8 variable
    class N9 notOp
    class N10 andOp
    class N11 variable
    class N12 variable
    class N13 xorOp
    class N14 orOp
    class N15 xorOp

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
    N0["1"]
    N10(("x2"))
    N11(("x1"))
    N12(("x0"))
    N1["0"]
    N2(("x6"))
    N3(("x6"))
    N4(("x5"))
    N5(("x4"))
    N6(("x3"))
    N7(("x5"))
    N8(("x4"))
    N9(("x3"))

    N10 --> N9
    N10 -.-> N6
    N11 --> N6
    N11 -.-> N10
    N12 --> N11
    N12 -.-> N10
    N2 --> N1
    N2 -.-> N0
    N3 --> N0
    N3 -.-> N1
    N4 --> N3
    N4 -.-> N2
    N5 --> N4
    N5 -.-> N1
    N6 --> N5
    N6 -.-> N4
    N7 --> N2
    N7 -.-> N3
    N8 --> N7
    N8 -.-> N0
    N9 --> N8
    N9 -.-> N7

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
    class N11 bddVariable
    class N12 bddVariable

    classDef bddVariable fill:lightblue,stroke:#333,stroke-width:2px,color:#000
    classDef terminal fill:lightgray,stroke:#333,stroke-width:2px,color:#000
```

## Analysis Summary

- **Variables**: 7
- **BDD Nodes**: 13
- **Expression**: ((x0 AND x1) OR (NOT x2)) XOR ((x3 AND (NOT x4)) OR (x5 XOR x6))

## BDD Node Table

The following table shows the internal structure of the BDD with node relationships:

| Index | Variable | False Child | True Child | Type |
|-------|----------|-------------|------------|------|
| 0 | x0 | 2 | 1 | Variable |
| 1 | x1 | 2 | 6 | Variable |
| 2 | x2 | 6 | 3 | Variable |
| 3 | x3 | 5 | 4 | Variable |
| 4 | x4 | 12 | 5 | Variable |
| 5 | x5 | 9 | 10 | Variable |
| 6 | x3 | 8 | 7 | Variable |
| 7 | x4 | 11 | 8 | Variable |
| 8 | x5 | 10 | 9 | Variable |
| 9 | x6 | 11 | 12 | Variable |
| 10 | x6 | 12 | 11 | Variable |
| 11 | - | - | - | Terminal(0) |
| 12 | - | - | - | Terminal(1) |

**Note**: Nodes are ordered topologically (parents before children) with terminal nodes at the end.
