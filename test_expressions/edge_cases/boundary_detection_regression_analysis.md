# BDD Analysis Report

## Original Expression

```
foo_AND_bar AND bar_OR_baz AND baz_NOT_qux AND qux_XOR_alpha
```

## Expression Tree

The following diagram shows the parse tree of the logical expression:

```mermaid
---
title: Expression Tree
---
flowchart TD
    N1(("foo_AND_bar"))
    N2(("bar_OR_baz"))
    N3["AND"]
    N4(("baz_NOT_qux"))
    N5["AND"]
    N6(("qux_XOR_alpha"))
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
    N2(("foo_AND_bar"))
    N3(("baz_NOT_qux"))
    N4(("bar_OR_baz"))
    N5(("qux_XOR_alpha"))

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
- **Expression**: foo_AND_bar AND bar_OR_baz AND baz_NOT_qux AND qux_XOR_alpha

## BDD Node Table

The following table shows the internal structure of the BDD with node relationships:

| Index | Variable | False Child | True Child | Type |
|-------|----------|-------------|------------|------|
| 0 | qux_XOR_alpha | 5 | 1 | Variable |
| 1 | bar_OR_baz | 5 | 2 | Variable |
| 2 | baz_NOT_qux | 5 | 3 | Variable |
| 3 | foo_AND_bar | 5 | 4 | Variable |
| 4 | - | - | - | Terminal(1) |
| 5 | - | - | - | Terminal(0) |

**Note**: Nodes are ordered topologically (parents before children) with terminal nodes at the end.
