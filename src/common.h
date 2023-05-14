#ifndef COMMON_H
#define COMMON_H
#include "config.h"
#include "logger/logger.h"
#include <stdint.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
typedef struct {
    rlim_t max_cpu_time;
    rlim_t max_real_time;
    rlim_t max_memory;

    rlim_t max_stack;
    rlim_t max_process_number;
    rlim_t max_output_size;

    char* exe_path;
    char** exe_args;
    char** exe_envs;
    char* input_path;
    char* output_path;
    char* log_path;
    char* seccomp_rule;

    uid_t uid;
    gid_t gid;

} RunConfig;

typedef struct {
    rlim_t cpu_time;
    rlim_t real_time;
    rlim_t memory;
    int signal; /* 0 if not killed by signal */
    int exit_code; /* exit code of process */
    int error; /* one of SandboxExitCode */
    int result; /* one of JudgeResultCode */
} RunResult;

typedef enum {
    SUCCESS, /* everything is ok */
    INVALID_CONFIG, /* invalied config */
    FORK_FAILED, /* run fork() failed */
    PTHREAD_FAILED, /* run child thread failed */
    WAIT_FAILED, /* run wait4() failed */
    DUP2_FAILED, /* run dup2() failed */
    SETITIMER_FAILED, /* run setitimer() failed */
    SETRLIMIT_FAILED, /* run setrlimit() failed */
    SETUID_FAILED, /* run setuid() failed */
    SETGID_FAILED, /* run setgid() failed */
    FILE_DESCRIPTOR_IS_NULL, /* file descriptor is null */
    LOAD_SECCOMP_FAILED, /* load seccomp rules failed */
    EXECVE_FAILED, /* run execve() failed */
    ROOT_REQUIRED, /* sandbox needs root privilege */
} SandboxExitCode;

typedef enum {
    CPU_TIME_LIMIT_EXCEEDED = 1,
    REAL_TIME_LIMIT_EXCEEDED = 2,
    MEMORY_LIMIT_EXCEEDED = 3,
    STACK_LIMIT_EXCEEDED = 4,
    OUTPUT_LIMIT_EXCEEDED = 5,
    RUNTIME_ERROR = 6,
    // This will never be given by sandbox, just for consistency
    WRONG_ANSWER = 7,
} JudgeResultCode;

typedef struct {
    pid_t pid;
    int time_limit;
} timeout_info;

struct rule {
    int (*fn)(RunConfig* config);
    const char* rule_name;
};

#define reflect(x) \
    __attribute__((section("rules"))) struct rule __##x = { x, #x };

#define SANDBOX_ERROR_EXIT(x)                            \
    {                                                    \
        LOG_ERROR("[Sandbox] code:%d, error:%s", x, #x); \
        result->error = x;                               \
        exit(0);                                         \
    }
#define INITIALIZE_LOGGER(level)                                     \
    {                                                                \
        if (config->log_path) {                                      \
            logger_initFileLogger(config->log_path, 1024 * 1024, 5); \
            logger_autoFlush(1);                                     \
            logger_setLevel(LogLevel_##level);                       \
        } else {                                                     \
            logger_initFileLogger("sandbox.log", 1024 * 1024, 5);    \
            logger_autoFlush(1);                                     \
            logger_setLevel(LogLevel_##level);                       \
        }                                                            \
    }

#define LIMITED(resource) (resource != RLIM_INFINITY)

#endif // JUDGER_RUNNER_H
