# BDD Analysis Report

## Original Expression

```
x1 OR x2 OR x3 OR x4 OR x5 OR x6 OR x7 OR x8 OR x9 OR x10
```

## Expression Tree

The following diagram shows the parse tree of the logical expression:

```mermaid
---
title: Expression Tree
---
flowchart TD
    N1(("x1"))
    N2(("x2"))
    N3["OR"]
    N4(("x3"))
    N5["OR"]
    N6(("x4"))
    N7["OR"]
    N8(("x5"))
    N9["OR"]
    N10(("x6"))
    N11["OR"]
    N12(("x7"))
    N13["OR"]
    N14(("x8"))
    N15["OR"]
    N16(("x9"))
    N17["OR"]
    N18(("x10"))
    N19["OR"]

    N3 --> N1
    N3 --> N2
    N5 --> N3
    N5 --> N4
    N7 --> N5
    N7 --> N6
    N9 --> N7
    N9 --> N8
    N11 --> N9
    N11 --> N10
    N13 --> N11
    N13 --> N12
    N15 --> N13
    N15 --> N14
    N17 --> N15
    N17 --> N16
    N19 --> N17
    N19 --> N18

    class N1 variable
    class N2 variable
    class N3 orOp
    class N4 variable
    class N5 orOp
    class N6 variable
    class N7 orOp
    class N8 variable
    class N9 orOp
    class N10 variable
    class N11 orOp
    class N12 variable
    class N13 orOp
    class N14 variable
    class N15 orOp
    class N16 variable
    class N17 orOp
    class N18 variable
    class N19 orOp

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
    N2(("x9"))
    N3(("x8"))
    N4(("x7"))
    N5(("x6"))
    N6(("x5"))
    N7(("x4"))
    N8(("x3"))
    N9(("x2"))
    N10(("x10"))
    N11(("x1"))

    N2 -.-> N0
    N2 --> N1
    N3 -.-> N2
    N3 --> N1
    N4 -.-> N3
    N4 --> N1
    N5 -.-> N4
    N5 --> N1
    N6 -.-> N5
    N6 --> N1
    N7 -.-> N6
    N7 --> N1
    N8 -.-> N7
    N8 --> N1
    N9 -.-> N8
    N9 --> N1
    N10 -.-> N9
    N10 --> N1
    N11 -.-> N10
    N11 --> N1

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

    classDef bddVariable fill:lightblue,stroke:#333,stroke-width:2px,color:#000
    classDef terminal fill:lightgray,stroke:#333,stroke-width:2px,color:#000
```

## Analysis Summary

- **Variables**: 10
- **BDD Nodes**: 12
- **Expression**: x1 OR x2 OR x3 OR x4 OR x5 OR x6 OR x7 OR x8 OR x9 OR x10

## BDD Node Table

The following table shows the internal structure of the BDD with node relationships:

| Index | Variable | False Child | True Child | Type |
|-------|----------|-------------|------------|------|
| 0 | x1 | 1 | 10 | Variable |
| 1 | x10 | 2 | 10 | Variable |
| 2 | x2 | 3 | 10 | Variable |
| 3 | x3 | 4 | 10 | Variable |
| 4 | x4 | 5 | 10 | Variable |
| 5 | x5 | 6 | 10 | Variable |
| 6 | x6 | 7 | 10 | Variable |
| 7 | x7 | 8 | 10 | Variable |
| 8 | x8 | 9 | 10 | Variable |
| 9 | x9 | 11 | 10 | Variable |
| 10 | - | - | - | Terminal(1) |
| 11 | - | - | - | Terminal(0) |

**Note**: Nodes are ordered topologically (parents before children) with terminal nodes at the end.
