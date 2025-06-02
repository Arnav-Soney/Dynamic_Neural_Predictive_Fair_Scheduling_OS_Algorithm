# import pandas as pd
# import matplotlib.pyplot as plt
# import matplotlib.patches as mpatches

# # Load execution log
# df = pd.read_csv("E:\\OS scheduling algo\\dnpfs_log.csv")

# # Normalize queue info
# queue_colors = {0: "skyblue", 1: "lightcoral"}

# # Gantt chart
# def plot_gantt(df):
#     fig, ax = plt.subplots(figsize=(12, 6))
#     yticks = []
#     ylabels = []
#     for i, row in df.iterrows():
#         start = row["Start"]
#         end = row["End"]
#         pid = row["PID"]
#         queue = row["Queue"]
#         color = queue_colors.get(queue, "gray")
#         ax.barh(pid, end - start, left=start, height=0.6, color=color, edgecolor="black")
#         ax.text((start + end) / 2, pid, f"P{pid}", va='center', ha='center', fontsize=8, color='black')
#         yticks.append(pid)
#         ylabels.append(f"P{pid}")
#     ax.set_xlabel("Time")
#     ax.set_ylabel("Process")
#     ax.set_title("DNPFS Gantt Chart")
#     ax.set_yticks(sorted(set(yticks)))
#     ax.set_yticklabels(sorted(set(ylabels)))
#     ax.grid(True, axis='x', linestyle='--', alpha=0.7)

#     # Legend
#     patches = [mpatches.Patch(color=clr, label=f"Queue {q}") for q, clr in queue_colors.items()]
#     ax.legend(handles=patches)

#     plt.tight_layout()
#     plt.show()

# # Stats
# def print_stats(df):
#     total_time = df["End"].max()
#     context_switches = len(df)
#     queue_usage = df.groupby("Queue").size()

#     print("\n📊 Summary:")
#     print(f"⏱️ Total execution time: {total_time}")
#     print(f"🔁 Context switches    : {context_switches}")
#     # print(f"📌 Queue distribution:")
#     for q, count in queue_usage.items():
#         print(f"  - Queue {q}: {count} entries")

# # CPU usage plot
# def plot_cpu_utilization(df):
#     timeline = []
#     for _, row in df.iterrows():
#         for t in range(row["Start"], row["End"]):
#             timeline.append((t, row["PID"]))

#     usage_df = pd.DataFrame(timeline, columns=["Time", "PID"])
#     usage_counts = usage_df["Time"].value_counts().sort_index()
#     plt.figure(figsize=(12, 3))
#     plt.plot(usage_counts.index, usage_counts.values, drawstyle="steps-post", label="CPU Busy")
#     plt.title("CPU Activity Over Time")
#     plt.xlabel("Time")
#     plt.ylabel("CPU Active")
#     plt.grid(True)
#     plt.tight_layout()
#     plt.show()

# # Main
# if __name__ == "__main__":
#     print_stats(df)
#     plot_gantt(df)
#     plot_cpu_utilization(df)


import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Load the log file
df = pd.read_csv("dnpfs_log.csv")  # Columns: PID,Start,End,Burst,Queue

# Set a consistent color palette
sns.set(style="whitegrid")
colors = sns.color_palette("tab10", n_colors=df['PID'].nunique())

# Plot Gantt Chart
def plot_gantt(df):
    fig, ax = plt.subplots(figsize=(12, 6))

    process_colors = {}
    for i, pid in enumerate(sorted(df['PID'].unique())):
        process_colors[pid] = colors[i]

    for _, row in df.iterrows():
        ax.barh(y=f"P{row['PID']}",
                width=row['End'] - row['Start'],
                left=row['Start'],
                height=0.6,
                color=process_colors[row['PID']],
                edgecolor='black')

    ax.set_xlabel("Time")
    ax.set_ylabel("Processes")
    ax.set_title("DNPFS Gantt Chart")
    plt.tight_layout()
    plt.show()

# Plot Queue Usage Over Time
def plot_queue_timeline(df):
    fig, ax = plt.subplots(figsize=(12, 4))

    for _, row in df.iterrows():
        label = f"Q{row['Queue']}"
        color = 'red' if row['Queue'] == 0 else 'blue'
        ax.barh(y=label,
                left=row['Start'],
                width=row['End'] - row['Start'],
                height=0.4,
                color=color,
                alpha=0.6)

    ax.set_xlabel("Time")
    ax.set_title("Queue Usage Over Time (Red = High Priority, Blue = Low Priority)")
    plt.tight_layout()
    plt.show()

# Plot Process Execution Summary
def plot_execution_summary(df):
    process_df = df.groupby("PID").agg({
        "Burst": "sum",
        "Start": "min",
        "End": "max"
    }).reset_index()

    process_df["Turnaround Time"] = process_df["End"] - process_df["Start"]

    fig, axes = plt.subplots(1, 2, figsize=(12, 4))

    sns.barplot(x="PID", y="Burst", data=process_df, ax=axes[0], palette=colors)
    axes[0].set_title("Total CPU Burst per Process")
    axes[0].set_ylabel("Time Units")

    sns.barplot(x="PID", y="Turnaround Time", data=process_df, ax=axes[1], palette=colors)
    axes[1].set_title("Turnaround Time per Process")
    axes[1].set_ylabel("Time Units")

    plt.tight_layout()
    plt.show()

# Run all visualizations
plot_gantt(df)
plot_queue_timeline(df)
plot_execution_summary(df)
