#ifndef TFPS_H
#define TFPS_H

#include <stdbool.h>

/* Roshan.io frame limiter.
 * Sleeps (cheaply, using a high-resolution timer on Windows) until the frame
 * budget for `fps` is used up, then spins for the last ~0.4 ms so frame pacing
 * stays smooth. fps <= 0 means unlimited. Uses its own monotonic clock, so it
 * is not affected by glfwSetTime(). */
void tfps_limit(int fps);

/* Smoothed frames-per-second of the main loop (updated by tfps_limit). */
float tfps_current(void);

#endif
