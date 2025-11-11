# BDD Analysis Report

## Original Expression

```
x AND y OR z
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
    N3(("x"))
    N4(("y"))
    N5(("z"))

    N1 --> N2
    N2 --> N3
    N2 --> N4
    N1 --> N5
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
    N2(("z"))
    N3(("y"))
    N4(("x"))

    N2 -.-> N0
    N2 --> N1
    N3 -.-> N2
    N3 --> N1
    N4 -.-> N2
    N4 --> N3
```

## Analysis Summary

- **Variables**: 3
- **BDD Nodes**: 5
- **Expression**: x AND y OR z
