import math
import itertools
import subprocess
import multiprocessing
import numpy as np
import pandas as pd

from os import path
from glob import iglob
from pymoo.factory import get_sampling, get_crossover, get_mutation
from pymoo.algorithms.nsga2 import NSGA2
from pymoo.optimize import minimize
from pymoo.visualization.scatter import Scatter
from pymoo.model.problem import Problem
from pymoo.operators.mixed_variable_operator import MixedVariableSampling
from pymoo.operators.mixed_variable_operator import MixedVariableMutation
from pymoo.operators.mixed_variable_operator import MixedVariableCrossover

var2idx = {
  var: i for i, var in enumerate(['mutation_probability',
                                  'crossover_rate',
                                  'mu',
                                  'lambda',
                                  'k'])
}


def get_var(x, var):
  return x[var2idx[var]]


mask = [
  'real',  # (0) mutation_probability
  'real',  # (1) crossover_rate
  'int',   # (2) mu
  'int',   # (3) lambda
  'int',   # (4) k
]

bounds = [
  [0.005, 0.02],  # (0) mutation_probability
  [0.7, 1],       # (1) crossover_rate
  [25, 50],       # (2) mu
  [30, 75],       # (3) lambda
  [8, 14],        # (4) k
]

lower_bounds = np.fromiter(map(lambda x: x[0], bounds), dtype=np.float32)
upper_bounds = np.fromiter(map(lambda x: x[1], bounds), dtype=np.float32)


def get_ex2_metaheuristic_result(args, dataset: str, mutation_probability: float,
                                 crossover_rate: float, mu: int, lambda_: int, k: int):
  timeout_ms = 1000
  cmd = list(map(str, [
    args.program,
    '--timeout-ms', timeout_ms,
    '--filename', dataset,
    '--mutation-probability', mutation_probability,
    '--crossover-rate', crossover_rate,
    '--mu', mu,
    '--lambda', lambda_,
    '-k', k,
  ]))

  # execute program and capture its output
  with subprocess.Popen(cmd, stdout=subprocess.PIPE, bufsize=1,
                        universal_newlines=True) as p:
    stdout = itertools.islice(p.stdout, 5, None)

    for line in stdout:
      if not line.lstrip().startswith('#'):
        break
    
    stdout = itertools.islice(stdout, 1, None)
    solution_raw = next(stdout).split(': ')[1]
    solution = int(float(solution_raw))

  return solution


def run_ex2_metaheuristic(args, x):
  mutation_probability = get_var(x, 'mutation_probability')
  crossover_rate = get_var(x, 'crossover_rate')
  mu = get_var(x, 'mu')
  lambda_ = get_var(x, 'lambda')
  k = get_var(x, 'k')

  if mu >= lambda_ or k >= lambda_:
    return math.inf, math.inf
  
  solutions = [
    get_ex2_metaheuristic_result(args, dataset, mutation_probability,
                                 crossover_rate, mu, lambda_, k)
    for dataset in iglob(path.join(args.datasets, '*.tsp'))
  ]

  average = np.mean(solutions)
  stddev = np.std(solutions)

  return average, stddev


class MetaProblem(Problem):
  def __init__(self, args, **kwargs):
    super().__init__(n_var=len(mask),
                     n_obj=2,
                     n_constr=1,
                     xl=lower_bounds,
                     xu=upper_bounds,
                     elementwise_evaluation=True,
                     **kwargs)
    self.args = args
    
  def _evaluate(self, x, out, *args, **kwargs):
    ##########################
    # minimization functions #
    ##########################

    average, stddev = run_ex2_metaheuristic(self.args, x)

    ###############
    # constraints #
    ###############

    # mu < lambda
    mu_lt_lambda = get_var(x, 'mu') - get_var(x, "lambda") - 1

    # k < lambda
    k_lt_lambda = get_var(x, "k") - get_var(x, "lambda") - 1

    out['F'] = [average, stddev]
    out['G'] = [mu_lt_lambda, k_lt_lambda]


def calibrate(args):
  sampling = MixedVariableSampling(mask, {
    'real': get_sampling('real_random'),
    'int': get_sampling('int_random')
  })

  crossover = MixedVariableCrossover(mask, {
    'real': get_crossover('real_sbx', prob=0.85, eta=3.0),
    'int': get_crossover('int_sbx', prob=0.85, eta=3.0)
  })

  mutation = MixedVariableMutation(mask, {
    'real': get_mutation('real_pm', eta=3.0),
    'int': get_mutation('int_pm', eta=3.0)
  })

  algorithm = NSGA2(
      pop_size=30,
      sampling=sampling,
      crossover=crossover,
      mutation=mutation,
      eliminate_duplicates=True,
  )

  # instantiate parallel problem optimization
  n_threads = multiprocessing.cpu_count()
  pool = multiprocessing.pool.ThreadPool(n_threads)
  problem = MetaProblem(args, parallelization=('starmap', pool.starmap))

  res = minimize(problem,
                 algorithm,
                 ('n_gen', 15),
                 return_least_infeasible=False,
                 verbose=True,
                 seed=42,
                 save_history=True)

  best_solution = res.X[0]

  mutation_probability = get_var(best_solution, 'mutation_probability')
  crossover_rate = get_var(best_solution, 'crossover_rate')
  mu = get_var(best_solution, 'mu')
  lambda_ = get_var(best_solution, 'lambda')
  k = get_var(best_solution, 'k')

  min_average = res.F[0][0]
  min_stddev = res.F[0][1]

  print(f'min_average: {min_average}')
  print(f'min_stddev: {min_stddev}')

  print(f'mutation_probability: {mutation_probability}')
  print(f'crossover_rate: {crossover_rate}')
  print(f'mu: {mu}')
  print(f'lambda: {lambda_}')
  print(f'k: {k}')

  df = pd.DataFrame(data=[best_solution], columns=[
    'mutation_probability',
    'crossover_rate',
    'mu',
    'lambda',
    'k'])

  print(df)
  
  df.to_csv(path.join(args.output, f'calibration.csv'), sep=',', index=False,
          encoding='utf-8', decimal='.')
