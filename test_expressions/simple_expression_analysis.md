# BDD Analysis Report

## Original Expression

```
(a AND b) OR (c AND d)
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
    N3(("a"))
    N4(("b"))
    N5["AND"]
    N6(("c"))
    N7(("d"))

    N1 --> N2
    N2 --> N3
    N2 --> N4
    N1 --> N5
    N5 --> N6
    N5 --> N7
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
    N2(("d"))
    N3(("c"))
    N4(("b"))
    N5(("a"))

    N2 -.-> N0
    N2 --> N1
    N3 -.-> N0
    N3 --> N2
    N4 -.-> N3
    N4 --> N1
    N5 -.-> N3
    N5 --> N4
```

## Analysis Summary

- **Variables**: 4
- **BDD Nodes**: 6
- **Expression**: (a AND b) OR (c AND d)
