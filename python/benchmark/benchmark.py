import subprocess
from itertools import islice
from collections import defaultdict
from os import path
from pathlib import Path
from glob import iglob
from typing import Iterable

import pandas as pd
from . import ParameterGrid


TIMEOUT_MS_LIST = [
  100,     # 0.1 seconds
  1000,    # 1 second
  10000,   # 10 second
  60000,   # 1 minute
]

grid_params_ex1_cplex = {
  'timeout_ms': TIMEOUT_MS_LIST,
  'trials': [11],
}

grid_params_ex2_metaheuristic = {
  'timeout_ms': TIMEOUT_MS_LIST,
  'trials': [11],
}

grid_params_random_baseline = {
  'timeout_ms': TIMEOUT_MS_LIST,
  'trials': [11],
}

grid_params = {
  'ex1-cplex': grid_params_ex1_cplex,
  'ex2-metaheuristic': grid_params_ex2_metaheuristic,
  'random-baseline': grid_params_random_baseline,
}

df_column_names_ex1_cplex = [
  'dataset',
  'N',
  'solution',
  'program_time_ms',
  'was_interrupted',
  'timeout_ms',
]

df_column_names_ex2_metaheuristic = [
  'dataset',
  'N',
  'solution',
  'best_of_generations',
  'heuristic_cost',
  'initial_best_cost',
  'initial_improved_cost',
  'timeout_ms',
]

df_column_names_random_baseline = [
  'dataset',
  'N',
  'solution',
  'program_time_ms',
  'was_interrupted',
  'timeout_ms',
]

df_column_names = {
  'ex1-cplex': df_column_names_ex1_cplex,
  'ex2-metaheuristic': df_column_names_ex2_metaheuristic,
  'random-baseline': df_column_names_random_baseline,
}


def compose(f):
  def compose_helper(g):
    return lambda x: f(g(x))
  return compose_helper


def parse_ex1_cplex_output(stdout: Iterable):
  # pairs of (key, parser). Parser is a function that receives a string in input.
  line_to_key_parser = [
    ('N', int),
    ('program_time_ms', int),
    ('was_interrupted', lambda x: True if int(x) > 0 else False),
  ]

  parse_result = defaultdict(dict)

  for j, line in enumerate(islice(stdout, 3)):
    value = line.split(': ')[1]
    key, value_parser = line_to_key_parser[j]
    parse_result[key] = value_parser(value)

  line = next(stdout)
  line_split = line.split(': ')

  if len(line_split) is 0:
    # no solution has been found
    parse_result['solution'] = None
  else:
    parse_result['solution'] = compose(int)(float)(line_split[1])
  
  return parse_result


def parse_ex2_metaheuristic_output(stdout: Iterable):
  # pairs of (key, parser). Parser is a function that receives a string in input.
  line_to_key_parser = [
    ('N', int),
    ('heuristic_cost', int),
    ('initial_best_cost', int),
    ('initial_improved_cost', int),
    ('best_of_generations', int),
    ('solution', compose(int)(float)),
  ]

  parse_result = defaultdict(dict)

  for j, line in enumerate(stdout):
    if j < 4:
      value = line.split(': ')[1]
      key, value_parser = line_to_key_parser[j]
      parse_result[key] = value_parser(value)
    else:
      break
  
  next(stdout)

  for line in stdout:
    if not line.lstrip().startswith('#'):
      break
  
  for j, line in enumerate(stdout, start=4):
    value = line.split(': ')[1]
    key, value_parser = line_to_key_parser[j]
    parse_result[key] = value_parser(value)

  return parse_result


def parse_random_baseline_output(stdout: Iterable):
  # pairs of (key, parser). Parser is a function that receives a string in input.
  line_to_key_parser = [
    ('N', int),
    ('program_time_ms', int),
    ('was_interrupted', lambda x: True if int(x) > 0 else False),
    ('solution', compose(int)(float)),
  ]

  parse_result = defaultdict(dict)

  for j, line in enumerate(stdout):
    value = line.split(': ')[1]
    key, value_parser = line_to_key_parser[j]
    parse_result[key] = value_parser(value)
  
  return parse_result


def benchmark_ex1_cplex(args, dataset: str, df: pd.DataFrame,
                        timeout_ms: int, trials: int):
  dataset_name = Path(dataset).stem

  for i in range (1, trials + 1):
    cmd = list(map(str, [args.program, timeout_ms, dataset]))

    # execute program and capture its output
    with subprocess.Popen(cmd, stdout=subprocess.PIPE, bufsize=1,
                          universal_newlines=True) as p:
      parse_result = parse_ex1_cplex_output(p.stdout)

    df = df.append({
      **parse_result,
      'dataset': dataset_name,
      'timeout_ms': timeout_ms,
    }, ignore_index=True)

  return df


def benchmark_ex2_metaheuristic(args, dataset: str, df: pd.DataFrame,
                                timeout_ms: int, trials: int):
  dataset_name = Path(dataset).stem

  for i in range (1, trials + 1):
    cmd = list(map(str, [args.program, timeout_ms, dataset]))

    # execute program and capture its output
    with subprocess.Popen(cmd, stdout=subprocess.PIPE, bufsize=1,
                          universal_newlines=True) as p:
      parse_result = parse_ex2_metaheuristic_output(p.stdout)

    df = df.append({
      **parse_result,
      'dataset': dataset_name,
      'timeout_ms': timeout_ms,
    }, ignore_index=True)
  return df


def benchmark_random_baseline(args, dataset: str, df: pd.DataFrame,
                              timeout_ms: int, trials: int):
  dataset_name = Path(dataset).stem

  for i in range (1, trials + 1):
    cmd = list(map(str, [args.program, timeout_ms, dataset]))

    # execute program and capture its output
    with subprocess.Popen(cmd, stdout=subprocess.PIPE, bufsize=1,
                          universal_newlines=True) as p:
      parse_result = parse_random_baseline_output(p.stdout)

    df = df.append({
      **parse_result,
      'dataset': dataset_name,
      'timeout_ms': timeout_ms,
    }, ignore_index=True)

  return df


def benchmark(args):
  """
  args.program_type: 'ex1-cplex' | 'ex2-metaheuristic' | 'random-baseline'
  args.program: str
  args.datasets: str
  args.output: str
  """

  print(f'args.program_type: {args.program_type}')

  grid = ParameterGrid(grid_params[args.program_type])
  df = pd.DataFrame(columns=df_column_names[args.program_type])

  create_df_map = {
    'ex1-cplex': benchmark_ex1_cplex,
    'ex2-metaheuristic': benchmark_ex2_metaheuristic,
    'random-baseline': benchmark_random_baseline,
  }

  for dataset in iglob(path.join(args.datasets, '*.tsp')):
    print(f'dataset: {dataset}')
    for i, comb in enumerate(grid):
      print(f'**comb: {comb}')
      print(f'Iteration #{i+1} started...\n')

      df = create_df_map[args.program_type](args, dataset, df, **comb)

      print(f'Iteration #{i+1} completed!\n\n')

    # break

  df.to_csv(path.join(args.output, f'{args.program_type}.csv'), sep=',', index=False,
            encoding='utf-8', decimal='.')
