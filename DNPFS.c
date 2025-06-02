#include <stdio.h>

#define MAX_PROCESSES 10
#define ALPHA 0.5

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int priority;

    float predicted_burst;
    int fairness_credit;
    int completed;
    int completion_time;
    int start_time;
    int turnaround_time;
    int waiting_time;
} Process;

int n;
Process processes[MAX_PROCESSES];

void update_prediction(Process *p, int actual_burst) {
    p->predicted_burst = ALPHA * actual_burst + (1 - ALPHA) * p->predicted_burst;
}

float compute_score(Process *p) {
    float alpha_score = 1.0f / (p->predicted_burst + 1.0f);  // Avoid division by 0
    float beta_score = (float)p->fairness_credit;
    float gamma_score = (float)p->priority;

    float alpha_weight = 0.5f;
    float beta_weight = 0.3f;
    float gamma_weight = 0.2f;

    return alpha_score * alpha_weight + beta_score * beta_weight + gamma_score * gamma_weight;
}

void print_gantt_chart(int timeline[], int length) {
    printf("\nGantt Chart:\n|");
    for (int i = 0; i < length; i++) {
        if (i > 0 && timeline[i] == timeline[i - 1])
            printf("   ");
        else
            printf(" P%d ", timeline[i]);
        printf("|");
    }
    printf("\n");
    for (int i = 0; i <= length; i++)
        printf("%2d  ", i);
    printf("\n");
}

void calculate_metrics() {
    float total_waiting_time = 0;
    float total_turnaround_time = 0;
    printf("\nPID\tAT\tBT\tCT\tTAT\tWT\n");
    for (int i = 0; i < n; i++) {
        processes[i].turnaround_time = processes[i].completion_time - processes[i].arrival_time;
        processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;

        total_turnaround_time += processes[i].turnaround_time;
        total_waiting_time += processes[i].waiting_time;

        printf("P%d\t%d\t%d\t%d\t%d\t%d\n",
            processes[i].pid,
            processes[i].arrival_time,
            processes[i].burst_time,
            processes[i].completion_time,
            processes[i].turnaround_time,
            processes[i].waiting_time);
    }

    printf("\nAverage Turnaround Time: %.2f", total_turnaround_time / n);
    printf("\nAverage Waiting Time: %.2f\n", total_waiting_time / n);
}

int main() {
    printf("Enter number of processes: ");
    scanf("%d", &n);

    printf("Enter Arrival Time, Burst Time, and Priority for each process:\n");
    for (int i = 0; i < n; i++) {
        processes[i].pid = i + 1;
        printf("P%d: ", processes[i].pid);
        scanf("%d %d %d", &processes[i].arrival_time, &processes[i].burst_time, &processes[i].priority);
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].predicted_burst = (float)processes[i].burst_time;
        processes[i].fairness_credit = 0;
        processes[i].completed = 0;
    }

    int time = 0;
    int completed = 0;
    int timeline[1000];
    int timeline_index = 0;

    while (completed < n) {
        int selected = -1;
        float max_score = -1.0;

        for (int i = 0; i < n; i++) {
            if (!processes[i].completed && processes[i].arrival_time <= time) {
                float score = compute_score(&processes[i]);
                if (score > max_score) {
                    max_score = score;
                    selected = i;
                }
            }
        }

        if (selected == -1) {
            // No process ready to execute
            timeline[timeline_index++] = 0;  // 0 means idle
            time++;
            continue;
        }

        int quantum = (int)processes[selected].predicted_burst;
        if (quantum < 1) quantum = 1;

        int actual_burst = (processes[selected].remaining_time < quantum)
                        ? processes[selected].remaining_time
                        : quantum;

        // Execute the process
        for (int t = 0; t < actual_burst; t++) {
            timeline[timeline_index++] = processes[selected].pid;
        }

        time += actual_burst;
        processes[selected].remaining_time -= actual_burst;

        if (processes[selected].remaining_time == 0) {
            processes[selected].completed = 1;
            processes[selected].completion_time = time;
            completed++;
        }

        update_prediction(&processes[selected], actual_burst);

        // Increase fairness credits for others
        for (int i = 0; i < n; i++) {
            if (i != selected && !processes[i].completed && processes[i].arrival_time <= time) {
                processes[i].fairness_credit++;
            }
        }
    }

    print_gantt_chart(timeline, timeline_index);
    calculate_metrics();

    return 0;
}
