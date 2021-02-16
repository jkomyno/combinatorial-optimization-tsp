import numpy as np

from pymoo.optimize import minimize
from pymoo.factory import get_termination
from pymoo.model.problem import Problem
from pymoo.util.termination.f_tol import MultiObjectiveSpaceToleranceTermination
from typing import List, Tuple


def lower_upper_bounds(bounds: List[Tuple[float, float]]):
  lower_bounds = np.fromiter(map(lambda x: x[0], bounds), dtype=np.float32)
  upper_bounds = np.fromiter(map(lambda x: x[1], bounds), dtype=np.float32)

  return lower_bounds, upper_bounds


def get_termination_for_variables(max_iterations: int = 25):
  """
  Returns the termination criteria for the blackbox optimization problem
  """

  # tol: the tolerance in the objective space on average
  # n_last: it considers the maximum of the last n elements in a sliding window as a worst case
  # nth_gen: the termination criterion is computed every n generations
  # n_max_gen: if the algorithm never converges, stop after n generations
  termination = MultiObjectiveSpaceToleranceTermination(tol=0.0025,
                                                        n_last=30,
                                                        nth_gen=5,
                                                        n_max_gen=max_iterations,
                                                        n_max_evals=None)

  return termination


def get_termination_for_final_tuning(time: str = '06:00:00'):
  """
  Returns the termination criteria for the final tuning blackbox optimization problem
  """

  termination = get_termination('time', time)

  return termination


def get_minimizer(problem, algorithm, termination) -> Tuple[int, float, float]:
  res = minimize(problem,
                 algorithm,
                 termination,
                 return_least_infeasible=False,
                 verbose=True,
                 seed=42,
                 save_history=True)

  best_solution = res.X[0]
  min_average = res.F[0][0]
  min_stddev = res.F[0][1]
  
  return res, best_solution, min_average, min_stddev