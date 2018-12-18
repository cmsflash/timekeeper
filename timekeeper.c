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

char*** allocate_file_buffers(const int file_count, const int line_count) {
    char*** file_buffers = (char***)malloc(file_count * sizeof(char**));
    for (int i = 0; i < file_count; i++) {
        file_buffers[i] = (char**)malloc(line_count * sizeof(char*));
        for (int j = 0; j < line_count; j++) {
            file_buffers[i][j] = malloc(DEFAULT_STRING_SIZE);
        }
    }
    return file_buffers;
}

char** read_proc_file(
        char** file_buffer, const int pid, const char* filename,
        const int line_count
    ) {
    char* path = (char*)malloc(DEFAULT_STRING_SIZE);
    sprintf(path, "/proc/%d/%s", pid, filename);
    FILE* file = fopen(path, "r");
    for (int i = 0; i < line_count; i++) {
        file_buffer[i] = malloc(DEFAULT_STRING_SIZE);
        fscanf(file, "%s", file_buffer[i]);
    }
    fclose(file);
    free(path);
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

void triple_free(const void*** pointer, const int length0, const int lenght1) {
    for (int i = 0; i < length0; i++) {
        double_free(pointer[i], lenght1);
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

void close_pipes_except(int** pipes, int count, int read, int write) {
    for (int i = 0; i < count; i++) {
        if (i == read) {
            close(pipes[1]);
        } else if (i == write) {
            close(pipes[0]);
        } else {
            close(pipes[0]);
            close(pipes[1]);
        }
    }
}

int is_pipe_symbol(char c) {
    result = strcmp(argv[i], "!") == 0 || strcmp(argv[i], "|") == 0;
    return result;
}

int main(int argc, char** argv) {
    if (argc == 1) {
        return;
    }
    signal(SIGINT, deliver_to_child);
    int process_count = 1;
    for (int i = 1; i < argc; i++) {
        if (is_pipe_symbol(argv[i])) {
            process_count++;
        }
    }
    child_pids = (pid_t*)malloc(process_count * sizeof(pid_t));
    
    int* argcs = (int *)malloc(process_count * sizeof(int));
    int argv_index = 1;
    for (int i = 0; i < process_count; i++) {
        argcs[i] = 0;
        while (argv_index < argc && is_pipe_symbol(argv[argv_index])) {
            argcs[i]++;
            argv_index++;
        }
        argv_index++;
    }
    char*** argvs = (char ***)malloc(process_count * sizeof(char**));
    argv_index = 0;
    for (int i = 0; i < process_count; i++) {
        argvs[i] = (char**)malloc((argcs[i] + 1) * sizeof(char*));
        argv_index++;
        for (int j = 0; j < argcs[i]; j++) {
            argvs[i][j] = argv[argv_index];
            argv_index++;
        }
        argvs[i][argcs[i]] = NULL;
    }

    int** pipes = create_pipes_alloc(process_count - 1);

    struct timespec* starts = (struct timespec*)malloc(
        process_count * sizeof(struct timespec)
    );
    for (int i = 0; i < process_count; i++) {
        clock_gettime(CLOCK_MONOTONIC, &starts[i]);
        pid_t pid = fork();
        if (pid == 0) {
            close_pipes_except(pipes, process_count - 1, i - 1, i);
            printf("%d", i);
            if (i - 1 >= 0) {
                dup2(pipes[i - 1][0], 0);
            }
            if (i < process_count - 1) {
                dup2(pipes[i][1], 1);
            }
            execute(argvs[i][0], argvs[i]);
            exit(0);
        } else if (pid > 0) {
            child_pids[i] = pid;
            child_count++;
            printf(
                "Process with id: %d created for the command: %s\n",
                pid, argvs[i][0]
            );
        } else if (pid < 0) {
            printf("Error creating new process(es)");
            exit(0);
        }
    }

    int* return_statuses = (int*)malloc(process_count * sizeof(int));
    struct timespec* stops = (struct timespec*)malloc(
        process_count * sizeof(struct timespec)
    );
    struct timespec* real_times = (struct timespec*)malloc(
        process_count * sizeof(struct timespec)
    );
    char*** stats = allocate_file_buffers(process_count, 44);
    char*** statuses = allocate_file_buffers(process_count, 93);

    for (int i = 0; i < process_count; i++) {
        waitid(P_PID, child_pids[i], NULL, WEXITED | WNOWAIT);
        clock_gettime(CLOCK_MONOTONIC, &stops[i]);
        read_proc_file(stats[i], child_pids[i], "stat", 44);
        read_proc_file(statuses[i], child_pids[i], "status", 93);
    }
    for (int i = 0; i < process_count; i++) {
        waitpid(child_pids[i], &return_statuses[i], 0);
        child_count--;
        int signaled = WIFSIGNALED(return_statuses[i]);
        int signal_id = WTERMSIG(return_statuses[i]);
        timespec_diff(&starts[i], &stops[i], &real_times[i]);
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
                argv[1], WTERMSIG(return_statuses[i])
            );
        }
        printf(
            (
                "real: %.02lf s, user: %.02lf s, system: %.02lf s,"
                " context switch: %d\n"
            ),
            to_double(&real_times[i]), user_time, sys_time, context_switches
        );
    }

    triple_free(stats, process_count, 44);
    triple_free(statuses, process_count, 93);
    double_free(pipes, process_count - 1);
    free(starts);
    free(stops);
    free(real_times);
    return 0;
}
