import numpy as np
from multiprocessing import cpu_count
from multiprocessing.pool import ThreadPool
from .calibrate_variables import calibrate_variables
from .calibrate_final_tuning import calibrate_final_tuning


def calibrate(args):
  # overflows and underflows in numpy should stop the calibration process
  np.seterr(all='raise')

  with ThreadPool(cpu_count()) as pool:
    print('Executing first step...')
    hyperparameters = calibrate_variables(args, pool=pool)

    print('Executing second step...')
    calibrate_final_tuning(args, pool=pool, hyperparameters)
