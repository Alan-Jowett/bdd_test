# BDD Analysis Report

## Original Expression

```
foo_123_AND_456 OR bar_AND_baz_789 XOR test_OR_case_NOT_working AND final_XOR_test
```

## Expression Tree

The following diagram shows the parse tree of the logical expression:

```mermaid
---
title: Expression Tree
---
flowchart TD
    N1(("foo_123_AND_456"))
    N2(("bar_AND_baz_789"))
    N3["OR"]
    N4(("test_OR_case_NOT_working"))
    N5(("final_XOR_test"))
    N6["AND"]
    N7["XOR"]

    N3 --> N1
    N3 --> N2
    N6 --> N4
    N6 --> N5
    N7 --> N3
    N7 --> N6

    class N1 variable
    class N2 variable
    class N3 orOp
    class N4 variable
    class N5 variable
    class N6 andOp
    class N7 xorOp

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
    N2(("foo_123_AND_456"))
    N3(("bar_AND_baz_789"))
    N4(("test_OR_case_NOT_working"))
    N5(("test_OR_case_NOT_working"))
    N6(("foo_123_AND_456"))
    N7(("bar_AND_baz_789"))
    N8(("final_XOR_test"))

    N2 --> N1
    N2 -.-> N0
    N3 --> N1
    N3 -.-> N2
    N4 --> N1
    N4 -.-> N0
    N5 --> N0
    N5 -.-> N1
    N6 --> N5
    N6 -.-> N4
    N7 --> N5
    N7 -.-> N6
    N8 --> N7
    N8 -.-> N3

    class N0 terminal
    class N1 terminal
    class N2 bddVariable
    class N3 bddVariable
    class N4 bddVariable
    class N5 bddVariable
    class N6 bddVariable
    class N7 bddVariable
    class N8 bddVariable

    classDef bddVariable fill:lightblue,stroke:#333,stroke-width:2px,color:#000
    classDef terminal fill:lightgray,stroke:#333,stroke-width:2px,color:#000
```

## Analysis Summary

- **Variables**: 4
- **BDD Nodes**: 9
- **Expression**: foo_123_AND_456 OR bar_AND_baz_789 XOR test_OR_case_NOT_working AND final_XOR_test

## BDD Node Table

The following table shows the internal structure of the BDD with node relationships:

| Index | Variable | False Child | True Child | Type |
|-------|----------|-------------|------------|------|
| 0 | final_XOR_test | 5 | 1 | Variable |
| 1 | bar_AND_baz_789 | 2 | 3 | Variable |
| 2 | foo_123_AND_456 | 4 | 3 | Variable |
| 3 | test_OR_case_NOT_working | 7 | 8 | Variable |
| 4 | test_OR_case_NOT_working | 8 | 7 | Variable |
| 5 | bar_AND_baz_789 | 6 | 7 | Variable |
| 6 | foo_123_AND_456 | 8 | 7 | Variable |
| 7 | - | - | - | Terminal(1) |
| 8 | - | - | - | Terminal(0) |

**Note**: Nodes are ordered topologically (parents before children) with terminal nodes at the end.
