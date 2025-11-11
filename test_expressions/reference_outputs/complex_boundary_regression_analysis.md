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
    N1["XOR"]
    N2["OR"]
    N3(("foo_123_AND_456"))
    N4(("bar_AND_baz_789"))
    N5["AND"]
    N6(("test_OR_case_NOT_working"))
    N7(("final_XOR_test"))

    N1 --> N2
    N2 --> N3
    N2 --> N4
    N1 --> N5
    N5 --> N6
    N5 --> N7

    class N1 xorOp
    class N2 orOp
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
    N2(("foo_123_AND_456"))
    N3(("test_OR_case_NOT_working"))
    N4(("test_OR_case_NOT_working"))
    N5(("foo_123_AND_456"))
    N6(("final_XOR_test"))
    N7(("final_XOR_test"))
    N8(("bar_AND_baz_789"))

    N2 -.-> N0
    N2 --> N1
    N3 -.-> N0
    N3 --> N1
    N4 -.-> N1
    N4 --> N0
    N5 -.-> N3
    N5 --> N4
    N6 -.-> N2
    N6 --> N5
    N7 -.-> N1
    N7 --> N4
    N8 -.-> N6
    N8 --> N7

    class N0 terminal
    class N1 terminal
    class N2 bddVariable
    class N3 bddVariable
    class N4 bddVariable
    class N5 bddVariable
    class N6 bddVariable
    class N7 bddVariable
    class N8 bddVariable

    classDef bddVariable fill:lightblue,stroke:#333,stroke-width:2px
    classDef terminal fill:lightgray,stroke:#333,stroke-width:2px
```

## Analysis Summary

- **Variables**: 4
- **BDD Nodes**: 9
- **Expression**: foo_123_AND_456 OR bar_AND_baz_789 XOR test_OR_case_NOT_working AND final_XOR_test

## BDD Node Table

The following table shows the internal structure of the BDD with node relationships:

| Index | Variable | False Child | True Child | Type |
|-------|----------|-------------|------------|------|
| 0 | bar_AND_baz_789 | 2 | 1 | Variable |
| 1 | final_XOR_test | 7 | 4 | Variable |
| 2 | final_XOR_test | 6 | 3 | Variable |
| 3 | foo_123_AND_456 | 5 | 4 | Variable |
| 4 | test_OR_case_NOT_working | 7 | 8 | Variable |
| 5 | test_OR_case_NOT_working | 8 | 7 | Variable |
| 6 | foo_123_AND_456 | 8 | 7 | Variable |
| 7 | - | - | - | Terminal(1) |
| 8 | - | - | - | Terminal(0) |

**Note**: Nodes are ordered topologically (parents before children) with terminal nodes at the end.
