# Plot speedup and efficiency graphs for MPI results

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

pd.options.mode.chained_assignment = None

n = input("Enter the value of n to plot: ")

# Load the data
data = pd.read_csv("results/results_mpi.csv")

# Sort by code and n
data = data.sort_values(["code", "n", "processes"])

# Filter the data
filter = pow(2, int(n))
data = data[data["n"] == filter]

custom_code_order = ["M", "MD", "MC", "MB"]
xticks_values = [1, 2, 4, 8, 16]
color_map = {code: plt.cm.tab10(i) for i, code in enumerate(data["code"].unique())}

# Plot the speedup
fig, ax = plt.subplots()
for code in custom_code_order:
    if code in data["code"].values:
        # Compute the speedup
        code_data = data[data["code"] == code]
        sequential_data = code_data[code_data["processes"] == 1]
        code_data["speedup1"] = (
            sequential_data["time1effective"].values[0] / code_data["time1message"]
        )
        code_data["speedup2"] = (
            sequential_data["time2effective"].values[0] / code_data["time2message"]
        )

        color = color_map[code]
        if code != "MB":
            ax.plot(
                code_data["processes"].tolist(),
                code_data["speedup1"].tolist(),
                color=color,
                marker="o",
                linestyle="--",
                label=(code + " (symmetry)"),
            )

        ax.plot(
            code_data["processes"].tolist(),
            code_data["speedup2"].tolist(),
            color=color,
            marker="o",
            linestyle="-",
            label=(code + " (transpose)"),
        )


ax.set_xlabel("Number of processes")
ax.set_ylabel("Speedup")
ax.set_xscale("log")
ax.legend()
ax.set_xticks(xticks_values)
ax.set_xticklabels([str(x) for x in xticks_values])
plt.grid(True, linestyle="--", alpha=0.7)
plt.tight_layout()
plt.show()

# Plot the efficiency
fig, ax = plt.subplots()
for code in custom_code_order:
    if code in data["code"].values:
        # Compute the efficiency
        code_data = data[data["code"] == code]
        sequential_data = code_data[code_data["processes"] == 1]
        code_data["speedup1"] = (
            sequential_data["time1effective"].values[0] / code_data["time1message"]
        )
        code_data["efficiency1"] = code_data["speedup1"] / code_data["processes"] * 100
        code_data["speedup2"] = (
            sequential_data["time2effective"].values[0] / code_data["time2message"]
        )
        code_data["efficiency2"] = code_data["speedup2"] / code_data["processes"] * 100

        color = color_map[code]
        if code != "MB":
            ax.plot(
                code_data["processes"].tolist(),
                code_data["efficiency1"].tolist(),
                color=color,
                marker="o",
                linestyle="--",
                label=(code + " (symmetry)"),
            )

        ax.plot(
            code_data["processes"].tolist(),
            code_data["efficiency2"].tolist(),
            color=color,
            marker="o",
            linestyle="-",
            label=(code + " (transpose)"),
        )

ax.set_xlabel("Number of processes")
ax.set_ylabel("Efficiency")
ax.set_xscale("log")
ax.legend()
ax.set_xticks(xticks_values)
ax.set_xticklabels([str(x) for x in xticks_values])
plt.grid(True, linestyle="--", alpha=0.7)
plt.tight_layout()
plt.show()