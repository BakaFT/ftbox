#include "../common.h"
#include <unistd.h>

void LoadSeccompRule(RunConfig* config, RunResult* result)
{
    // A simple reflection implementation in C, using GCC's section attribute.
    extern struct rule __start_rules;
    extern struct rule __stop_rules;

    struct rule* matched_rule = NULL;
    for (struct rule* p = &__start_rules; p < &__stop_rules; p++) {
        if (strcmp(p->rule_name, config->seccomp_rule) == 0) {
            p->fn(config);
            matched_rule = p;
            break;
        }
    }
    if (matched_rule == NULL) {
        SANDBOX_ERROR_EXIT(LOAD_SECCOMP_FAILED)
    }
}