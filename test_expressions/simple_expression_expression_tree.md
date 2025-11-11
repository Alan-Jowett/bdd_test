---
title: Expression Tree
---
flowchart TD
    N1["OR"]
    N2["AND"]
    N3(("a"))
    N4(("b"))
    N5["AND"]
    N6(("c"))
    N7(("d"))

    N1 --> N2
    N2 --> N3
    N2 --> N4
    N1 --> N5
    N5 --> N6
    N5 --> N7
