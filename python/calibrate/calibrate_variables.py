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


TIMEOUT_MS = 2000
MAX_ITERATIONS = 25
EXECUTIONS_FOR_ITERATION = 30


var2idx = {
  var: i for i, var in enumerate(['mutation_probability',
                                  'crossover_rate',
                                  'mu',
                                  'lambda',
                                  'k'])
}


def get_var(x, var):
  value = x[var2idx[var]]
  if var == 'mu' or var == 'lambda':
    return 2 * value
  return value


mask = [
  'real',  # (0) mutation_probability
  'real',  # (1) crossover_rate
  'int',   # (2) mu
  'int',   # (3) lambda
  'int',   # (4) k
]

bounds = [
  (0.01, 0.02),       # (0) mutation_probability
  (0.8, 1),           # (1) crossover_rate
  (13, 25),           # (2) mu / 2
  (15, 38),           # (3) lambda / 2
  (10, 14),           # (4) k
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

    v_mu = get_var(x, 'mu')
    v_lambda = get_var(x, 'lambda')
    v_k = get_var(x, 'k')

    # mu < lambda
    mu_lt_lambda = v_mu - v_lambda - 1

    # k < lambda
    k_lt_lambda = v_k - v_lambda - 1

    ###########
    # outputs #
    ###########

    # 'F' represents the problem objectives
    out['F'] = [average, stddev]

    # 'G' represents the problem constraints in the form g(x) <= 0
    out['G'] = [mu_lt_lambda, k_lt_lambda]


def get_ex2_metaheuristic_result(args, dataset: str, mutation_probability: float,
                                 crossover_rate: float, mu: int, lambda_: int, k: int):
  timeout_ms = TIMEOUT_MS
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
    stdout = itertools.islice(p.stdout, 4, None)

    for line in stdout:
      if not line.lstrip().startswith('#'):
        break
    
    stdout = itertools.islice(stdout, 3, None)
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


def calibrate_variables(args, pool):
  sampling = MixedVariableSampling(mask, {
    # Real numbers are sampled via Latin Hypercube Sampling
    'real': get_sampling('real_lhs'),

    # Integer numbers are sampled via Uniform Random Sampling
    'int': get_sampling('int_random')
  })

  crossover = MixedVariableCrossover(mask, {
    'real': get_crossover('real_sbx', prob=0.9, eta=3.0),
    'int': get_crossover('real_sbx', prob=0.9, eta=3.0)
  })

  mutation = MixedVariableMutation(mask, {
    # Real numbers are mutated via polynomial mutation
    'real': get_mutation('real_pm', eta=3.0),

    # Integer numbers are mutated via polynomial mutation
    'int': get_mutation('int_pm', eta=3.0)
  })

  problem = MetaProblem(args, parallelization=('starmap', pool.starmap))
  algorithm = NSGA2(
    pop_size=EXECUTIONS_FOR_ITERATION,
    sampling=sampling,
    crossover=crossover,
    mutation=mutation,
    eliminate_duplicates=True,
  )
  termination = utils.get_termination_for_variables(max_iterations=MAX_ITERATIONS)

  res, \
    best_solution, \
    min_average, \
    min_stddev = utils.get_minimizer(problem, algorithm, termination)

  best_solution = res.X[0]
  min_average = res.F[0][0]
  min_stddev = res.F[0][1]
  
  save_csv(args, best_solution, min_average, min_stddev)


def save_csv(args, best_solution, min_average, min_stddev):
  mutation_probability = get_var(best_solution, 'mutation_probability')
  crossover_rate = get_var(best_solution, 'crossover_rate')
  mu = get_var(best_solution, 'mu')
  lambda_ = get_var(best_solution, 'lambda')
  k = get_var(best_solution, 'k')

  print(f'mutation_probability: {mutation_probability}')
  print(f'crossover_rate: {crossover_rate}')
  print(f'mu: {mu}')
  print(f'lambda: {lambda_}')
  print(f'k: {k}')

  print(f'min_average: {min_average}')
  print(f'min_stddev: {min_stddev}')

  df = pd.DataFrame.from_records([{
    'mutation_probability': mutation_probability,
    'crossover_rate': crossover_rate,
    'mu': mu,
    'lambda': lambda_,
    'k': k,
    'min_average': min_average,
    'min_stddev': min_stddev,
  }])
  
  df.to_csv(path.join(args.output, f'calibration_variables.csv'), sep=',', index=False,
            encoding='utf-8', decimal='.')
