# BDD Analysis Report

## Original Expression

```
var_1 AND var_2 OR var_3
```

## Expression Tree

The following diagram shows the parse tree of the logical expression:

```mermaid
---
title: Expression Tree
---
flowchart TD
    N1(("var_1"))
    N2(("var_2"))
    N3["AND"]
    N4(("var_3"))
    N5["OR"]

    N3 --> N1
    N3 --> N2
    N5 --> N3
    N5 --> N4

    class N1 variable
    class N2 variable
    class N3 andOp
    class N4 variable
    class N5 orOp

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
    N2(("var_3"))
    N3(("var_2"))
    N4(("var_1"))

    N2 -.-> N0
    N2 --> N1
    N3 -.-> N2
    N3 --> N1
    N4 -.-> N2
    N4 --> N3

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
- **Expression**: var_1 AND var_2 OR var_3

## BDD Node Table

The following table shows the internal structure of the BDD with node relationships:

| Index | Variable | False Child | True Child | Type |
|-------|----------|-------------|------------|------|
| 0 | var_1 | 2 | 1 | Variable |
| 1 | var_2 | 2 | 3 | Variable |
| 2 | var_3 | 4 | 3 | Variable |
| 3 | - | - | - | Terminal(1) |
| 4 | - | - | - | Terminal(0) |

**Note**: Nodes are ordered topologically (parents before children) with terminal nodes at the end.
