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
    N1["OR"]
    N2["AND"]
    N3(("var_1"))
    N4(("var_2"))
    N5(("var_3"))

    N1 --> N2
    N2 --> N3
    N2 --> N4
    N1 --> N5

    class N1 orOp
    class N2 andOp
    class N3 variable
    class N4 variable
    class N5 variable

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
    N0(("var_1"))
    N1(("var_3"))
    N2["0"]
    N3["1"]
    N4(("var_2"))

    N0 -.-> N1
    N1 -.-> N2
    N1 --> N3
    N0 --> N4

    class N0 default
    class N1 default
    class N2 default
    class N3 default
    class N4 default
```

## Analysis Summary

- **Variables**: 3
- **BDD Nodes**: 4
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
