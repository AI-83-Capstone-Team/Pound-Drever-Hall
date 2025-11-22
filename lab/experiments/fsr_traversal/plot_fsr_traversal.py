import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path
from matplotlib.widgets import Cursor

def plot_csv_folder(folder_path, smoothing=False, normalize=False):
    folder = Path(folder_path)
    files = sorted(folder.glob("*.csv"))

    if not files:
        print("No CSV files found.")
        return

    plt.ion()  # interactive mode

    for csv_file in files:
        df = pd.read_csv(csv_file)

        # Extract columns
        x = df.iloc[:, 0]
        y1 = df.iloc[:, 1]
        y2 = df.iloc[:, 2]

        # Optional normalization (0→1 scaling)
        if normalize:
            y1 = (y1 - y1.min()) / (y1.max() - y1.min())
            y2 = (y2 - y2.min()) / (y2.max() - y2.min())

        # Optional smoothing
        if smoothing:
            y1 = y1.rolling(window=5, center=True, min_periods=1).mean()
            y2 = y2.rolling(window=5, center=True, min_periods=1).mean()

        # High-DPI figure
        plt.figure(figsize=(10, 6), dpi=150)

        plt.plot(x, y1, label=df.columns[1], linewidth=1.8)
        plt.plot(x, y2, label=df.columns[2], linewidth=1.8)

        plt.title(csv_file.stem)
        plt.xlabel(df.columns[0])
        plt.ylabel("Signal")
        plt.grid(True, alpha=0.25)
        plt.legend()

        # Add interactive cursor
        Cursor(plt.gca(), useblit=True, color='gray', linewidth=1)

        plt.tight_layout()
        plt.show()

        input("Press ENTER for next file...")

    plt.ioff()


# Run it
plot_csv_folder("data", smoothing=False, normalize=False)
