#include "../common.h"

void SetResourceLimit(RunConfig* config, RunResult* result)
{
    if (LIMITED(config->max_memory)) {
        struct rlimit memory_limit;
        // Because virtual memory is normally much bigger than resident set size, so
        // we need this
        int mulitplier = 2;
        memory_limit.rlim_cur = memory_limit.rlim_max = config->max_memory * mulitplier;
        if (setrlimit(RLIMIT_AS, &memory_limit) == -1) {
            SANDBOX_ERROR_EXIT(SETRLIMIT_FAILED)
        }
    }
    if (LIMITED(config->max_stack)) {
        struct rlimit stack_limit;
        stack_limit.rlim_cur = stack_limit.rlim_max = config->max_stack;
        if (setrlimit(RLIMIT_STACK, &stack_limit) == -1) {
            SANDBOX_ERROR_EXIT(SETRLIMIT_FAILED)
        }
    }
    if (LIMITED(config->max_process_number)) {
        struct rlimit process_limit;
        process_limit.rlim_cur = process_limit.rlim_max = config->max_process_number;
        if (setrlimit(RLIMIT_NPROC, &process_limit) == -1) {
            SANDBOX_ERROR_EXIT(SETRLIMIT_FAILED)
        }
    }
    if (LIMITED(config->max_output_size) || config->output_path != NULL) {
        struct rlimit output_limit;
        output_limit.rlim_cur = output_limit.rlim_max = config->max_output_size;
        if (setrlimit(RLIMIT_FSIZE, &output_limit) == -1) {
            SANDBOX_ERROR_EXIT(SETRLIMIT_FAILED)
        }
    }
}