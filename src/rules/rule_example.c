#include "../common.h"
#include <errno.h>
#include <seccomp.h>

// I recommend you to use SCMO_ACT_KILL to reject any syscall that is not
// allowed
// because it's handled in runner.c

int _example_rule(RunConfig* config)
{
    // Initialize the filter context with default action.
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL);
    // Start writing your rule here
    if (!ctx) {
        return LOAD_SECCOMP_FAILED;
    }
    if (seccomp_load(ctx) != 0) {
        return LOAD_SECCOMP_FAILED;
    }
    seccomp_release(ctx);
    return 0;
}

int example(RunConfig* config) { return _example_rule(config); }

// Remember to use register_rule() to register your rule
register_rule(example)