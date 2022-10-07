#include "../common.h"
#include <errno.h>
#include <fcntl.h>
#include <seccomp.h>

int _c_cpp_rule(RunConfig* config, int allow_write_file)
{
    int syscalls_whitelist[] = {
        SCMP_SYS(read), SCMP_SYS(fstat), SCMP_SYS(mmap),
        SCMP_SYS(mprotect), SCMP_SYS(munmap), SCMP_SYS(uname),
        SCMP_SYS(arch_prctl), SCMP_SYS(brk), SCMP_SYS(access),
        SCMP_SYS(exit_group), SCMP_SYS(close), SCMP_SYS(readlink),
        SCMP_SYS(sysinfo), SCMP_SYS(write), SCMP_SYS(writev),
        SCMP_SYS(lseek), SCMP_SYS(clock_gettime), SCMP_SYS(pread64)
    };

    // All syscalls are disabled by default by reason of EPERM
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL);
    if (!ctx) {
        return LOAD_SECCOMP_FAILED;
    }
    // Allow syscalls in the white list
    for (int i = 0; i < sizeof(syscalls_whitelist) / sizeof(int); i++) {
        if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, syscalls_whitelist[i], 0) != 0) {
            return LOAD_SECCOMP_FAILED;
        }
    }

    if (seccomp_rule_add(
            ctx, SCMP_ACT_ALLOW, SCMP_SYS(execve), 1,
            SCMP_CMP(0, SCMP_CMP_EQ, (scmp_datum_t)(config->exe_path)))
        != 0) {
        return LOAD_SECCOMP_FAILED;
    }
    if (!allow_write_file) {
        // only O_RDONLY is allowed
        if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 1,
                SCMP_CMP(1, SCMP_CMP_MASKED_EQ, O_ACCMODE, 0))
            != 0) {
            return LOAD_SECCOMP_FAILED;
        }
        if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(openat), 1,
                SCMP_CMP(2, SCMP_CMP_MASKED_EQ, O_ACCMODE))
            != 0) {
            return LOAD_SECCOMP_FAILED;
        }
    } else {
        if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 0) != 0) {
            return LOAD_SECCOMP_FAILED;
        }
        if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(openat), 0) != 0) {
            return LOAD_SECCOMP_FAILED;
        }
    }

    if (seccomp_load(ctx) != 0) {
        return LOAD_SECCOMP_FAILED;
    }
    seccomp_release(ctx);
    return 0;
}

int c_cpp(RunConfig* config) { return _c_cpp_rule(config, 0); }

int c_cpp_with_io(RunConfig* config) { return _c_cpp_rule(config, 1); }
reflect(c_cpp_with_io) reflect(c_cpp)
