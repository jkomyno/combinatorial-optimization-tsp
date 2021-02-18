# A Comparison of a Mixed Integer Linear Programming Method and a Metaheuristic Method for TSP

-------------------------------------------------------------------------

This project has been written in partial fulfillment of the requirements for the Methods and Models for Combinatorial Optimization class, supervised by [Professor Luigi de Giovanni](https://www.math.unipd.it/~luigi).

## Abstract

In this report, we discuss two different implementations of the symmetric Travelling Salesman Problem in the context of minimizing the time taken by a board drilling machine. The first implementation is an exact Mixed Integer Linear Programming model based on a compact network flow formulation using `IBM CPLEX`.
The second approach uses a metaheuristic method based on Genetic Algorithms, Local Search, and the Farthest Insertion heuristic. Multiple crossover and mutation operators, as well as several strategies for evolving the genetic pool are examined. 
We test our implementations on both real-world datasets and simulated (but hopefully realistic) instances, with sizes ranging from `10` to `1665` nodes. The metaheuristic method is calibrated on a subset of the simulated datasets.
Finally, we present a detailed comparison between the exact and metaheuristic implementations in terms of running times and gaps with respect to the optimal solutions, also confronting the two algorithms with a random baseline method.
We perform a benchmark of our implementations running the same TSP instances $11$ times and collecting relevant about the fluctuations of the given solutions.

## Deliverables

A comprehensive report of the project can be found in [report.pdf](report.pdf).

To visualize the clustering quality plots and tables generated for this report, please refer to the [analysis.ipynb](analysis.ipynb) `Python` notebook.
If Github struggles to load the notebook file, please try viewing the notebook with [nbviewer](https://nbviewer.jupyter.org/github/jkomyno/combinatorial-optimization-tsp/blob/master/analysis.ipynb).

The main `C++17` TSP solvers are in the `ex1-cplex`, `ex2-metaheuristic`, and `random-baseline` folders.

We developed some modules written in Python3 to perform our experiments:

- `python/benchmark`: the module responsible for executing the benchmark of the TSP solvers
- `python/calibrate`: the module responsible for calibrating the hyperparameters of the metaheuristic solver

## Script Execution

### Requirements

- Internet connection
- This repository: `git clone --recurse-submodules https://github.com/jkomyno/combinatorial-optimization-tsp`
- Any `x86-x64` Linux distribution with bash shell support
- `g++` `v7.5.0` or superior
- `Python` `v3.6.9` or superior
- IBM `CPLEX` `v12.8.0`
- `make`

### Compiling the Solvers

We defined a Makefile to include all third-party dependencies and link the required dynamic libraries for our programs written in `C++17`. To create the ex1-cplex.out, ex2-metaheuristic.out, and random-baseline.out files in the `./build` folder, please run:

```
  make all
```

### Running the Solvers

Every solver shares some common command-line options:

- `-f, --filename [FILE]`: Name of the input TSP instance file;
- `-t, --timeout-ms [TIMEOUT]`: Timeout expressed in milliseconds;
- `-s, --show-path`: If specified, it shows the Hamiltonian circuit found by the solver;
- `--help`: If specified, it shows the available command-line options.

By default, the timeout is set to `1000` milliseconds (1 second) and the actual solution path is not shown (only the cost of the solution is reported).

We also provided some convenient scripts for running the solvers.
Command-line arguments are propagated via the `$@` bash expansion.

#### Running `ex1-cplex`

```
  ./scripts/ex1-cplex.sh
```

#### Running `ex2-metaheuristic`

```
  ./scripts/ex2-baseline.sh
```

`ex2-metaheuristic` supports some other command-line options:

- `-m, --mutation-probability [PROBABILITY]`: Probability that a mutation occurs in the genetic algorithm;
- `-c, --crossover-rate [RATE]` Probability that two selected solutions are mated to create a new offspring;
- `--mu [SIZE]`: Size of the population pool;
- `--lambda [SIZE]`: Size of the offspring pool before being pruned;
- `-k [SIZE]`: Size of the tournament selection;
- `-N, --max-gen-no-improvement [NUMBER]`: Maximum number of generations without solution improvement;
- `-M, --max-gen [NUMBER]`: Maximum number of generations.

For each of these custom arguments, we set a default value equal to the result of the metaheuristic calibration phase. Even if the calibration phase is repeated in the future, the default arguments are updated dynamically without having to compile the solver's `C++17` sources again.

#### Running `random-baseline`

```
  ./scripts/random-baseline.sh
```

### Running the `Python` scripts

Please keep in mind that running either the benchmark or calibration scripts will take many hours.
First of all, please install the required third-party dependencies using `pip`:

```
  python3 -m pip install -r ./python/requirements.txt
```

#### Running the Benchmark Script

```
  ./scripts/benchmark.sh
```

#### Running the Calibration Script

```
  ./scripts/calibrate.sh
```

## License

[MIT License](LICENSE).