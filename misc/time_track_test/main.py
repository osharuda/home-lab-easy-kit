import sys

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib

matplotlib.use('Qt5Agg')
data_fn = sys.argv[1]
data_df = pd.read_csv(data_fn)

timestamps = data_df['Timestamp'].to_numpy()
n_bins = 500

fig, axs = plt.subplots(sharey=True, tight_layout=True)

# Plot settings for meander with F=50KHz. As time tracking is configured for rising and fall edges, two events per period is generated,
# so event frequency is 100K events per seconds. Meander may not be exactly 50% filled, so two peaks are expected.
axs.hist(timestamps, bins=n_bins, range=(9e-6, 11e-6))

plt.grid(True, axis='both')
plt.show()
