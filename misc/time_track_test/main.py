import sys

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
import scipy.constants
import numpy as np

matplotlib.use('Qt5Agg')
data_fn = sys.argv[1]
data_df = pd.read_csv(data_fn)

timestamps = data_df['Timestamp'].to_numpy()
n_bins = 2000

fig, axs = plt.subplots(sharey=True, tight_layout=True)
#axs.hist(timestamps, bins=n_bins, range=[4.8e-5, 5.2e-5])   # 10 KHz
axs.hist(timestamps, bins=n_bins, range=[3e-5, 7e-5])   # 50 KHz
plt.grid(True, axis='both')
plt.show()
