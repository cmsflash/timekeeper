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

int child_count;
pid_t* child_pids;

void deliver_to_child(const int signum) {
    for (int i = 0; i < child_count; i++) {
        kill(child_pids[i], signum);
    }
}

void timespec_diff(
        const struct timespec *start, const struct timespec *stop,
        struct timespec *result
    ) {
    if ((stop->tv_nsec - start->tv_nsec) < 0) {
        result->tv_sec = stop->tv_sec - start->tv_sec - 1;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
    } else {
        result->tv_sec = stop->tv_sec - start->tv_sec;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec;
    }
}

double to_double(const struct timespec* spec) {
    double time = spec->tv_sec + spec->tv_nsec / 1000000000.0;
    return time;
}

char** read_proc_file_alloc(
        const int pid, const char* filename, const int line_count
    ) {
    char* path = (char*)malloc(DEFAULT_STRING_SIZE);
    sprintf(path, "/proc/%d/%s", pid, filename);
    char** lines = (char**)malloc(line_count * sizeof(char*));
    FILE* file = fopen(path, "r");
    for (int i = 0; i < line_count; i++) {
        lines[i] = malloc(DEFAULT_STRING_SIZE);
        fscanf(file, "%s", lines[i]);
    }
    fclose(file);
    free(path);
    return lines;
}

double parse_stat_time(const char* stat) {
    double time_ = ((double)strtol(stat, NULL, 10) / sysconf(_SC_CLK_TCK));
    return time_;
}

void execute(const char* command, const char** arguments) {
    int success_code = execvp(command, arguments);
    if (success_code == -1) {
        printf(
            "timekeeper experienced an error in starting the command: %s\n",
            command
        );
        exit(1);
    }
}

void double_free(const void** pointer, const int length) {
    for (int i = 0; i < length; i++) {
        free(pointer[i]);
    }
    free(pointer);
}

int** create_pipes_alloc(int count) {
    int** pipes = malloc(count * sizeof(int*));
    for (int i = 0; i < count; i++) {
        pipes[i] = malloc(2 * sizeof(int));
        pipe(pipes[i]);
    }
    return pipes;
}

int main(int argc, char** argv) {
    if (argc == 1) {
        return;
    }
    signal(SIGINT, deliver_to_child);
    child_count = 1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "!") == 0) {
            child_count++;
        }
    }
    child_pids = (pid_t*)malloc(child_count * sizeof(pid_t));
    
    int* argcs = (int *)malloc(child_count * sizeof(int));
    int argv_index = 1;
    for (int i = 0; i < child_count; i++) {
        argcs[i] = 0;
        while (argv_index < argc && strcmp(argv[argv_index], "!") != 0) {
            argcs[i]++;
            argv_index++;
        }
        argv_index++;
    }
    char*** argvs = (char ***)malloc(child_count * sizeof(char**));
    argv_index = 0;
    for (int i = 0; i < child_count; i++) {
        argvs[i] = (char**)malloc(argcs[i] * sizeof(char*));
        argv_index++;
        for (int j = 0; j < argcs[i]; j++) {
            argvs[i][j] = argv[argv_index];
            argv_index++;
        }
    }

    int** pipes = create_pipes_alloc(child_count - 1);

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    pid_t pid = fork();
    if (pid == 0) {
        execute(argvs[0][0], argvs[0]);
        exit(0);
    } else if (pid > 0) {
        child_pids[child_count] = pid;
        child_count++;
        printf(
            "Process with id: %d created for the command: %s\n", pid, argv[1]
        );
    } else if (pid < 0) {
        printf("Error creating new process(es)");
        exit(0);
    }

    int return_status;
    struct timespec stop, real_time;

    waitid(P_PID, pid, NULL, WNOWAIT);
    clock_gettime(CLOCK_MONOTONIC, &stop);
    char** stats = read_proc_file_alloc(pid, "stat", 44);
    char** statuses = read_proc_file_alloc(pid, "status", 93);
    
    waitpid(pid, &return_status, 0);
    int signaled = WIFSIGNALED(return_status);
    int signal_id = WTERMSIG(return_status);
    timespec_diff(&start, &stop, &real_time);
    double user_time = parse_stat_time(stats[14]);
    double sys_time = parse_stat_time(stats[15]);
    int context_switches = atoi(statuses[87]) + atoi(statuses[89]);

    if (signaled) {
        printf(
            (
                "The command \"%s\" is interrupted"
                " by the signal number = %d (%s)\n"
            ),
            argv[1], signal_id, SIGNAL_NAMES[signal_id]
        );
    } else {
        printf(
            (
                "The command \"%s\" terminated"
                " with returned status code = %d\n"
            ),
            argv[1], WTERMSIG(return_status)
        );
    }
    printf(
        (
            "real: %.02lf s, user: %.02lf s, system: %.02lf s,"
            " context switch: %d\n"
        ),
        to_double(&real_time), user_time, sys_time, context_switches
    );

    double_free(stats, 44);
    double_free(statuses, 93);
    double_free(stats, child_count - 1);
    return 0;
}
