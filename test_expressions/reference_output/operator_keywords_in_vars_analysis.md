# BDD Analysis Report

## Original Expression

```
AND_at_start AND OR_in_middle AND at_end_NOT AND NOT_multiple_AND_keywords
```

## Expression Tree

The following diagram shows the parse tree of the logical expression:

```mermaid
---
title: Expression Tree
---
flowchart TD
    N1(("AND_at_start"))
    N2(("OR_in_middle"))
    N3["AND"]
    N4(("at_end_NOT"))
    N5["AND"]
    N6(("NOT_multiple_AND_keywords"))
    N7["AND"]

    N3 --> N1
    N3 --> N2
    N5 --> N3
    N5 --> N4
    N7 --> N5
    N7 --> N6

    class N1 variable
    class N2 variable
    class N3 andOp
    class N4 variable
    class N5 andOp
    class N6 variable
    class N7 andOp

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
    N2(("at_end_NOT"))
    N3(("OR_in_middle"))
    N4(("NOT_multiple_AND_keywords"))
    N5(("AND_at_start"))

    N2 --> N1
    N2 -.-> N0
    N3 --> N2
    N3 -.-> N0
    N4 --> N3
    N4 -.-> N0
    N5 --> N4
    N5 -.-> N0

    class N0 terminal
    class N1 terminal
    class N2 bddVariable
    class N3 bddVariable
    class N4 bddVariable
    class N5 bddVariable

    classDef bddVariable fill:lightblue,stroke:#333,stroke-width:2px,color:#000
    classDef terminal fill:lightgray,stroke:#333,stroke-width:2px,color:#000
```

## Analysis Summary

- **Variables**: 4
- **BDD Nodes**: 6
- **Expression**: AND_at_start AND OR_in_middle AND at_end_NOT AND NOT_multiple_AND_keywords

## BDD Node Table

The following table shows the internal structure of the BDD with node relationships:

| Index | Variable | False Child | True Child | Type |
|-------|----------|-------------|------------|------|
| 0 | AND_at_start | 5 | 1 | Variable |
| 1 | NOT_multiple_AND_keywords | 5 | 2 | Variable |
| 2 | OR_in_middle | 5 | 3 | Variable |
| 3 | at_end_NOT | 5 | 4 | Variable |
| 4 | - | - | - | Terminal(1) |
| 5 | - | - | - | Terminal(0) |

**Note**: Nodes are ordered topologically (parents before children) with terminal nodes at the end.
