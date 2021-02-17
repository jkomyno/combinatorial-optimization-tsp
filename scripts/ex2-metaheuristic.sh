#!/bin/bash

set -e

CALIBRATION_VAR=$(awk 'NR == 2 {print $0}' ./results/calibration/calibration_variables.csv)
CALIBRATION_FIN=$(awk 'NR == 2 {print $0}' results/calibration/calibration_final_tuning.csv)

MUTATION_PROBABILITY=$(echo ${CALIBRATION_VAR} | cut -d, -f1)
CROSSOVER_RATE=$(echo ${CALIBRATION_VAR} | cut -d, -f2)
MU=$(echo ${CALIBRATION_VAR} | cut -d, -f3)
LAMBDA=$(echo ${CALIBRATION_VAR} | cut -d, -f4)
K=$(echo ${CALIBRATION_VAR} | cut -d, -f5)

MAX_GEN_NO_IMPROVEMENT=$(echo ${CALIBRATION_FIN} | cut -d, -f1)
MAX_GEN=$(echo ${CALIBRATION_FIN} | cut -d, -f2)

./build/ex2-metaheuristic.out $@
