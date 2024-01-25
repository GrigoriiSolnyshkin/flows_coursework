# How to use

First, compile. For example, 
```bash
g++-12 main.cpp -o main
```

Then you can solve flow problem in interactive mode
```bash
./main interactive
```
or you can perform time measurements on different maxflow algorithms.

Currently, two types of tests are available: `akc-hard` 
(special test with linear number of edges where Dinic's algorithm is not very comfortable)
and `random-full` (full graph with random edges capacities),
and four algorithms: `dinics` (Dinic's algorithm without any heuristics),
`edmonds` (Edmonds-Karp algorithm), `linkcut` (Dinic's algorithm using linkcut tree),
`scaled-dinics` (Dinic's with scaling).

Usage:
```bash
./main timeit algorithm generation-method relative-size-of-test
```

In this coursework the Stoer-Wagner algorithm is implemented as well
(feel free to check [implementation](stoer_wagner.hpp) and [tests](unit-tests/stoer_wagner_tests.cpp)).

# Results
Dinic's implemented with linkcut effectively does not speed up the algorithm:

| **Generation method** | linkcut | dinics | scaled-dinics | edmonds |
|-----------------------|---------|--------|---------------|---------|
| Full random (200)     | 13ms    | 9ms    | 42ms          | 700ms   |
| Full random (500)     | 76ms    | 50ms   | 223ms         | 11500ms |
| Full random (1000)    | 362ms   | 241ms  | 1400ms        | 85600ms |
| Full random (2000)    | 1500ms  | 952ms  | 5400ms        | -       |
| Full random (5000)    | 6900ms  | 5300ms | 33400ms       | -       |
| Hard (100)            | 86ms    | 20ms   | 23ms          | 14ms    |
| Hard (200)            | 342ms   | 69ms   | 76ms          | 45ms    |
| Hard (500)            | 2174ms  | 403ms  | 451ms         | 257ms   |
| Hard (1000)           | 8558ms  | 1593ms | 1710ms        | 1037ms  |
| Hard (1500)           | 19241ms | 3631ms | 3852ms        | 2289ms  |
| Hard (2000)           | 34227ms | 6586ms | 6923ms        | 4220ms  |

