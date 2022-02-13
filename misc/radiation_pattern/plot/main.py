import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
import scipy.constants
import numpy as np

matplotlib.use('Qt5Agg')
background_fn = 'data/background.csv'
data_fn = 'data/data.csv'

data_df = pd.read_csv(data_fn)
background_df = pd.read_csv(background_fn)

data_angle_deg = data_df['Angle'].to_numpy()
data_angle = data_angle_deg * scipy.constants.pi / 180.0
background_angle_deg = background_df['Angle'].to_numpy()
background_angle = background_angle_deg * scipy.constants.pi / 180.0

data = data_df['Value'].to_numpy()
background = background_df['Value'].to_numpy()
angle_grid = np.linspace(-90, 90, 37)
np.append(angle_grid, 0)

fig, ax = plt.subplots(subplot_kw={'projection': 'polar'})
ax.set_rmax(3.5)
ax.set_rmin(0)
ax.set_yscale('linear')
ax.set_ylim((0, 3.5), auto=True)
ax.set_rticks([0.5, 1, 1.5, 2, 2.5, 3, 3.3])
ax.set_rlabel_position(-45.0)
ax.set_thetamin(-90)
ax.set_thetamax(90)
ax.set_thetagrids(angle_grid)
ax.grid(True)
ax.plot(data_angle, data, label='Flashlight is ON')
ax.plot(background_angle, background, label='Flashlight is OFF, background')
ax.set_title('Radiation pattern (ADC data, Volts), ', va='top')
ax.set_theta_zero_location('N')
ax.legend(loc="lower left")
plt.show()
