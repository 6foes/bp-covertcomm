# Nolan Shah
# Covert Channels Communication Assignment
# COSC 3330, Computer Architecture, University of Houston

import matplotlib as mpl
import numpy as np
import matplotlib.pyplot as plt

data = np.genfromtxt('./out.csv', delimiter=',', skip_header=1, skip_footer=1, names=['y'])

fig = plt.figure()
ax1 = fig.add_subplot(111)

ax1.plot(data['y'], color='r')

plt.show()