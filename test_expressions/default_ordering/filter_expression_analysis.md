# BDD Analysis Report

## Original Expression

```
((x0 AND x1) OR (NOT x2)) XOR ((x3 AND (NOT x4)) OR (x5 XOR x6))
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
    N3["AND"]
    N4(("x0"))
    N5(("x1"))
    N6["NOT"]
    N7(("x2"))
    N8["OR"]
    N9["AND"]
    N10(("x3"))
    N11["NOT"]
    N12(("x4"))
    N13["XOR"]
    N14(("x5"))
    N15(("x6"))

    N1 --> N2
    N2 --> N3
    N3 --> N4
    N3 --> N5
    N2 --> N6
    N6 --> N7
    N1 --> N8
    N8 --> N9
    N9 --> N10
    N9 --> N11
    N11 --> N12
    N8 --> N13
    N13 --> N14
    N13 --> N15
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
    N2(("x6"))
    N3(("x6"))
    N4(("x5"))
    N5(("x4"))
    N6(("x3"))
    N7(("x5"))
    N8(("x4"))
    N9(("x3"))
    N10(("x2"))
    N11(("x1"))
    N12(("x0"))

    N2 -.-> N0
    N2 --> N1
    N3 -.-> N1
    N3 --> N0
    N4 -.-> N2
    N4 --> N3
    N5 -.-> N1
    N5 --> N4
    N6 -.-> N4
    N6 --> N5
    N7 -.-> N3
    N7 --> N2
    N8 -.-> N0
    N8 --> N7
    N9 -.-> N7
    N9 --> N8
    N10 -.-> N6
    N10 --> N9
    N11 -.-> N10
    N11 --> N6
    N12 -.-> N10
    N12 --> N11
```

## Analysis Summary

- **Variables**: 7
- **BDD Nodes**: 13
- **Expression**: ((x0 AND x1) OR (NOT x2)) XOR ((x3 AND (NOT x4)) OR (x5 XOR x6))
