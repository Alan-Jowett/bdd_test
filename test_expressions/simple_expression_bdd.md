---
title: BDD
---
flowchart TD
    N0["0"]
    N1["1"]
    N2(("d"))
    N3(("c"))
    N4(("b"))
    N5(("a"))

    N2 -.-> N0
    N2 --> N1
    N3 -.-> N0
    N3 --> N2
    N4 -.-> N3
    N4 --> N1
    N5 -.-> N3
    N5 --> N4
