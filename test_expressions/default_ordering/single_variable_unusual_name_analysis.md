# BDD Analysis Report

## Original Expression

```
variable_with_unusual$characters@123
```

## Expression Tree

The following diagram shows the parse tree of the logical expression:

```mermaid
---
title: Expression Tree
---
flowchart TD
    N1(("variable_with_unusual$characters@123"))
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
    N2(("variable_with_unusual$characters@123"))

    N2 -.-> N0
    N2 --> N1
```

## Analysis Summary

- **Variables**: 1
- **BDD Nodes**: 3
- **Expression**: variable_with_unusual$characters@123
