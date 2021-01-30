import argparse
from . import benchmark

if __name__ == '__main__':
  parser = argparse.ArgumentParser()
  required = parser.add_argument_group('required arguments')

  subparsers = parser.add_subparsers()

  ex1_cplex_parser = subparsers.add_parser('ex1-cplex', help='benchmark the ex1-cplex program')
  ex1_cplex_parser.set_defaults(program_type='ex1-cplex')

  ex2_metaheuristic_parser = subparsers.add_parser('ex2-metaheuristic', help='benchmark the ex2-metaheuristic program')
  ex2_metaheuristic_parser.set_defaults(program_type='ex2-metaheuristic')

  random_baseline_parser = subparsers.add_parser('random-baseline', help='benchmark the random-baseline program')
  random_baseline_parser.set_defaults(program_type='random-baseline')

  required.add_argument('--program', type=str, required=True, help='program executable to benchmark')
  required.add_argument('--datasets', type=str, required=True, help='folder that contains the datasets in TSPLIB format')
  required.add_argument('--output', type=str, required=True, help='folder where the results of the benchmark will be written to')
  args = parser.parse_args()

  benchmark(args)
