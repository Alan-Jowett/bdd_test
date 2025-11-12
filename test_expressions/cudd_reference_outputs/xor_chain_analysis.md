# BDD Analysis Report

## Original Expression

```
a XOR b XOR c XOR d XOR e
```

## Expression Tree

The following diagram shows the parse tree of the logical expression:

```mermaid
---
title: Expression Tree
---
flowchart TD
    N1["XOR"]
    N2["XOR"]
    N3["XOR"]
    N4["XOR"]
    N5(("a"))
    N6(("b"))
    N7(("c"))
    N8(("d"))
    N9(("e"))

    N1 --> N2
    N2 --> N3
    N3 --> N4
    N4 --> N5
    N4 --> N6
    N3 --> N7
    N2 --> N8
    N1 --> N9

    class N1 xorOp
    class N2 xorOp
    class N3 xorOp
    class N4 xorOp
    class N5 variable
    class N6 variable
    class N7 variable
    class N8 variable
    class N9 variable

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
    N1(("b"))
    N2(("c"))
    N3(("d"))
    N4(("e"))
    N5["0"]
    N6["1"]
    N7(("e"))
    N8(("d"))
    N9(("c"))
    N10(("b"))

    N0 -.-> N1
    N1 -.-> N2
    N2 -.-> N3
    N3 -.-> N4
    N4 -.-> N5
    N4 --> N6
    N3 --> N7
    N2 --> N8
    N1 --> N9
    N0 --> N10

    class N0 default
    class N1 default
    class N2 default
    class N3 default
    class N4 default
    class N5 default
    class N6 default
    class N7 default
    class N8 default
    class N9 default
    class N10 default
```

## Analysis Summary

- **Variables**: 5
- **BDD Nodes**: 6
- **Expression**: a XOR b XOR c XOR d XOR e

## BDD Node Table

The following table shows the internal structure of the BDD with node relationships:

| Index | Variable | False Child | True Child | Type |
|-------|----------|-------------|------------|------|
| 0 | a | 2 | 1 | Variable |
| 1 | b | 4 | 3 | Variable |
| 2 | b | 4 | 3 | Variable |
| 3 | c | 6 | 5 | Variable |
| 4 | c | 6 | 5 | Variable |
| 5 | d | 8 | 7 | Variable |
| 6 | d | 8 | 7 | Variable |
| 7 | e | 10 | 9 | Variable |
| 8 | e | 10 | 9 | Variable |
| 9 | - | - | - | Terminal(1) |
| 10 | - | - | - | Terminal(0) |

**Note**: Nodes are ordered topologically (parents before children) with terminal nodes at the end.
