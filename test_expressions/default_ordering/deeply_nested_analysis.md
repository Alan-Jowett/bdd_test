# BDD Analysis Report

## Original Expression

```
(((a AND b) OR (c AND d)) XOR ((e OR f) AND (g XOR h))) AND (((NOT i) OR j) XOR ((k AND l) OR (NOT m)))
```

## Expression Tree

The following diagram shows the parse tree of the logical expression:

```mermaid
---
title: Expression Tree
---
flowchart TD
    N1["AND"]
    N2["XOR"]
    N3["OR"]
    N4["AND"]
    N5(("a"))
    N6(("b"))
    N7["AND"]
    N8(("c"))
    N9(("d"))
    N10["AND"]
    N11["OR"]
    N12(("e"))
    N13(("f"))
    N14["XOR"]
    N15(("g"))
    N16(("h"))
    N17["XOR"]
    N18["OR"]
    N19["NOT"]
    N20(("i"))
    N21(("j"))
    N22["OR"]
    N23["AND"]
    N24(("k"))
    N25(("l"))
    N26["NOT"]
    N27(("m"))

    N1 --> N2
    N2 --> N3
    N3 --> N4
    N4 --> N5
    N4 --> N6
    N3 --> N7
    N7 --> N8
    N7 --> N9
    N2 --> N10
    N10 --> N11
    N11 --> N12
    N11 --> N13
    N10 --> N14
    N14 --> N15
    N14 --> N16
    N1 --> N17
    N17 --> N18
    N18 --> N19
    N19 --> N20
    N18 --> N21
    N17 --> N22
    N22 --> N23
    N23 --> N24
    N23 --> N25
    N22 --> N26
    N26 --> N27
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
    N2(("m"))
    N3(("l"))
    N4(("k"))
    N5(("m"))
    N6(("l"))
    N7(("k"))
    N8(("j"))
    N9(("i"))
    N10(("h"))
    N11(("h"))
    N12(("g"))
    N13(("f"))
    N14(("e"))
    N15(("g"))
    N16(("f"))
    N17(("e"))
    N18(("d"))
    N19(("c"))
    N20(("b"))
    N21(("a"))

    N2 -.-> N0
    N2 --> N1
    N3 -.-> N2
    N3 --> N0
    N4 -.-> N2
    N4 --> N3
    N5 -.-> N1
    N5 --> N0
    N6 -.-> N5
    N6 --> N1
    N7 -.-> N5
    N7 --> N6
    N8 -.-> N7
    N8 --> N4
    N9 -.-> N4
    N9 --> N8
    N10 -.-> N0
    N10 --> N9
    N11 -.-> N9
    N11 --> N0
    N12 -.-> N10
    N12 --> N11
    N13 -.-> N0
    N13 --> N12
    N14 -.-> N13
    N14 --> N12
    N15 -.-> N11
    N15 --> N10
    N16 -.-> N9
    N16 --> N15
    N17 -.-> N16
    N17 --> N15
    N18 -.-> N14
    N18 --> N17
    N19 -.-> N14
    N19 --> N18
    N20 -.-> N19
    N20 --> N17
    N21 -.-> N19
    N21 --> N20
```

## Analysis Summary

- **Variables**: 13
- **BDD Nodes**: 22
- **Expression**: (((a AND b) OR (c AND d)) XOR ((e OR f) AND (g XOR h))) AND (((NOT i) OR j) XOR ((k AND l) OR (NOT m)))
