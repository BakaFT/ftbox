#define _DEFAULT_SOURCE
#include "common.h"
#include "logger/logger.h"
#include "parser/parser.h"
#include "runner/runner.h"
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

extern struct argp argp;
int main(int argc, char** argv)
{
    if (argc == 1) {
        argp_help(&argp, stdout, ARGP_HELP_DOC, argv[0]);
        argp_help(&argp, stdout, ARGP_HELP_SHORT_USAGE | ARGP_HELP_LONG, argv[0]);
        return 0;
    }
    RunConfig* config = malloc(sizeof(RunConfig));
    // Since RunResult *result will be a shared memory(which will be modified by
    // child)
    // so we need to use mmap to avoid copy-on-write on forking
    RunResult* result = mmap(NULL, sizeof(RunResult), PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    INITIALIZE_LOGGER(DEBUG)
    ParseCommandline(argc, argv, config);
    Run(config, result);
    printf("{\"cpu_time\":%lu,\"real_time\":%lu,\"memory\":%lu,\"signal\":%d,"
           "\"exit_code\":%d,\"error\":%d,\"result\":%d}\n",
        result->cpu_time, result->real_time, result->memory, result->signal,
        result->exit_code, result->error, result->result);
    LOG_INFO("{\"cpu_time\":%lu,\"real_time\":%lu,\"memory\":%lu,\"signal\":%d,"
             "\"exit_code\":%d,\"error\":%d,\"result\":%d}\n",
        result->cpu_time, result->real_time, result->memory, result->signal,
        result->exit_code, result->error, result->result);

    return 0;
}