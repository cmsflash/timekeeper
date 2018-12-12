/*
Filename: timekeeper_3035233228.c
Author: Shen Zhuoran
U No.: 3035233228
Development Platform:
    Ubuntu 18.04.1 LTS as Windows Subsystem for Linux,
    gcc 7.3.0
Compilation:
    gcc timekeeper_3035233228.c -o timekeeper_3035233228 -Wall
Completed features: 1, 2, 3, 4, 5, 6
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>

int DEFAULT_STRING_SIZE = 400;
char* SIGNAL_NAMES[] = {
    "placeholder", "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP",
    "SIGABRT", "SIGBUS", "SIGFPE", "SIGKILL", "SIGUSR1", "SIGSEGV", "SIGUSR2",
    "SIGPIPE", "SIGALRM", "SIGTERM", "SIGSTKFLT", "SIGCHLD", "SIGCONT",
    "SIGSTOP", "SIGTSTP", "SIGTTIN", "SIGTTOU", "SIGURG", "SIGXCPU", "SIGXFSZ",
    "SIGVTALRM", "SIGPROF", "SIGWINCH", "SIGIO", "SIGPWR", "SIGSYS",
    "SIGRTMIN", "SIGRTMIN+1", "SIGRTMIN+2", "SIGRTMIN+3", "SIGRTMIN+4",
    "SIGRTMIN+5", "SIGRTMIN+6", "SIGRTMIN+7", "SIGRTMIN+8", "SIGRTMIN+9",
    "SIGRTMIN+10", "SIGRTMIN+11", "SIGRTMIN+12", "SIGRTMIN+13", "SIGRTMIN+14",
    "SIGRTMIN+15", "SIGRTMAX-14", "SIGRTMAX-13", "SIGRTMAX-12", "SIGRTMAX-11",
    "SIGRTMAX-10", "SIGRTMAX-9", "SIGRTMAX-8", "SIGRTMAX-7", "SIGRTMAX-6",
    "SIGRTMAX-5", "SIGRTMAX-4", "SIGRTMAX-3", "SIGRTMAX-2", "SIGRTMAX-1",
    "SIGRTMAX"
};

struct process {
    char** args;
    struct process* next;
};

pid_t* child_pids;
int* waiteds;
int process_count = 0;

void deliver_to_child(int signum) {
    for (int i = 0; i < process_count; i++) {
        kill(child_pids[i], signum);
    }
}

void timespec_diff(
        struct timespec *start, struct timespec *stop, struct timespec *result
    ) {
    if ((stop->tv_nsec - start->tv_nsec) < 0) {
        result->tv_sec = stop->tv_sec - start->tv_sec - 1;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
    } else {
        result->tv_sec = stop->tv_sec - start->tv_sec;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec;
    }
    return;
}

double to_double(struct timespec* spec) {
    double time = spec->tv_sec + spec->tv_nsec / 1000000000.0;
    return time;
}

void collect_child(char*** processes, int index, struct timespec start) {
    pid_t pid = child_pids[index];
    int return_status;
    waitid(P_PID, pid, NULL, WNOWAIT | WEXITED);

    char* stat_path = (char*)malloc(DEFAULT_STRING_SIZE);
    sprintf(stat_path, "/proc/%d/stat", pid);
    char** stats = (char**)malloc(44 * sizeof(char*));
    FILE* stat_file = fopen(stat_path, "r");
    for (int i = 0; i < 44; i++) {
        stats[i] = malloc(DEFAULT_STRING_SIZE);
        fscanf(stat_file, "%s", stats[i]);
    }
    fclose(stat_file);
    
    char* status_path = (char*)malloc(DEFAULT_STRING_SIZE);
    sprintf(status_path, "/proc/%d/status", pid);
    char** statuses = (char**)malloc(93 * sizeof(char*));
    FILE* status_file = fopen(status_path, "r");
    for (int i = 0; i < 93; i++) {
        statuses[i] = malloc(DEFAULT_STRING_SIZE);
        fscanf(status_file, "%s", statuses[i]);
    }
    fclose(status_file);
    
    struct timespec stop, real_time;
    clock_gettime(CLOCK_MONOTONIC, &stop);
    waitpid(pid, &return_status, 0);
    int signaled = WIFSIGNALED(return_status);
    int signal_id = WTERMSIG(return_status);
    timespec_diff(&start, &stop, &real_time);
    double user_time = (
        (double)strtol(stats[14], NULL, 10) / sysconf(_SC_CLK_TCK)
    );
    double sys_time = (
        (double)strtol(stats[15], NULL, 10) / sysconf(_SC_CLK_TCK)
    );
    int context_switches = atoi(statuses[87]) + atoi(statuses[89]);
    if (signaled) {
        printf(
            (
                "The command \"%s\" is interrupted"
                " by the signal number = %d (%s)\n"
            ),
            processes[index][0], signal_id, SIGNAL_NAMES[signal_id]
        );
    } else {
        printf(
            (
                "The command \"%s\" terminated"
                " with returned status code = %d\n"
            ),
            processes[index][0], WTERMSIG(return_status)
        );
    }
    printf(
        (
            "real: %.02lf s, user: %.02lf s, system: %.02lf s,"
            " context switch: %d\n"
        ),
        to_double(&real_time), user_time, sys_time, context_switches
    );
}

char** construct_process(char** argv, int start_index, int end_index) {
    char** args = (char**)malloc((end_index - start_index + 1) * sizeof(char*));
    for (int i = 0; i < end_index - start_index; i++) {
        args[i] = argv[start_index + i];
    }
    args[end_index - start_index] = NULL;
    return args;
}

int main(int argc, char** argv) {
    if (argc == 1) {
        return;
    }
    signal(SIGINT, deliver_to_child);
    process_count = 1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "!") == 0) {
            process_count++;
        }
    }
    char*** processes = (char***)malloc(process_count * sizeof(char**));
    int first_arg_index = 1;
    int process_id = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "!") == 0) {
            processes[process_id] = construct_process(argv, first_arg_index, i);
            process_id++;
        }
        if (i == argc - 1) {
            processes[process_id] = construct_process(
                argv, first_arg_index, argc
            );
            process_id++;
        }
    }
    int** pfds = (int**)malloc((process_count - 1) * sizeof(int*));
    for (int i = 0; i < process_count - 1; i++) {
        pfds[i] = (int*)malloc(2 * sizeof(int));
        pipe(pfds[i]);
    }
    child_pids = (pid_t*)malloc(process_count * sizeof(pid_t));
    waiteds = (int*)malloc(process_count * sizeof(int));
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    pid_t pid;
    for (int i = 0; i < process_count; i++) {
        pid = fork();
        if (pid > 0) {
            /////
            sleep(1);
            child_pids[i] = pid;
            waiteds[i] = 0;
            printf(
                "Process with id: %d created for the command: %s\n",
                pid, processes[i][0]
            );
        } else if (pid == 0) {
            for (int j = 0; j < process_count - 1; j++) {
                if (j == i - 1) {
                    dup2(pfds[j][0], 0);
                    close(pfds[j][1]);
                } else if (j == i && j < process_count - 1) {
                    close(pfds[j][0]);
                    dup2(pfds[j][1], 1);
                } else if (j < process_count - 1) {
                    close(pfds[j][0]);
                    close(pfds[j][1]);
                }
            }
            printf("=========%s", processes[i][0]);
            int success_code = execvp(processes[i][0], processes[i]);
            if (success_code == -1) {
                printf(
                    (
                        "timekeeper experienced an error"
                        " in starting the command: %s\n"
                    ),
                    processes[i][0]
                );
            }
        } else {
            printf("Error creating new process(es).");
        }
    }
    while (1) {
        sleep(1000);
    }
}
