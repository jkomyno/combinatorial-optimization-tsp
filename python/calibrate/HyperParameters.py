from dataclasses import dataclass


@dataclass(frozen=True)
class HyperParameters:
  """
  HyperParameters wraps the best combination of parameters for ex2-metaheuristic found
  in the first step of the calibration process.
  """

  # Probability that a new offspring is mutated.
  # 0 <= mutation_probability <= 1
  mutation_probability: float

  # Probability that a pair of solutions creates an offspring.
  # 0 <= mutation_probability <= 1
  crossover_rate: float

  # Size of the population pool.
  # mu is even
  mu: int

  # Size of the offspring pool before being pruned.
  # lambda is even
  lambda_: int

  # Size of the tournament selection.
  k: int


  def __str__(self):
    return (f'\n\t- mutation-probability: {self.mutation_probability}\n'
            f'\t- crossover-rate: {self.crossover_rate}\n'
            f'\t- μ: {self.mu}\n'
            f'\t- λ: {self.lambda_}\n'
            f'\t- k: {self.k}\n')


  def __iter__(self):
    """
    Enables iterating over data class fields
    """
    return iter(self.__dict__.items())
