# BDD Analysis Report

## Original Expression

```
var_with_éxtended_chars AND ñoñ_ascii_123 OR café_münü XOR résumé_naïve
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
    N4(("var_with_éxtended_chars"))
    N5(("ñoñ_ascii_123"))
    N6(("café_münü"))
    N7(("résumé_naïve"))

    N1 --> N2
    N2 --> N3
    N3 --> N4
    N3 --> N5
    N2 --> N6
    N1 --> N7

    class N1 xorOp
    class N2 orOp
    class N3 andOp
    class N4 variable
    class N5 variable
    class N6 variable
    class N7 variable

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
    N1["1"]
    N2(("ñoñ_ascii_123"))
    N3(("var_with_éxtended_chars"))
    N4(("ñoñ_ascii_123"))
    N5(("var_with_éxtended_chars"))
    N6(("résumé_naïve"))
    N7(("résumé_naïve"))
    N8(("café_münü"))

    N2 -.-> N0
    N2 --> N1
    N3 -.-> N0
    N3 --> N2
    N4 -.-> N1
    N4 --> N0
    N5 -.-> N1
    N5 --> N4
    N6 -.-> N3
    N6 --> N5
    N7 -.-> N1
    N7 --> N0
    N8 -.-> N6
    N8 --> N7

    class N0 terminal
    class N1 terminal
    class N2 bddVariable
    class N3 bddVariable
    class N4 bddVariable
    class N5 bddVariable
    class N6 bddVariable
    class N7 bddVariable
    class N8 bddVariable

    classDef bddVariable fill:lightblue,stroke:#333,stroke-width:2px,color:#000
    classDef terminal fill:lightgray,stroke:#333,stroke-width:2px,color:#000
```

## Analysis Summary

- **Variables**: 4
- **BDD Nodes**: 9
- **Expression**: var_with_éxtended_chars AND ñoñ_ascii_123 OR café_münü XOR résumé_naïve

## BDD Node Table

The following table shows the internal structure of the BDD with node relationships:

| Index | Variable | False Child | True Child | Type |
|-------|----------|-------------|------------|------|
| 0 | café_münü | 2 | 1 | Variable |
| 1 | résumé_naïve | 7 | 8 | Variable |
| 2 | résumé_naïve | 5 | 3 | Variable |
| 3 | var_with_éxtended_chars | 7 | 4 | Variable |
| 4 | ñoñ_ascii_123 | 7 | 8 | Variable |
| 5 | var_with_éxtended_chars | 8 | 6 | Variable |
| 6 | ñoñ_ascii_123 | 8 | 7 | Variable |
| 7 | - | - | - | Terminal(1) |
| 8 | - | - | - | Terminal(0) |

**Note**: Nodes are ordered topologically (parents before children) with terminal nodes at the end.
