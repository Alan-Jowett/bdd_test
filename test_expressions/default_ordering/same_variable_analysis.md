# BDD Analysis Report

## Original Expression

```
x0 AND x0
```

## Expression Tree

The following diagram shows the parse tree of the logical expression:

```mermaid
---
title: Expression Tree
---
flowchart TD
    N1["AND"]
    N2(("x0"))
    N3(("x0"))

    N1 --> N2
    N1 --> N3
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
    N2(("x0"))

    N2 -.-> N0
    N2 --> N1
```

## Analysis Summary

- **Variables**: 1
- **BDD Nodes**: 3
- **Expression**: x0 AND x0
