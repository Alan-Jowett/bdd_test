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
    N1["XOR"]
    N2["OR"]
    N3["AND"]
    N4(("x0"))
    N5(("x1"))
    N6["NOT"]
    N7(("x2"))
    N8["OR"]
    N9["AND"]
    N10(("x3"))
    N11["NOT"]
    N12(("x4"))
    N13["XOR"]
    N14(("x5"))
    N15(("x6"))

    N1 --> N2
    N2 --> N3
    N3 --> N4
    N3 --> N5
    N2 --> N6
    N6 --> N7
    N1 --> N8
    N8 --> N9
    N9 --> N10
    N9 --> N11
    N11 --> N12
    N8 --> N13
    N13 --> N14
    N13 --> N15

    class N1 xorOp
    class N2 orOp
    class N3 andOp
    class N4 variable
    class N5 variable
    class N6 notOp
    class N7 variable
    class N8 orOp
    class N9 andOp
    class N10 variable
    class N11 notOp
    class N12 variable
    class N13 xorOp
    class N14 variable
    class N15 variable

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
    N1["0"]
    N2(("x6"))
    N3(("x6"))
    N4(("x5"))
    N5(("x4"))
    N6(("x3"))
    N7(("x5"))
    N8(("x4"))
    N9(("x3"))
    N10(("x2"))
    N11(("x1"))
    N12(("x0"))

    N2 -.-> N1
    N2 --> N0
    N3 -.-> N1
    N3 --> N0
    N4 -.-> N2
    N4 --> N3
    N5 -.-> N1
    N5 --> N4
    N6 -.-> N4
    N6 --> N5
    N7 -.-> N2
    N7 --> N3
    N8 -.-> N1
    N8 --> N4
    N9 -.-> N4
    N9 --> N5
    N10 -.-> N9
    N10 --> N6
    N11 -.-> N10
    N11 --> N6
    N12 -.-> N10
    N12 --> N11

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
- **BDD Nodes**: 8
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
