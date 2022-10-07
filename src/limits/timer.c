#include "../common.h"
#include <sys/time.h>
#include <unistd.h>

void ConvertTime(rlim_t millisecond, struct itimerval* timer)
{
    // Convert given millisecond to second and microsecond, where microsecond is
    // <= 999999
    time_t microsecond = millisecond % 1000 * 1000;
    suseconds_t second = millisecond / 1000;

    timer->it_interval.tv_sec = 0;
    timer->it_interval.tv_usec = 0;
    timer->it_value.tv_sec = second;
    timer->it_value.tv_usec = microsecond;
}

void SetTimerLimit(RunConfig* config, RunResult* result)
{
    // We use setitimer to get more accurate time limit
    struct itimerval cpu_timer;
    struct itimerval real_timer;
    ConvertTime(config->max_cpu_time, &cpu_timer);
    ConvertTime(config->max_real_time, &real_timer);
    if (setitimer(ITIMER_VIRTUAL, &cpu_timer, NULL) == -1) {
        SANDBOX_ERROR_EXIT(SETITIMER_FAILED)
    }
    if (setitimer(ITIMER_REAL, &real_timer, NULL) == -1) {
        SANDBOX_ERROR_EXIT(SETITIMER_FAILED)
    }
}