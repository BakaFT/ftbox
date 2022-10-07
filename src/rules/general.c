#include <errno.h>
#include <fcntl.h>
#include <linux/seccomp.h>
#include <seccomp.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../common.h"

int _general_rule(RunConfig *config)
{
    int syscalls_blacklist[] = {SCMP_SYS(clone), SCMP_SYS(fork), SCMP_SYS(vfork),
                                SCMP_SYS(kill),
#ifdef __NR_execveat
                                SCMP_SYS(execveat)
#endif
    };

    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ALLOW);
    if (!ctx)
    {
        return LOAD_SECCOMP_FAILED;
    }
    for (int i = 0; i < sizeof(syscalls_blacklist) / sizeof(int); i++)
    {
        if (seccomp_rule_add(ctx, SCMP_ACT_KILL, syscalls_blacklist[i], 0) != 0)
        {
            return LOAD_SECCOMP_FAILED;
        }
    }
    // use SCMP_ACT_KILL for socket, python will be killed immediately
    if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EACCES), SCMP_SYS(socket), 0) != 0)
    {
        return LOAD_SECCOMP_FAILED;
    }
    if (seccomp_rule_add(
            ctx, SCMP_ACT_KILL, SCMP_SYS(execve), 1,
            SCMP_CMP(0, SCMP_CMP_NE, (scmp_datum_t)(config->exe_path))) != 0)
    {
        return LOAD_SECCOMP_FAILED;
    }
    // do not allow "w" and "rw" using open
    if (seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(open), 1, SCMP_CMP(1, SCMP_CMP_MASKED_EQ, O_WRONLY, O_WRONLY)) != 0)
    {
        return LOAD_SECCOMP_FAILED;
    }
    if (seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(open), 1, SCMP_CMP(1, SCMP_CMP_MASKED_EQ, O_RDWR, O_RDWR)) != 0)
    {
        return LOAD_SECCOMP_FAILED;
    }
    // do not allow "w" and "rw" using openat
    if (seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(openat), 1, SCMP_CMP(2, SCMP_CMP_MASKED_EQ, O_WRONLY, O_WRONLY)) != 0)
    {
        return LOAD_SECCOMP_FAILED;
    }
    if (seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(openat), 1, SCMP_CMP(2, SCMP_CMP_MASKED_EQ, O_RDWR, O_RDWR)) != 0)
    {
        return LOAD_SECCOMP_FAILED;
    }
    if (seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(alarm), 1,
                         SCMP_CMP(0, SCMP_CMP_EQ, 0)) != 0)
    {
        return LOAD_SECCOMP_FAILED;
    }

    if (seccomp_load(ctx) != 0)
    {
        return LOAD_SECCOMP_FAILED;
    }
    seccomp_release(ctx);
    return 0;
}

int general(RunConfig *config) { return _general_rule(config); }

reflect(general)
