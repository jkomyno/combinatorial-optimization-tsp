from . import utils
import math
import itertools
import subprocess
import pandas as pd
import numpy as np

from os import path
from glob import iglob
from pymoo.algorithms.nsga2 import NSGA2
from pymoo.operators.mixed_variable_operator import MixedVariableSampling
from pymoo.operators.mixed_variable_operator import MixedVariableMutation
from pymoo.operators.mixed_variable_operator import MixedVariableCrossover
from pymoo.model.problem import Problem
from pymoo.factory import get_sampling
from pymoo.factory import get_crossover
from pymoo.factory import get_mutation


TIMEOUT_MS = 60000
ITERATIONS_TIME = '06:00:00'
EXECUTIONS_FOR_ITERATION = 50


var2idx = {
  var: i for i, var in enumerate(['max_gen_no_improvement',
                                  'max_gen'])
}


def get_var(x, var):
  return x[var2idx[var]]


mask = [
  'int',   # (0) max_gen_no_improvement
  'int',   # (1) max_gen
]

bounds = [
  (50, 200),          # (0) max_gen_no_improvement
  (50, 500),          # (1) max_gen
]

lower_bounds, upper_bounds = utils.lower_upper_bounds(bounds)


class MetaProblem(Problem):
  def __init__(self, args, **kwargs):
    super().__init__(n_var=len(mask),
                     n_obj=2,
                     n_constr=2,
                     xl=lower_bounds,
                     xu=upper_bounds,
                     elementwise_evaluation=True,
                     **kwargs)
    self.args = args
    
  def _evaluate(self, x, out, *args, **kwargs):
    ##########################
    # minimization functions #
    ##########################

    # we want to minimize our average solutions and the probability of spikes in those solutions
    average, stddev = run_ex2_metaheuristic(self.args, x)

    ###############
    # constraints #
    ###############

    max_gen_no_improvement = get_var(x, 'max_gen_no_improvement')
    max_gen = get_var(x, 'max_gen')

    # max_gen_no_improvement < max_gen
    max_gen_no_improvementt_lt_max_gen = max_gen_no_improvement - max_gen - 1

    ###########
    # outputs #
    ###########

    # 'F' represents the problem objectives
    out['F'] = [average, stddev]

    # 'G' represents the problem constraints in the form g(x) <= 0
    out['G'] = [max_gen_no_improvementt_lt_max_gen]


def get_ex2_metaheuristic_result(args, dataset: str, max_gen_no_improvement: int, max_gen: int):
  timeout_ms = TIMEOUT_MS
  cmd = list(map(str, [
    args.program,
    '--timeout-ms', timeout_ms,
    '--filename', dataset,
    '--max-gen-no-improvement', max_gen_no_improvement,
    '--max-gen', max_gen,
  ]))

  # execute program and capture its output
  with subprocess.Popen(cmd, stdout=subprocess.PIPE, bufsize=1,
                        universal_newlines=True) as p:
    stdout = itertools.islice(p.stdout, 4, None)

    for line in stdout:
      if not line.lstrip().startswith('#'):
        break
    
    stdout = itertools.islice(stdout, 4, None)
    solution_raw = next(stdout).split(': ')[1]
    solution = int(float(solution_raw))

  return solution


def run_ex2_metaheuristic(args, x):
  max_gen_no_improvement = get_var(x, 'max_gen_no_improvement')
  max_gen = get_var(x, 'max_gen')

  if max_gen_no_improvement >= max_gen:
    return math.inf, math.inf
  
  solutions = [
    get_ex2_metaheuristic_result(args, dataset, max_gen_no_improvement, max_gen)
    for dataset in iglob(path.join(args.datasets, '*.tsp'))
  ]

  average = np.mean(solutions)
  stddev = np.std(solutions)

  return average, stddev


def calibrate_final_tuning(args, pool):
  sampling = MixedVariableSampling(mask, {
    # Integer numbers are sampled via Uniform Random Sampling
    'int': get_sampling('int_random')
  })

  crossover = MixedVariableCrossover(mask, {
    'int': get_crossover('real_sbx', prob=0.9, eta=3.0)
  })

  mutation = MixedVariableMutation(mask, {
    # Integer numbers are mutated via polynomial mutation
    'int': get_mutation('int_pm', eta=3.0)
  })
  
  problem = MetaProblem(args, parallelization=None)
  algorithm = NSGA2(
    pop_size=EXECUTIONS_FOR_ITERATION,
    sampling=sampling,
    crossover=crossover,
    mutation=mutation,
    eliminate_duplicates=True,
  )
  termination = utils.get_termination_for_final_tuning(time=ITERATIONS_TIME)

  res, \
    best_solution, \
    min_average, \
    min_stddev = utils.get_minimizer(problem, algorithm, termination)
  
  save_csv(args, best_solution, min_average, min_stddev)


def save_csv(args, best_solution, min_average, min_stddev):
  max_gen_no_improvement = get_var(best_solution, 'max_gen_no_improvement')
  max_gen = get_var(best_solution, 'max_gen')

  print(f'max_gen_no_improvement: {max_gen_no_improvement}')
  print(f'max_gen: {max_gen}')

  print(f'min_average: {min_average}')
  print(f'min_stddev: {min_stddev}')

  df = pd.DataFrame.from_records([{
    'max_gen_no_improvement': max_gen_no_improvement,
    'max_gen': max_gen,
    'min_average': min_average,
    'min_stddev': min_stddev,
  }])
  
  df.to_csv(path.join(args.output, f'calibration_final_tuning.csv'), sep=',', index=False,
            encoding='utf-8', decimal='.')
