import numpy as np 
#import matplotlib.pyplot as plt

filename = "samples.bin"

mean = 0
std = 0.1
samples = 300

impulse1 = 100
impulse2 = 200

y = np.random.normal(mean, std, size=samples)

for i in range(impulse1, impulse1 + 10):
	y[i] = 1.0

for i in range(impulse2, impulse2 + 10):
	y[i] = -1.0

y = y * np.iinfo(np.int16).max
y = np.asarray(y, dtype=np.int16)
y.astype('int16').tofile(filename)