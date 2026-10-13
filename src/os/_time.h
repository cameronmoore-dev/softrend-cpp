#pragma once

#include "typedefs.h"

typedef struct
{
    u64 sec;
    u64 ms;
    u64 us;
    u64 ns;
} timepoint_;

typedef struct
{
    u64 sec;
    u64 ms;
    u64 us;
    u64 ns;
} duration_;

typedef struct
{
    timepoint_ previous_time;
    timepoint_ current_time;
} timestep_;

void update_timestep(timestep_ *ts, f64 *delta_time);
timepoint_ get_time();
timepoint_ get_future_point(timepoint_ *start, duration_ *duration);
duration_ set_duration(f64 seconds);
duration_ timepoint_diff(timepoint_ *end, timepoint_ *start);