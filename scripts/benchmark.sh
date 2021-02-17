#!/bin/bash

set -e

PROGRAMS_FOLDER=./
DATASETS_FOLDER=./tsp-datasets/benchmark/
OUTPUT_FOLDER=./results/benchmark/

mkdir -p $OUTPUT_FOLDER;

function benchmark {
  PROGRAM=$1

  python -m python.benchmark \
    --program "${PROGRAMS_FOLDER}${PROGRAM}" \
    --datasets "${DATASETS_FOLDER}" \
    --output "${OUTPUT_FOLDER}" \
    "${PROGRAM}"
}

benchmark 'ex1-cplex';
benchmark 'ex2-metaheuristic';
benchmark 'random-baseline';
