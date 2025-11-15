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
    N1(("a"))
    N10["OR"]
    N11(("g"))
    N12(("h"))
    N13["XOR"]
    N14["AND"]
    N15["XOR"]
    N16(("i"))
    N17["NOT"]
    N18(("j"))
    N19["OR"]
    N2(("b"))
    N20(("k"))
    N21(("l"))
    N22["AND"]
    N23(("m"))
    N24["NOT"]
    N25["OR"]
    N26["XOR"]
    N27["AND"]
    N3["AND"]
    N4(("c"))
    N5(("d"))
    N6["AND"]
    N7["OR"]
    N8(("e"))
    N9(("f"))

    N10 --> N8
    N10 --> N9
    N13 --> N11
    N13 --> N12
    N14 --> N10
    N14 --> N13
    N15 --> N14
    N15 --> N7
    N17 --> N16
    N19 --> N17
    N19 --> N18
    N22 --> N20
    N22 --> N21
    N24 --> N23
    N25 --> N22
    N25 --> N24
    N26 --> N19
    N26 --> N25
    N27 --> N15
    N27 --> N26
    N3 --> N1
    N3 --> N2
    N6 --> N4
    N6 --> N5
    N7 --> N3
    N7 --> N6

    class N1 variable
    class N2 variable
    class N3 andOp
    class N4 variable
    class N5 variable
    class N6 andOp
    class N7 orOp
    class N8 variable
    class N9 variable
    class N10 orOp
    class N11 variable
    class N12 variable
    class N13 xorOp
    class N14 andOp
    class N15 xorOp
    class N16 variable
    class N17 notOp
    class N18 variable
    class N19 orOp
    class N20 variable
    class N21 variable
    class N22 andOp
    class N23 variable
    class N24 notOp
    class N25 orOp
    class N26 xorOp
    class N27 andOp

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
title: BDD
---
flowchart TD
    N0["0"]
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
    N1["1"]
    N2(("m"))
    N20(("b"))
    N21(("a"))
    N3(("l"))
    N4(("k"))
    N5(("m"))
    N6(("l"))
    N7(("k"))
    N8(("j"))
    N9(("i"))

    N10 --> N9
    N10 -.-> N0
    N11 --> N0
    N11 -.-> N9
    N12 --> N11
    N12 -.-> N10
    N13 --> N12
    N13 -.-> N0
    N14 --> N12
    N14 -.-> N13
    N15 --> N10
    N15 -.-> N11
    N16 --> N15
    N16 -.-> N9
    N17 --> N15
    N17 -.-> N16
    N18 --> N17
    N18 -.-> N14
    N19 --> N18
    N19 -.-> N14
    N2 --> N1
    N2 -.-> N0
    N20 --> N17
    N20 -.-> N19
    N21 --> N20
    N21 -.-> N19
    N3 --> N0
    N3 -.-> N2
    N4 --> N3
    N4 -.-> N2
    N5 --> N0
    N5 -.-> N1
    N6 --> N1
    N6 -.-> N5
    N7 --> N6
    N7 -.-> N5
    N8 --> N4
    N8 -.-> N7
    N9 --> N8
    N9 -.-> N4

    class N0 terminal
    class N1 terminal
    class N2 bddVariable
    class N3 bddVariable
    class N4 bddVariable
    class N5 bddVariable
    class N6 bddVariable
    class N7 bddVariable
    class N8 bddVariable
    class N9 bddVariable
    class N10 bddVariable
    class N11 bddVariable
    class N12 bddVariable
    class N13 bddVariable
    class N14 bddVariable
    class N15 bddVariable
    class N16 bddVariable
    class N17 bddVariable
    class N18 bddVariable
    class N19 bddVariable
    class N20 bddVariable
    class N21 bddVariable

    classDef bddVariable fill:lightblue,stroke:#333,stroke-width:2px,color:#000
    classDef terminal fill:lightgray,stroke:#333,stroke-width:2px,color:#000
```

## Analysis Summary

- **Variables**: 13
- **BDD Nodes**: 22
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
| 8 | f | 21 | 9 | Variable |
| 9 | g | 11 | 10 | Variable |
| 10 | h | 12 | 21 | Variable |
| 11 | h | 21 | 12 | Variable |
| 12 | i | 17 | 13 | Variable |
| 13 | j | 14 | 17 | Variable |
| 14 | k | 16 | 15 | Variable |
| 15 | l | 16 | 20 | Variable |
| 16 | m | 20 | 21 | Variable |
| 17 | k | 19 | 18 | Variable |
| 18 | l | 19 | 21 | Variable |
| 19 | m | 21 | 20 | Variable |
| 20 | - | - | - | Terminal(1) |
| 21 | - | - | - | Terminal(0) |

**Note**: Nodes are ordered topologically (parents before children) with terminal nodes at the end.
