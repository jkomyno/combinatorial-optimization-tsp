#!/bin/bash

set -e

PROGRAMS_FOLDER=./
DATASETS_FOLDER=./tsp-datasets/calibration/
OUTPUT_FOLDER=./results/calibration/

mkdir -p $OUTPUT_FOLDER;

function calibrate {
  PROGRAM=$1

  python -m python.calibrate \
    --program "${PROGRAMS_FOLDER}${PROGRAM}" \
    --datasets "${DATASETS_FOLDER}" \
    --output "${OUTPUT_FOLDER}"
}

calibrate 'ex2-metaheuristic';
