#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_PROCESSES 100
#define ALPHA 0.5
#define MAX_TIME_LIMIT 100000

FILE *log_file;

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int priority; // 0: High, 1: Low
    float predicted_burst;
    int fairness_credit;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int io_request_time;
    int io_duration;
    int io_remaining;
    bool in_io;
    bool io_done;
    bool io_started;
    bool finished;
} Process;

Process processes[MAX_PROCESSES];
int n;

void log_execution(int pid, int start, int end, int burst, int queue) {
    if (burst > 0)
        fprintf(log_file, "%d,%d,%d,%d,%d\n", pid, start, end, burst, queue);
}

float compute_score(Process *p) {
    float alpha_score = 1.0f / (p->predicted_burst + 1.0f);
    float beta_score = (float)p->fairness_credit;
    float gamma_score = 1.0f - (float)p->priority;
    float alpha_weight = 0.5f, beta_weight = 0.3f, gamma_weight = 0.2f;
    return alpha_score * alpha_weight + beta_score * beta_weight + gamma_score * gamma_weight;
}

void update_prediction(Process *p, int actual_burst) {
    p->predicted_burst = ALPHA * actual_burst + (1 - ALPHA) * p->predicted_burst;
}

void calculate_metrics() {
    float total_waiting_time = 0, total_turnaround_time = 0;

    printf("\nPID\tAT\tBT\tCT\tTAT\tWT\n");
    for (int i = 0; i < n; i++) {
        processes[i].turnaround_time = processes[i].completion_time - processes[i].arrival_time;
        processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;
        total_waiting_time += processes[i].waiting_time;
        total_turnaround_time += processes[i].turnaround_time;

        printf("P%d\t%d\t%d\t%d\t%d\t%d\n",
            processes[i].pid,
            processes[i].arrival_time,
            processes[i].burst_time,
            processes[i].completion_time,
            processes[i].turnaround_time,
            processes[i].waiting_time);
    }

    printf("\nAverage TAT: %.2f", total_turnaround_time / n);
    printf("\nAverage WT : %.2f\n", total_waiting_time / n);
}

int main() {
    log_file = fopen("dnpfs_log.csv", "w");
    fprintf(log_file, "PID,Start,End,Burst,Queue\n");

    printf("Enter number of processes: ");
    scanf("%d", &n);

    printf("Enter Arrival Time, Burst Time, Priority (0=High,1=Low), IO_Request_Time, IO_Duration:\n");
    for (int i = 0; i < n; i++) {
        processes[i].pid = i + 1;
        printf("P%d: ", i + 1);
        scanf("%d %d %d %d %d",
            &processes[i].arrival_time,
            &processes[i].priority,
            &processes[i].burst_time,
            &processes[i].io_duration);
            &processes[i].io_request_time,

        processes[i].remaining_time = processes[i].burst_time;
        processes[i].predicted_burst = (float)processes[i].burst_time;
        processes[i].fairness_credit = 0;
        processes[i].in_io = false;
        processes[i].io_done = false;
        processes[i].io_started = false;
        processes[i].finished = false;
        processes[i].io_remaining = processes[i].io_duration;
    }

    int time = 0, completed = 0;

    while (completed < n && time < MAX_TIME_LIMIT) {
        // Handle I/O
        for (int i = 0; i < n; i++) {
            if (processes[i].in_io) {
                processes[i].io_remaining--;
                if (processes[i].io_remaining <= 0) {
                    processes[i].in_io = false;
                    processes[i].io_done = true;
                    processes[i].io_remaining = processes[i].io_duration;
                }
            }
        }

        // Select next process
        int selected = -1;
        float max_score = -1.0f;

        for (int i = 0; i < n; i++) {
            if (!processes[i].finished && !processes[i].in_io && processes[i].arrival_time <= time) {
                float score = compute_score(&processes[i]);
                if (score > max_score) {
                    max_score = score;
                    selected = i;
                }
            }
        }

        if (selected == -1) {
            time++;
            continue;
        }

        int exec_time = (int)processes[selected].predicted_burst;
        if (exec_time < 1) exec_time = 1;

        int actual_burst = 0;
        int start_time = time;

        for (int i = 0; i < exec_time; i++) {
            if (processes[selected].remaining_time == 0) break;

            // Check for IO request
            if (!processes[selected].io_done &&
                !processes[selected].io_started &&
                (processes[selected].burst_time - processes[selected].remaining_time) == processes[selected].io_request_time) {
                processes[selected].in_io = true;
                processes[selected].io_started = true;
                break;
            }

            processes[selected].remaining_time--;
            time++;
            actual_burst++;

            if (time >= MAX_TIME_LIMIT) break;
        }

        int end_time = time;

        // Log the burst if any actual execution happened
        log_execution(processes[selected].pid, start_time, end_time, actual_burst, processes[selected].priority);

        if (processes[selected].remaining_time == 0 && !processes[selected].finished) {
            processes[selected].finished = true;
            processes[selected].completion_time = time;
            completed++;
        }

        update_prediction(&processes[selected], actual_burst);

        // Fairness credit to all others
        for (int i = 0; i < n; i++) {
            if (i != selected && !processes[i].finished)
                processes[i].fairness_credit++;
        }

        if (time % 1000 == 0)
            printf("Time: %d, Completed: %d\n", time, completed);
    }

    fclose(log_file);

    if (time >= MAX_TIME_LIMIT)
        printf("\n⚠️ Aborted: Time limit exceeded (%d units).\n", MAX_TIME_LIMIT);

    calculate_metrics();
    printf("\n✅ Execution log written to dnpfs_log.csv\n");

    return 0;
}
