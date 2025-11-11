# BDD Analysis Report

## Original Expression

```
(var1 AND var2) OR (NOT var3) XOR (var4 OR var5) AND (NOT (var6 XOR var7))
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
    N4(("var1"))
    N5(("var2"))
    N6["NOT"]
    N7(("var3"))
    N8["AND"]
    N9["OR"]
    N10(("var4"))
    N11(("var5"))
    N12["NOT"]
    N13["XOR"]
    N14(("var6"))
    N15(("var7"))

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
    N8 --> N12
    N12 --> N13
    N13 --> N14
    N13 --> N15

    class N1 xorOp
    class N2 orOp
    class N3 andOp
    class N4 variable
    class N5 variable
    class N6 notOp
    class N7 variable
    class N8 andOp
    class N9 orOp
    class N10 variable
    class N11 variable
    class N12 notOp
    class N13 xorOp
    class N14 variable
    class N15 variable

    classDef variable fill:lightblue,stroke:#333,stroke-width:2px
    classDef andOp fill:lightgreen,stroke:#333,stroke-width:2px
    classDef orOp fill:lightcoral,stroke:#333,stroke-width:2px
    classDef notOp fill:yellow,stroke:#333,stroke-width:2px
    classDef xorOp fill:lightpink,stroke:#333,stroke-width:2px
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
    N2(("var7"))
    N3(("var7"))
    N4(("var6"))
    N5(("var5"))
    N6(("var4"))
    N7(("var6"))
    N8(("var5"))
    N9(("var4"))
    N10(("var3"))
    N11(("var2"))
    N12(("var1"))

    N2 -.-> N1
    N2 --> N0
    N3 -.-> N0
    N3 --> N1
    N4 -.-> N2
    N4 --> N3
    N5 -.-> N0
    N5 --> N4
    N6 -.-> N5
    N6 --> N4
    N7 -.-> N3
    N7 --> N2
    N8 -.-> N1
    N8 --> N7
    N9 -.-> N8
    N9 --> N7
    N10 -.-> N6
    N10 --> N9
    N11 -.-> N10
    N11 --> N6
    N12 -.-> N10
    N12 --> N11

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

    classDef bddVariable fill:lightblue,stroke:#333,stroke-width:2px
    classDef terminal fill:lightgray,stroke:#333,stroke-width:2px
```

## Analysis Summary

- **Variables**: 7
- **BDD Nodes**: 13
- **Expression**: (var1 AND var2) OR (NOT var3) XOR (var4 OR var5) AND (NOT (var6 XOR var7))

## BDD Node Table

The following table shows the internal structure of the BDD with node relationships:

| Index | Variable | False Child | True Child | Type |
|-------|----------|-------------|------------|------|
| 0 | var1 | 2 | 1 | Variable |
| 1 | var2 | 2 | 6 | Variable |
| 2 | var3 | 6 | 3 | Variable |
| 3 | var4 | 4 | 5 | Variable |
| 4 | var5 | 11 | 5 | Variable |
| 5 | var6 | 9 | 10 | Variable |
| 6 | var4 | 7 | 8 | Variable |
| 7 | var5 | 12 | 8 | Variable |
| 8 | var6 | 10 | 9 | Variable |
| 9 | var7 | 12 | 11 | Variable |
| 10 | var7 | 11 | 12 | Variable |
| 11 | - | - | - | Terminal(0) |
| 12 | - | - | - | Terminal(1) |

**Note**: Nodes are ordered topologically (parents before children) with terminal nodes at the end.