/*
 * Dynamic Neural-Predictive Fair Scheduling (DNPFS) with:
 * - I/O wait simulation
 * - Preemptive priority (Multilevel Queue)
 * - Log export for visualization
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_PROCESSES 100
#define TIME_QUANTUM 4

typedef struct
{
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int priority; // 0: High, 1: Low
    
    int io_burst_time;
    int waiting_time;
    int turnaround_time;
    bool finished;
    bool in_io;
    int last_executed;
} Process;

Process processes[MAX_PROCESSES];
int process_count = 0;

FILE *log_file;

void log_event(int time, int pid)
{
    if (pid == -1)
        fprintf(log_file, "%d,-1\n", time);
    else
        fprintf(log_file, "%d,%d\n", time, pid);
}

void read_input()
{
    printf("Enter number of processes: ");
    scanf("%d", &process_count);

    for (int i = 0; i < process_count; i++)
    {
        printf("Enter PID, Arrival Time, Burst Time, IO Burst Time, Priority (0=High, 1=Low): ");
        scanf("%d %d %d %d %d",
            &processes[i].pid,
            &processes[i].arrival_time,
            &processes[i].burst_time,
            &processes[i].io_burst_time,
            &processes[i].priority);
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].finished = false;
        processes[i].in_io = false;
        processes[i].waiting_time = 0;
        processes[i].turnaround_time = 0;
        processes[i].last_executed = -1;
    }
}

int select_process(int time)
{
    int selected = -1;
    for (int i = 0; i < process_count; i++)
    {
        if (!processes[i].finished && !processes[i].in_io && processes[i].arrival_time <= time)
        {
            if (selected == -1 ||
                (processes[i].priority < processes[selected].priority) ||
                (processes[i].priority == processes[selected].priority &&
                processes[i].remaining_time < processes[selected].remaining_time))
            {
                selected = i;
            }
        }
    }
    return selected;
}

void simulate()
{
    log_file = fopen("output.csv", "w");
    fprintf(log_file, "Time,PID\n");

    int time = 0;
    int completed = 0;

    while (completed < process_count)
    {
        int idx = select_process(time);

        if (idx == -1)
        {
            log_event(time, -1);
            time++;
            continue;
        }

        // Simulate execution
        log_event(time, processes[idx].pid);
        processes[idx].remaining_time--;
        processes[idx].last_executed = time;

        // Simulate I/O wait if halfway
        if (!processes[idx].in_io &&
            processes[idx].remaining_time == processes[idx].burst_time / 2 &&
            processes[idx].io_burst_time > 0)
        {
            processes[idx].in_io = true;
        }

        if (processes[idx].remaining_time == 0)
        {
            processes[idx].finished = true;
            processes[idx].turnaround_time = time + 1 - processes[idx].arrival_time;
            processes[idx].waiting_time = processes[idx].turnaround_time - processes[idx].burst_time;
            completed++;
        }

        // I/O recovery
        for (int i = 0; i < process_count; i++)
        {
            if (processes[i].in_io && time - processes[i].last_executed >= processes[i].io_burst_time)
            {
                processes[i].in_io = false;
            }
        }

        time++;
    }

    fclose(log_file);
}

void print_summary()
{
    printf("\nPID\tArrival\tBurst\tWait\tTurnaround\n");
    for (int i = 0; i < process_count; i++)
    {
        printf("%d\t%d\t%d\t%d\t%d\n",
            processes[i].pid,
            processes[i].arrival_time,
            processes[i].burst_time,
            processes[i].waiting_time,
            processes[i].turnaround_time);
    }
}

int main()
{
    read_input();
    simulate();
    print_summary();
    printf("\nExecution log written to output.csv\n");
    return 0;
}
