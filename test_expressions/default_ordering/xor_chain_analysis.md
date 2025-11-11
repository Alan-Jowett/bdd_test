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
    N2(("e"))
    N3(("e"))
    N4(("d"))
    N5(("d"))
    N6(("c"))
    N7(("c"))
    N8(("b"))
    N9(("b"))
    N10(("a"))

    N2 -.-> N0
    N2 --> N1
    N3 -.-> N1
    N3 --> N0
    N4 -.-> N2
    N4 --> N3
    N5 -.-> N3
    N5 --> N2
    N6 -.-> N4
    N6 --> N5
    N7 -.-> N5
    N7 --> N4
    N8 -.-> N6
    N8 --> N7
    N9 -.-> N7
    N9 --> N6
    N10 -.-> N8
    N10 --> N9
```

## Analysis Summary

- **Variables**: 5
- **BDD Nodes**: 11
- **Expression**: a XOR b XOR c XOR d XOR e
