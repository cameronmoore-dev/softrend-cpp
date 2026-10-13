#include "os/_time.h"

#include <time.h>

#include <stdio.h>

#define THOUSAND 1000L
#define MILLION  1000000L
#define BILLION  1000000000L

void update_timestep(timestep_ *ts, f64 *delta_time)
{
    ts->current_time = get_time();
    duration_ diff = timepoint_diff(&ts->current_time, &ts->previous_time);

    *delta_time = (f64)diff.ns / BILLION;
    ts->previous_time = ts->current_time;
}

timepoint_ get_time()
{
    struct timespec tspec;
    clock_gettime(CLOCK_MONOTONIC, &tspec);

    timepoint_ point = 
    {
        .sec = tspec.tv_sec,
        .ms  = (tspec.tv_sec * THOUSAND) + (tspec.tv_nsec / MILLION),
        .us  = (tspec.tv_sec * MILLION)  + (tspec.tv_nsec / THOUSAND),
        .ns  = (tspec.tv_sec * BILLION)  + (tspec.tv_nsec)
    };

    return point;
}

timepoint_ get_future_point(timepoint_ *start, duration_ *duration)
{
    timepoint_ point = 
    {
        .sec = start->sec + duration->sec,
        .ms  = start->ms  + duration->ms,
        .us  = start->us  + duration->us,
        .ns  = start->ns  + duration->ns,
    };

    return point;
}

duration_ set_duration(f64 seconds)
{
    duration_ duration = 
    {
        .sec = seconds,
        .ms  = seconds * THOUSAND,
        .us  = seconds * MILLION,
        .ns  = seconds * BILLION
    };

    return duration;
}

duration_ timepoint_diff(timepoint_ *end, timepoint_ *start)
{
    duration_ duration = 
    {
        .sec = end->sec - start->sec,
        .ms  = end->ms  - start->ms,
        .us  = end->us  - start->us,
        .ns  = end->ns  - start->ns
    };

    return duration;
}