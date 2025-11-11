# BDD Analysis Report

## Original Expression

```
x1 OR x2 OR x3 OR x4 OR x5 OR x6 OR x7 OR x8 OR x9 OR x10
```

## Expression Tree

The following diagram shows the parse tree of the logical expression:

```mermaid
---
title: Expression Tree
---
flowchart TD
    N1["OR"]
    N2["OR"]
    N3["OR"]
    N4["OR"]
    N5["OR"]
    N6["OR"]
    N7["OR"]
    N8["OR"]
    N9["OR"]
    N10(("x1"))
    N11(("x2"))
    N12(("x3"))
    N13(("x4"))
    N14(("x5"))
    N15(("x6"))
    N16(("x7"))
    N17(("x8"))
    N18(("x9"))
    N19(("x10"))

    N1 --> N2
    N2 --> N3
    N3 --> N4
    N4 --> N5
    N5 --> N6
    N6 --> N7
    N7 --> N8
    N8 --> N9
    N9 --> N10
    N9 --> N11
    N8 --> N12
    N7 --> N13
    N6 --> N14
    N5 --> N15
    N4 --> N16
    N3 --> N17
    N2 --> N18
    N1 --> N19
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
    N2(("x9"))
    N3(("x8"))
    N4(("x7"))
    N5(("x6"))
    N6(("x5"))
    N7(("x4"))
    N8(("x3"))
    N9(("x2"))
    N10(("x10"))
    N11(("x1"))

    N2 -.-> N0
    N2 --> N1
    N3 -.-> N2
    N3 --> N1
    N4 -.-> N3
    N4 --> N1
    N5 -.-> N4
    N5 --> N1
    N6 -.-> N5
    N6 --> N1
    N7 -.-> N6
    N7 --> N1
    N8 -.-> N7
    N8 --> N1
    N9 -.-> N8
    N9 --> N1
    N10 -.-> N9
    N10 --> N1
    N11 -.-> N10
    N11 --> N1
```

## Analysis Summary

- **Variables**: 10
- **BDD Nodes**: 12
- **Expression**: x1 OR x2 OR x3 OR x4 OR x5 OR x6 OR x7 OR x8 OR x9 OR x10
