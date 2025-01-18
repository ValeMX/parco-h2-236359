# Plot scalability graphs for MPI results

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

pd.options.mode.chained_assignment = None
np.seterr(divide='ignore', invalid='ignore')

n = input("Enter the starting value of n to compute and plot scalability: ")
if int(n) < 4 or int(n) > 8:
    print("Please enter a value between 4 and 8")
    exit()

# Load the data
data = pd.read_csv("results/results_mpi.csv")

# Sort by code and n
data = data.sort_values(["code", "n", "processes"])

# Filter the data
filter = pow(2, int(n))
data = data[data["n"] >= filter]

custom_code_order = ["M", "MD", "MC", "MB"]
xticks_values = [1, 2, 4, 8, 16]
color_map = {code: plt.cm.tab10(i) for i, code in enumerate(data["code"].unique())}

# Plot the scalability
fig, ax = plt.subplots()
for code in custom_code_order:
    if code in data["code"].values:
        # Compute the scalability
        code_data = data[data["code"] == code]
        to_plot = pd.DataFrame(
            {
                "processes": 1,
                "scalability1": 1,
                "scalability2": 1,
            },
            index=[0],
        )

        for i in xticks_values[1:]:
            new_row = pd.DataFrame(
                {
                    "processes": i,
                    "scalability1": code_data[
                        (code_data["processes"] == 1) & (code_data["n"] == filter)
                    ]["time1effective"].values[0]
                    / code_data[
                        (code_data["processes"] == i) & (code_data["n"] == i * filter)
                    ]["time1message"].values[0],
                    "scalability2": code_data[
                        (code_data["processes"] == 1) & (code_data["n"] == filter)
                    ]["time2effective"].values[0]
                    / code_data[
                        (code_data["processes"] == i) & (code_data["n"] == i * filter)
                    ]["time2message"].values[0],
                }, index=[len(to_plot)]
            )
            to_plot = pd.concat([to_plot, new_row], ignore_index=True)

        color = color_map[code]
        if code != "MB":
            ax.plot(
                to_plot["processes"].tolist(),
                to_plot["scalability1"].tolist(),
                color=color,
                marker="o",
                linestyle="--",
                label=(code + " (symmetry)"),
            )

        ax.plot(
            to_plot["processes"].tolist(),
            to_plot["scalability2"].tolist(),
            color=color,
            marker="o",
            linestyle="-",
            label=(code + " (transpose)"),
        )


ax.set_xlabel("Number of processes")
ax.set_ylabel("Scalability")
ax.set_xscale("log")
ax.legend()
ax.set_xticks(xticks_values)
ax.set_xticklabels([str(x) for x in xticks_values])
plt.grid(True, linestyle="--", alpha=0.7)
plt.tight_layout()
plt.show()
