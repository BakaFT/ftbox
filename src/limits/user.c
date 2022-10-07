#define _DEFAULT_SOURCE
#include "../common.h"
#include <grp.h>
#include <sys/types.h>
#include <unistd.h>

void SetUser(RunConfig* config, RunResult* result)
{
    // We run child by user "nobody", which is hard coded in InitConfig()
    gid_t* gid_nobody = &(config->gid);
    if ((setgid(config->gid) == -1 || setgroups(1, gid_nobody) == -1)) {
        SANDBOX_ERROR_EXIT(SETGID_FAILED)
    }
    if (setuid(config->uid) == -1) {
        SANDBOX_ERROR_EXIT(SETUID_FAILED)
    }
}