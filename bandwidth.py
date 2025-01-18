import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

n = input("Enter the value of n to plot: ")
data = pd.read_csv("results/results_mpi.csv")

data = data.sort_values(["code", "n"])

filter = pow(2, int(n))
data = data[data["n"] == filter]

custom_code_order = ["S", "SB", "M", "MD", "MC", "MB"]
xticks_values = [1, 2, 4, 8, 16]
color_map = {code: plt.cm.tab10(i) for i, code in enumerate(data["code"].unique())}

fig, ax = plt.subplots()

data["bandwidth"] = 2 * filter * filter * 8 / data["time2message"] / 1e9
data["effectivebandwidth"] = 2 * filter * filter * 8 / data["time2effective"] / 1e9

for code in custom_code_order:
    if code in data["code"].values:
        code_data = data[data["code"] == code]
        color = color_map[code]

        if code == "S" or code == "SB":
            ax.axhline(
                y=code_data["bandwidth"].values[0],
                color=color,
                linestyle="--",
                label=(code + f" ({code_data['bandwidth'].values[0]:.4f} GB/s)"),
            )
        else:
            ax.plot(
                code_data["processes"].tolist(),
                code_data["bandwidth"].tolist(),
                color=color,
                marker="o",
                linestyle="-",
                label=(code),
            )

            ax.plot(
                code_data["processes"].tolist(),
                code_data["effectivebandwidth"].tolist(),
                color=color,
                marker="o",
                linestyle="--",
                label=(code + " (effective)"),
            )

ax.set_xlabel("Number of processes")
ax.set_ylabel("Bandwidth (GB/s)")
ax.set_xscale("log")
ax.legend()
ax.set_xticks(xticks_values)
ax.set_xticklabels([str(x) for x in xticks_values])
plt.grid(True, linestyle="--", alpha=0.7)
plt.tight_layout()
plt.show()
