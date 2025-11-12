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
    N1["OR"]
    N2["OR"]
    N3["OR"]
    N4["OR"]
    N5["OR"]
    N6["OR"]
    N7["OR"]
    N8["OR"]
    N9["OR"]
    N10(("x1"))
    N11(("x2"))
    N12(("x3"))
    N13(("x4"))
    N14(("x5"))
    N15(("x6"))
    N16(("x7"))
    N17(("x8"))
    N18(("x9"))
    N19(("x10"))

    N1 --> N2
    N2 --> N3
    N3 --> N4
    N4 --> N5
    N5 --> N6
    N6 --> N7
    N7 --> N8
    N8 --> N9
    N9 --> N10
    N9 --> N11
    N8 --> N12
    N7 --> N13
    N6 --> N14
    N5 --> N15
    N4 --> N16
    N3 --> N17
    N2 --> N18
    N1 --> N19

    class N1 orOp
    class N2 orOp
    class N3 orOp
    class N4 orOp
    class N5 orOp
    class N6 orOp
    class N7 orOp
    class N8 orOp
    class N9 orOp
    class N10 variable
    class N11 variable
    class N12 variable
    class N13 variable
    class N14 variable
    class N15 variable
    class N16 variable
    class N17 variable
    class N18 variable
    class N19 variable

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
title: CUDD BDD
---
flowchart TD
    N0(("x1"))
    N1(("x10"))
    N2(("x2"))
    N3(("x3"))
    N4(("x4"))
    N5(("x5"))
    N6(("x6"))
    N7(("x7"))
    N8(("x8"))
    N9(("x9"))
    N10["0"]
    N11["1"]

    N0 -.-> N1
    N1 -.-> N2
    N2 -.-> N3
    N3 -.-> N4
    N4 -.-> N5
    N5 -.-> N6
    N6 -.-> N7
    N7 -.-> N8
    N8 -.-> N9
    N9 -.-> N10
    N9 --> N11

    class N0 default
    class N1 default
    class N2 default
    class N3 default
    class N4 default
    class N5 default
    class N6 default
    class N7 default
    class N8 default
    class N9 default
    class N10 default
    class N11 default
```

## Analysis Summary

- **Variables**: 10
- **BDD Nodes**: 11
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
