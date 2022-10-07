#ifndef SANDBOX_LIMITS_H
#define SANDBOX_LIMITS_H
#include "../common.h"

void SetResourceLimit(RunConfig* config, RunResult* result);
void SetTimerLimit(RunConfig* config, RunResult* result);
void LoadSeccompRule(RunConfig* config, RunResult* result);
void SetUser(RunConfig* config, RunResult* result);
#endif // SANDBOX_LIMITS_H
