# BDD Analysis Report

## Original Expression

```
NOT (a OR b) AND c
```

## Expression Tree

The following diagram shows the parse tree of the logical expression:

```mermaid
---
title: Expression Tree
---
flowchart TD
    N1["AND"]
    N2["NOT"]
    N3["OR"]
    N4(("a"))
    N5(("b"))
    N6(("c"))

    N1 --> N2
    N2 --> N3
    N3 --> N4
    N3 --> N5
    N1 --> N6

    class N1 andOp
    class N2 notOp
    class N3 orOp
    class N4 variable
    class N5 variable
    class N6 variable

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
    N0(("a"))
    N1(("b"))
    N2(("c"))
    N3["0"]
    N4["1"]

    N0 -.-> N1
    N1 -.-> N2
    N2 -.-> N3
    N2 --> N4

    class N0 default
    class N1 default
    class N2 default
    class N3 default
    class N4 default
```

## Analysis Summary

- **Variables**: 3
- **BDD Nodes**: 4
- **Expression**: NOT (a OR b) AND c

## BDD Node Table

The following table shows the internal structure of the BDD with node relationships:

| Index | Variable | False Child | True Child | Type |
|-------|----------|-------------|------------|------|
| 0 | a | 1 | 3 | Variable |
| 1 | b | 2 | 3 | Variable |
| 2 | c | 4 | 3 | Variable |
| 3 | - | - | - | Terminal(1) |
| 4 | - | - | - | Terminal(0) |

**Note**: Nodes are ordered topologically (parents before children) with terminal nodes at the end.
