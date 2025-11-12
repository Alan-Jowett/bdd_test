# BDD Analysis Report

## Original Expression

```
variable_with_unusual$characters@123
```

## Expression Tree

The following diagram shows the parse tree of the logical expression:

```mermaid
---
title: Expression Tree
---
flowchart TD
    N1(("variable_with_unusual$characters@123"))

    class N1 variable

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
    N0(("variable_with_unusual$characters@123"))
    N1["0"]
    N2["1"]

    N0 -.-> N1
    N0 --> N2

    class N0 default
    class N1 default
    class N2 default
```

## Analysis Summary

- **Variables**: 1
- **BDD Nodes**: 2
- **Expression**: variable_with_unusual$characters@123

## BDD Node Table

The following table shows the internal structure of the BDD with node relationships:

| Index | Variable | False Child | True Child | Type |
|-------|----------|-------------|------------|------|
| 0 | variable_with_unusual$characters@123 | 2 | 1 | Variable |
| 1 | - | - | - | Terminal(1) |
| 2 | - | - | - | Terminal(0) |

**Note**: Nodes are ordered topologically (parents before children) with terminal nodes at the end.
