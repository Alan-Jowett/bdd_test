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
title: BDD
---
flowchart TD
    N0["0"]
    N1["1"]
    N2(("c"))
    N3(("b"))
    N4(("a"))

    N2 -.-> N0
    N2 --> N1
    N3 --> N1
    N4 --> N1

    class N0 terminal
    class N1 terminal
    class N2 bddVariable
    class N3 bddVariable
    class N4 bddVariable

    classDef bddVariable fill:lightblue,stroke:#333,stroke-width:2px,color:#000
    classDef terminal fill:lightgray,stroke:#333,stroke-width:2px,color:#000
```

## Analysis Summary

- **Variables**: 3
- **BDD Nodes**: 5
- **Expression**: NOT (a OR b) AND c

## BDD Node Table

The following table shows the internal structure of the BDD with node relationships:

| Index | Variable | False Child | True Child | Type |
|-------|----------|-------------|------------|------|
| 0 | a | 1 | 4 | Variable |
| 1 | b | 2 | 4 | Variable |
| 2 | c | 4 | 3 | Variable |
| 3 | - | - | - | Terminal(1) |
| 4 | - | - | - | Terminal(0) |

**Note**: Nodes are ordered topologically (parents before children) with terminal nodes at the end.
