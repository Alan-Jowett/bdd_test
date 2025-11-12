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
    N1(("c"))
    N2["0"]
    N3(("d"))
    N4["1"]
    N5(("b"))

    N0 -.-> N1
    N1 -.-> N2
    N1 --> N3
    N3 --> N4
    N0 --> N5

    class N0 default
    class N1 default
    class N2 default
    class N3 default
    class N4 default
    class N5 default
```

## Analysis Summary

- **Variables**: 4
- **BDD Nodes**: 5
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
