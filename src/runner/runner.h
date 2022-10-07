#ifndef RUNNER_H
#define RUNNER_H
#include "../common.h"
#include "../logger/logger.h"

#define JUDGE_RESULT(JudgeResultCode)     \
    {                                     \
        result->result = JudgeResultCode; \
    }

void Run(RunConfig* config, RunResult* result);
void InitResult(RunResult* result);
void SetRlimit(RunConfig* config, RunResult* result);
#endif // RUNNER_H