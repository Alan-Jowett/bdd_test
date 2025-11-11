# BDD Analysis Report

## Original Expression

```
NOT (a OR b) AND c
```

## Expression Tree

The following diagram shows the parse tree of the logical expression:

```mermaid
---
title: Expression Tree
---
flowchart TD
    N1["AND"]
    N2["NOT"]
    N3["OR"]
    N4(("a"))
    N5(("b"))
    N6(("c"))

    N1 --> N2
    N2 --> N3
    N3 --> N4
    N3 --> N5
    N1 --> N6
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
    N2(("c"))
    N3(("b"))
    N4(("a"))

    N2 -.-> N0
    N2 --> N1
    N3 -.-> N2
    N3 --> N0
    N4 -.-> N3
    N4 --> N0
```

## Analysis Summary

- **Variables**: 3
- **BDD Nodes**: 5
- **Expression**: NOT (a OR b) AND c
