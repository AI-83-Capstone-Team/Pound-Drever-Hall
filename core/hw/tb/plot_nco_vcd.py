import numpy as np
import matplotlib.pyplot as plt

d = np.loadtxt("dumps/nco_out.csv", delimiter=",", skiprows=1)
t = d[:,0]
out1 = d[:,2]
out2 = d[:,3]
addr1 = d[:,4]
addr2 = d[:,5]

plt.plot(out1[:4000])
plt.plot(out2[:4000])
plt.plot(addr1[:4000])
plt.plot(addr2[:4000])
plt.grid(True)
plt.show()
