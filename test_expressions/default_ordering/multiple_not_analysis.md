# BDD Analysis Report

## Original Expression

```
NOT NOT NOT x
```

## Expression Tree

The following diagram shows the parse tree of the logical expression:

```mermaid
---
title: Expression Tree
---
flowchart TD
    N1["NOT"]
    N2["NOT"]
    N3["NOT"]
    N4(("x"))

    N1 --> N2
    N2 --> N3
    N3 --> N4
```

## Binary Decision Diagram (BDD)

The following diagram shows the optimized BDD representation:

```mermaid
---
title: BDD
---
flowchart TD
    N0["1"]
    N1["0"]
    N2(("x"))

    N2 -.-> N0
    N2 --> N1
```

## Analysis Summary

- **Variables**: 1
- **BDD Nodes**: 3
- **Expression**: NOT NOT NOT x
