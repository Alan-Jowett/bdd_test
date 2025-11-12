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
    N0(("var1"))
    N1(("var3"))
    N2(("var4"))
    N3(("var5"))
    N4["0"]
    N5(("var6"))
    N6(("var7"))
    N7["1"]
    N8(("var7"))
    N9(("var4"))
    N10(("var2"))

    N0 -.-> N1
    N1 -.-> N2
    N2 -.-> N3
    N3 -.-> N4
    N3 --> N5
    N5 -.-> N6
    N6 --> N7
    N5 --> N8
    N1 --> N9
    N0 --> N10

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
```

## Analysis Summary

- **Variables**: 7
- **BDD Nodes**: 8
- **Expression**: (var1 AND var2) OR (NOT var3) XOR (var4 OR var5) AND (NOT (var6 XOR var7))

## BDD Node Table

The following table shows the internal structure of the BDD with node relationships:

| Index | Variable | False Child | True Child | Type |
|-------|----------|-------------|------------|------|
| 0 | var1 | 2 | 1 | Variable |
| 1 | var2 | 2 | 3 | Variable |
| 2 | var3 | 4 | 3 | Variable |
| 3 | var4 | 5 | 6 | Variable |
| 4 | var4 | 5 | 6 | Variable |
| 5 | var5 | 10 | 6 | Variable |
| 6 | var6 | 8 | 7 | Variable |
| 7 | var7 | 10 | 9 | Variable |
| 8 | var7 | 10 | 9 | Variable |
| 9 | - | - | - | Terminal(1) |
| 10 | - | - | - | Terminal(0) |

**Note**: Nodes are ordered topologically (parents before children) with terminal nodes at the end.
