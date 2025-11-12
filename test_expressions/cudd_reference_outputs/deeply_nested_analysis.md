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

    class N1 andOp
    class N2 xorOp
    class N3 orOp
    class N4 andOp
    class N5 variable
    class N6 variable
    class N7 andOp
    class N8 variable
    class N9 variable
    class N10 andOp
    class N11 orOp
    class N12 variable
    class N13 variable
    class N14 xorOp
    class N15 variable
    class N16 variable
    class N17 xorOp
    class N18 orOp
    class N19 notOp
    class N20 variable
    class N21 variable
    class N22 orOp
    class N23 andOp
    class N24 variable
    class N25 variable
    class N26 notOp
    class N27 variable

    classDef variable fill:lightblue,stroke:#333,stroke-width:2px,color:#000
    classDef andOp fill:lightgreen,stroke:#333,stroke-width:2px,color:#000
    classDef orOp fill:lightcoral,stroke:#333,stroke-width:2px,color:#000
    classDef notOp fill:yellow,stroke:#333,stroke-width:2px,color:#000
    classDef xorOp fill:lightpink,stroke:#333,stroke-width:2px,color:#000
```

## Binary Decision Diagram (BDD)

The following diagram shows the optimized BDD representation:

```mermaid
---
title: CUDD BDD
---
flowchart TD
    N0(("a"))
    N1(("c"))
    N2(("e"))
    N3(("f"))
    N4["1"]
    N5(("g"))
    N6(("h"))
    N7(("i"))
    N8(("k"))
    N9(("m"))
    N10["0"]
    N11(("l"))
    N12(("j"))
    N13(("k"))
    N14(("h"))
    N15(("d"))
    N16(("e"))
    N17(("f"))
    N18(("g"))
    N19(("b"))

    N0 -.-> N1
    N1 -.-> N2
    N2 -.-> N3
    N3 -.-> N4
    N3 --> N5
    N5 -.-> N6
    N6 --> N7
    N7 -.-> N8
    N8 -.-> N9
    N9 -.-> N10
    N8 --> N11
    N7 --> N12
    N12 -.-> N13
    N5 --> N14
    N1 --> N15
    N15 --> N16
    N16 -.-> N17
    N17 --> N18
    N0 --> N19

    class N0 default
    class N1 default
    class N2 default
    class N3 default
    class N4 default
    class N5 default
    class N6 default
    class N7 default
    class N8 default
    class N9 default
    class N10 default
    class N11 default
    class N12 default
    class N13 default
    class N14 default
    class N15 default
    class N16 default
    class N17 default
    class N18 default
    class N19 default
```

## Analysis Summary

- **Variables**: 13
- **BDD Nodes**: 18
- **Expression**: (((a AND b) OR (c AND d)) XOR ((e OR f) AND (g XOR h))) AND (((NOT i) OR j) XOR ((k AND l) OR (NOT m)))

## BDD Node Table

The following table shows the internal structure of the BDD with node relationships:

| Index | Variable | False Child | True Child | Type |
|-------|----------|-------------|------------|------|
| 0 | a | 2 | 1 | Variable |
| 1 | b | 2 | 4 | Variable |
| 2 | c | 7 | 3 | Variable |
| 3 | d | 7 | 4 | Variable |
| 4 | e | 5 | 6 | Variable |
| 5 | f | 12 | 6 | Variable |
| 6 | g | 10 | 11 | Variable |
| 7 | e | 8 | 9 | Variable |
| 8 | f | 19 | 9 | Variable |
| 9 | g | 11 | 10 | Variable |
| 10 | h | 12 | 19 | Variable |
| 11 | h | 19 | 12 | Variable |
| 12 | i | 15 | 13 | Variable |
| 13 | j | 14 | 15 | Variable |
| 14 | k | 17 | 16 | Variable |
| 15 | k | 17 | 16 | Variable |
| 16 | l | 17 | 19 | Variable |
| 17 | m | 18 | 19 | Variable |
| 18 | - | - | - | Terminal(0) |
| 19 | - | - | - | Terminal(1) |

**Note**: Nodes are ordered topologically (parents before children) with terminal nodes at the end.
