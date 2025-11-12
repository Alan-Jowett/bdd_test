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
title: CUDD BDD
---
flowchart TD
    N0(("x0"))
    N1(("x2"))
    N2(("x3"))
    N3(("x5"))
    N4(("x6"))
    N5["0"]
    N6["1"]
    N7(("x6"))
    N8(("x4"))
    N9(("x3"))
    N10(("x1"))

    N0 -.-> N1
    N1 -.-> N2
    N2 -.-> N3
    N3 -.-> N4
    N4 -.-> N5
    N4 --> N6
    N3 --> N7
    N2 --> N8
    N1 --> N9
    N0 --> N10

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
| 1 | x1 | 2 | 3 | Variable |
| 2 | x2 | 4 | 3 | Variable |
| 3 | x3 | 6 | 5 | Variable |
| 4 | x3 | 6 | 5 | Variable |
| 5 | x4 | 10 | 6 | Variable |
| 6 | x5 | 8 | 7 | Variable |
| 7 | x6 | 10 | 9 | Variable |
| 8 | x6 | 10 | 9 | Variable |
| 9 | - | - | - | Terminal(1) |
| 10 | - | - | - | Terminal(0) |

**Note**: Nodes are ordered topologically (parents before children) with terminal nodes at the end.
