#include <stdio.h>
typedef struct {
    int pid, at, bt, ct, tat, wt;
} Process;

int main() {
    Process p[100];
    int n, current_time = 0;
    printf("Enter number of processes: ");
    scanf("%d", &n);

    FILE *fp = fopen("fcfs_log.csv", "w");
    fprintf(fp, "PID,Start,End,Burst,Queue\n");

    for (int i = 0; i < n; i++) {
        printf("Enter AT and BT for P%d: ", i+1);
        scanf("%d%d", &p[i].at, &p[i].bt);
        p[i].pid = i + 1;
    }

    for (int i = 0; i < n; i++) {
        int start_time = (current_time < p[i].at) ? p[i].at : current_time;
        int end_time = start_time + p[i].bt;
        p[i].ct = end_time;
        p[i].tat = p[i].ct - p[i].at;
        p[i].wt = p[i].tat - p[i].bt;
        fprintf(fp, "%d,%d,%d,%d,0\n", p[i].pid, start_time, end_time, p[i].bt);
        current_time = end_time;
    }
    fclose(fp);
}
