import numpy as np
import matplotlib.pyplot as plt

def main():
    # Load CSV with 3 columns
    # No header, comma-separated
    data = np.loadtxt("lockin_log.csv", delimiter=",")

    x = data[:, 0]
    y1 = data[:, 1]
    y2 = data[:, 2]

    plt.plot(x, y1, label="y1")
    plt.plot(x, y2, label="y2")

    plt.xlabel("x")
    plt.ylabel("y")
    plt.legend()
    plt.grid(True)

    plt.show()

if __name__ == "__main__":
    main()
