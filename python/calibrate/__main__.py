import argparse
from . import calibrate

if __name__ == '__main__':
  parser = argparse.ArgumentParser()
  required = parser.add_argument_group('required arguments')

  required.add_argument('--program', type=str, required=True, help='ex2-metaheuristic program to calibrate')
  required.add_argument('--datasets', type=str, required=True, help='folder that contains the training datasets in TSPLIB format')
  required.add_argument('--output', type=str, required=True, help='folder where the results of the benchmark will be written to')
  args = parser.parse_args()

  calibrate(args)
