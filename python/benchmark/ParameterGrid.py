from itertools import product


class ParameterGrid:
  """
  Grid of parameters with a discrete number of values for each.
  Can be used to iterate over parameter value combinations.
  """

  def __init__(self, grid_params):
    # sort keys of a dictionary for reproducibility
    self.items = sorted(grid_params.items())

  def __iter__(self):
    """
    Iterate over the points in the grid.
    """
    # sort keys of a dictionary for reproducibility
    if not self.items:
      yield {}
    else:
      keys, values = zip(*self.items)
      # for each entry v of the cartesian product
      for v in product(*values):
        params = dict(zip(keys, v))
        yield params
