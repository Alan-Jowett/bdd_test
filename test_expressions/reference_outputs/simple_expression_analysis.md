# BDD Analysis Report

## Original Expression

```
(a AND b) OR (c AND d)
```

## Expression Tree

The following diagram shows the parse tree of the logical expression:

```mermaid
---
title: Expression Tree
---
flowchart TD
    N1["OR"]
    N2["AND"]
    N3(("a"))
    N4(("b"))
    N5["AND"]
    N6(("c"))
    N7(("d"))

    N1 --> N2
    N2 --> N3
    N2 --> N4
    N1 --> N5
    N5 --> N6
    N5 --> N7

    class N1 orOp
    class N2 andOp
    class N3 variable
    class N4 variable
    class N5 andOp
    class N6 variable
    class N7 variable

    classDef variable fill:lightblue,stroke:#333,stroke-width:2px
    classDef andOp fill:lightgreen,stroke:#333,stroke-width:2px
    classDef orOp fill:lightcoral,stroke:#333,stroke-width:2px
    classDef notOp fill:yellow,stroke:#333,stroke-width:2px
    classDef xorOp fill:lightpink,stroke:#333,stroke-width:2px
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
    N2(("d"))
    N3(("c"))
    N4(("b"))
    N5(("a"))

    N2 -.-> N0
    N2 --> N1
    N3 -.-> N0
    N3 --> N2
    N4 -.-> N3
    N4 --> N1
    N5 -.-> N3
    N5 --> N4

    class N0 terminal
    class N1 terminal
    class N2 bddVariable
    class N3 bddVariable
    class N4 bddVariable
    class N5 bddVariable

    classDef bddVariable fill:lightblue,stroke:#333,stroke-width:2px
    classDef terminal fill:lightgray,stroke:#333,stroke-width:2px
```

## Analysis Summary

- **Variables**: 4
- **BDD Nodes**: 6
- **Expression**: (a AND b) OR (c AND d)

## BDD Node Table

The following table shows the internal structure of the BDD with node relationships:

| Index | Variable | False Child | True Child | Type |
|-------|----------|-------------|------------|------|
| 0 | a | 2 | 1 | Variable |
| 1 | b | 2 | 4 | Variable |
| 2 | c | 5 | 3 | Variable |
| 3 | d | 5 | 4 | Variable |
| 4 | - | - | - | Terminal(1) |
| 5 | - | - | - | Terminal(0) |

**Note**: Nodes are ordered topologically (parents before children) with terminal nodes at the end.
