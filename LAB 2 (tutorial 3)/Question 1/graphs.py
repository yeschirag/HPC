import pandas as pd
import matplotlib.pyplot as plt
import math

serial = pd.read_csv("serial_results.csv")
parallel = pd.read_csv("parallel_results.csv")

PI = math.pi


# ============================================
# GRAPH 1
# Number of Points vs Estimated Pi
# ============================================

plt.figure(figsize=(9, 6))

plt.plot(
    serial["points"],
    serial["pi"],
    marker="o",
    label="Sequential"
)

for t in [1, 2, 4, 8, 16]:
    data = parallel[parallel["threads"] == t]

    plt.plot(
        data["points"],
        data["pi"],
        marker="o",
        label=f"Pthreads - {t} threads"
    )

plt.axhline(
    PI,
    linestyle="--",
    label="Actual π"
)

plt.xscale("log")

plt.xlabel("Number of Points")
plt.ylabel("Estimated π")
plt.title("Number of Points vs Estimated π")

plt.legend()
plt.grid(True)

plt.savefig("01_points_vs_pi.png", dpi=300, bbox_inches="tight")
plt.show()


# ============================================
# GRAPH 2
# Number of Points vs Error
# ============================================

plt.figure(figsize=(9, 6))

# Remove zero errors for log scale
serial_error = serial[serial["error"] > 0]

plt.plot(
    serial_error["points"],
    serial_error["error"],
    marker="o",
    label="Sequential"
)

for t in [1, 2, 4, 8, 16]:

    data = parallel[
        (parallel["threads"] == t) &
        (parallel["error"] > 0)
    ]

    plt.plot(
        data["points"],
        data["error"],
        marker="o",
        label=f"Pthreads - {t} threads"
    )

plt.xscale("log")
plt.yscale("log")

plt.xlabel("Number of Points")
plt.ylabel("Absolute Error")
plt.title("Number of Points vs Error")

plt.legend()
plt.grid(True)

plt.savefig("02_points_vs_error.png", dpi=300, bbox_inches="tight")
plt.show()


# ============================================
# GRAPH 3
# Number of Points vs Execution Time
# ============================================

plt.figure(figsize=(9, 6))

plt.plot(
    serial["points"],
    serial["time"],
    marker="o",
    label="Sequential"
)

for t in [1, 4, 16]:

    data = parallel[
        (parallel["threads"] == t) &
        (parallel["time"] > 0)
    ]

    plt.plot(
        data["points"],
        data["time"],
        marker="o",
        label=f"Pthreads - {t} threads"
    )

plt.xscale("log")

# IMPORTANT:
# Do NOT use log Y-axis because small times can be zero.

plt.xlabel("Number of Points")
plt.ylabel("Execution Time (seconds)")
plt.title("Number of Points vs Execution Time")

plt.legend()
plt.grid(True)

plt.savefig("03_points_vs_time.png", dpi=300, bbox_inches="tight")
plt.show()


# ============================================
# GRAPH 4
# Number of Threads vs Execution Time
# ============================================

# Use 10^9 points
N = 1000000000

data = parallel[
    (parallel["points"] == N) &
    (parallel["time"] > 0)
].copy()

plt.figure(figsize=(9, 6))

plt.plot(
    data["threads"],
    data["time"],
    marker="o"
)

plt.xticks([1, 2, 4, 8, 16])

plt.xlabel("Number of Threads")
plt.ylabel("Execution Time (seconds)")
plt.title(
    f"Number of Threads vs Execution Time ({N:,} Points)"
)

plt.grid(True)

plt.savefig("04_threads_vs_time.png", dpi=300, bbox_inches="tight")
plt.show()


# ============================================
# GRAPH 5
# Number of Threads vs Speedup
# ============================================

T1 = data[
    data["threads"] == 1
]["time"].iloc[0]

data["speedup"] = T1 / data["time"]

plt.figure(figsize=(9, 6))

plt.plot(
    data["threads"],
    data["speedup"],
    marker="o",
    label="Actual Speedup"
)

# Ideal speedup
plt.plot(
    data["threads"],
    data["threads"],
    linestyle="--",
    label="Ideal Speedup"
)

plt.xticks([1, 2, 4, 8, 16])

plt.xlabel("Number of Threads")
plt.ylabel("Speedup")
plt.title(
    f"Number of Threads vs Speedup ({N:,} Points)"
)

plt.legend()
plt.grid(True)

plt.savefig("05_threads_vs_speedup.png", dpi=300, bbox_inches="tight")
plt.show()


# ============================================
# GRAPH 6
# Number of Threads vs Efficiency
# ============================================

data["efficiency"] = (
    data["speedup"] /
    data["threads"]
) * 100

plt.figure(figsize=(9, 6))

plt.plot(
    data["threads"],
    data["efficiency"],
    marker="o",
    label="Actual Efficiency"
)

plt.axhline(
    100,
    linestyle="--",
    label="Ideal Efficiency"
)

plt.xticks([1, 2, 4, 8, 16])

plt.xlabel("Number of Threads")
plt.ylabel("Efficiency (%)")
plt.title(
    f"Number of Threads vs Efficiency ({N:,} Points)"
)

plt.legend()
plt.grid(True)

plt.savefig("06_threads_vs_efficiency.png", dpi=300, bbox_inches="tight")
plt.show()


# ============================================
# GRAPH 7
# Sequential vs Pthreads Execution Time
# ============================================

plt.figure(figsize=(9, 6))

serial_time = serial[serial["time"] > 0]

plt.plot(
    serial_time["points"],
    serial_time["time"],
    marker="o",
    label="Sequential"
)

for t in [1, 2, 4, 8, 16]:

    data_t = parallel[
        (parallel["threads"] == t) &
        (parallel["time"] > 0)
    ]

    plt.plot(
        data_t["points"],
        data_t["time"],
        marker="o",
        label=f"Pthreads - {t}"
    )

plt.xscale("log")

plt.xlabel("Number of Points")
plt.ylabel("Execution Time (seconds)")
plt.title("Sequential vs Pthreads Execution Time")

plt.legend()
plt.grid(True)

plt.savefig("07_sequential_vs_parallel.png", dpi=300, bbox_inches="tight")
plt.show()


# ============================================
# GRAPH 8
# Speedup vs Number of Points
# ============================================

plt.figure(figsize=(9, 6))

for t in [2, 4, 8, 16]:

    data_t = parallel[
        (parallel["threads"] == t) &
        (parallel["time"] > 0)
    ].copy()

    one_thread = parallel[
        (parallel["threads"] == 1) &
        (parallel["time"] > 0)
    ][["points", "time"]]

    data_t = data_t.merge(
        one_thread,
        on="points",
        suffixes=("", "_1thread")
    )

    data_t["speedup"] = (
        data_t["time_1thread"] /
        data_t["time"]
    )

    plt.plot(
        data_t["points"],
        data_t["speedup"],
        marker="o",
        label=f"{t} threads"
    )

plt.xscale("log")

plt.xlabel("Number of Points")
plt.ylabel("Speedup")
plt.title("Speedup vs Number of Points")

plt.legend()
plt.grid(True)

plt.savefig("08_speedup_vs_points.png", dpi=300, bbox_inches="tight")
plt.show()


print("\n================================")
print(" ALL 8 GRAPHS GENERATED")
print("================================")