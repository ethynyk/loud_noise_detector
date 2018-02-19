import matplotlib.pyplot as plt
import numpy as np

thresh_perc = 0.3

thresh = pow(0x80, 2) * thresh_perc

with open("samples.bin", "rb") as f:
	data = np.fromfile(f, dtype=np.int16)
	x = np.arange(0, 15, 0.05)
	plt.plot(x, data)
	plt.axis([0, 15, -32767, 32767])
	plt.axhline(thresh);
	plt.axhline(thresh * -1);
	plt.show()

