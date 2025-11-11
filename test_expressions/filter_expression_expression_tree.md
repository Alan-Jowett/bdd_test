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
