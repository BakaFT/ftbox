#define _POSIX_C_SOURCE
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../common.h"
#include "../limits/sandbox_limits.h"
#include "runner.h"

void CheckPrivilegeAndConfig(RunConfig* config, RunResult* result)
{
    // current user must be root
    if (getuid() != 0)
        SANDBOX_ERROR_EXIT(ROOT_REQUIRED);

    // check config
    if (config->exe_path == NULL || config->max_cpu_time < 1000 || config->max_real_time < 1000 || config->max_stack < 1 || config->max_memory < 1 || config->max_process_number < 1 || config->max_output_size < 1)
        SANDBOX_ERROR_EXIT(INVALID_CONFIG);
}

void ChildProcess(RunConfig* config, RunResult* result)
{
    // Redirect IO
    FILE *input_file = NULL, *output_file = NULL;
    if (config->input_path != NULL) {
        input_file = fopen(config->input_path, "r");
        if (input_file == NULL)
            SANDBOX_ERROR_EXIT(FILE_DESCRIPTOR_IS_NULL)
        if (dup2(fileno(input_file), fileno(stdin)) == -1) {
            SANDBOX_ERROR_EXIT(DUP2_FAILED)
        }
    }

    if (config->output_path != NULL) {
        output_file = fopen(config->output_path, "a");
        if (output_file == NULL) {
            SANDBOX_ERROR_EXIT(FILE_DESCRIPTOR_IS_NULL)
        }
        if (dup2(fileno(output_file), fileno(stdout)) == -1) {
            SANDBOX_ERROR_EXIT(DUP2_FAILED)
        }

        if (dup2(fileno(output_file), fileno(stderr)) == -1) {
            SANDBOX_ERROR_EXIT(DUP2_FAILED)
        }
    }

    SetTimerLimit(config, result);

    LoadSeccompRule(config, result);
    SetUser(config, result);
    // must put this function after all others, cause if RLIMIT_FSIZE is set, then
    // writing to LOG will get SIGXFSZ
    // TODO: fix this bug if necessary
    SetResourceLimit(config, result);
    // ready to go
    if (execve(config->exe_path, config->exe_args, config->exe_envs) == -1) {
        SANDBOX_ERROR_EXIT(EXECVE_FAILED)
    }
}

void InitResult(RunResult* result)
{
    result->cpu_time = 0;
    result->real_time = 0;
    result->memory = 0;
    result->signal = 0;
    result->exit_code = 0;
    result->error = 0;
    result->result = 0;
}

void SignalHandler(RunConfig* config, RunResult* result, int* status)
{
    // if child was killed by a signal
    // this won't be true if child exited normally
    if (WIFSIGNALED(*status) != 0) {
        // get the signal number
        result->signal = WTERMSIG(*status);
    } else {
        return;
    }

    switch (result->signal) {
    case SIGALRM:
        // CPU time limit exceeded(blocking time not included)
        JUDGE_RESULT(REAL_TIME_LIMIT_EXCEEDED)
        break;
    case SIGVTALRM:
        // REAL time limit exceeded(like sleeping etc.)
        JUDGE_RESULT(CPU_TIME_LIMIT_EXCEEDED)
        break;
    case SIGSYS:
        // SIGSYS is triggered by seccomp(SCMP_ACT_KILL) or illegal syscalls
        JUDGE_RESULT(RUNTIME_ERROR)
        break;
    case SIGSEGV:
        if (LIMITED(config->max_memory) && result->memory > config->max_memory) {
            JUDGE_RESULT(MEMORY_LIMIT_EXCEEDED)
        }
        // It's a liitle bit complicated to judge if it's a stack overflow so we
        // consider it as a runtime error
        // use sigaltstack(2) to implement a stack overflow detector if needed
        else {
            LOG_DEBUG("UNEXCEPTED_SIGSEGV");
            JUDGE_RESULT(RUNTIME_ERROR)
        }

        break;
    case SIGXFSZ:
        JUDGE_RESULT(OUTPUT_LIMIT_EXCEEDED)
        break;
    default:
        if (result->signal != 0) {
            LOG_DEBUG("UNEXCEPTED_SIGNAL: %d", result->signal);
            JUDGE_RESULT(RUNTIME_ERROR)
        } else {
            LOG_FATAL("WHAT THE FUCK");
        }
    }
}
void GenerateResult(pid_t child, int* status, RunConfig* config,
    RunResult* result, struct rusage* usage,
    struct timeval* start, struct timeval* end)
{
    if (wait4(child, status, 0, usage) == -1) {
        kill(child, SIGKILL);
        SANDBOX_ERROR_EXIT(WAIT_FAILED);
    }

    // update result
    gettimeofday(end, NULL);
    result->real_time = (uint32_t)(end->tv_sec * 1000 + end->tv_usec / 1000 - start->tv_sec * 1000 - start->tv_usec / 1000);

    result->exit_code = WEXITSTATUS(*status);
    result->cpu_time = (uint32_t)(usage->ru_utime.tv_sec * 1000 + usage->ru_utime.tv_usec / 1000);
    result->memory = (uint64_t)(usage->ru_maxrss * 1024);
    // handle judge result
    if (result->exit_code != 0) {
        JUDGE_RESULT(RUNTIME_ERROR)
    }
    SignalHandler(config, result, status);
}

void Run(RunConfig* config, RunResult* result)
{
    CheckPrivilegeAndConfig(config, result);
    InitResult(result);

    struct rusage* usage = malloc(sizeof(struct rusage));
    struct timeval* start = malloc(sizeof(struct timeval));
    struct timeval* end = malloc(sizeof(struct timeval));
    gettimeofday(start, NULL);
    int status;
    pid_t pid = fork();
    if (pid == 0) {
        ChildProcess(config, result);
    } else if (pid < 0) {
        SANDBOX_ERROR_EXIT(FORK_FAILED);
    } else {
        GenerateResult(pid, &status, config, result, usage, start, end);
    }
}
