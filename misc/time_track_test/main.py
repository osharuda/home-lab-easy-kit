import sys

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
import scipy.constants
import numpy as np

matplotlib.use('Qt5Agg')
#data_fn = sys.argv[1]
#data_fn = "data_100khz_release.csv"
data_fn = "data_50khz_release.csv"
data_df = pd.read_csv(data_fn)

timestamps = data_df['Timestamp'].to_numpy()
n_bins = 100

fig, axs = plt.subplots(sharey=True, tight_layout=True)
#axs.hist(timestamps, bins=n_bins, range=[4.8e-5, 5.2e-5])   # 10 KHz
#axs.hist(timestamps, bins=n_bins, range=[1e-6, 5.0e-5])   # 100 KHz (eff 200KHz)
axs.hist(timestamps, bins=n_bins, range=[9e-6, 11e-6])   # 50 KHz (eff 100KHz)
plt.grid(True, axis='both')
plt.show()
