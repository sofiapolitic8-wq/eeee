#include "tfps.h"

#include <stdint.h>

#ifdef _WIN32
#include <windows.h>

#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif

static double s_freq = 0.0;
static HANDLE s_timer = NULL;
static bool s_timer_tried = false;

static double now_sec(void) {
  LARGE_INTEGER c;
  if (s_freq == 0.0) {
    LARGE_INTEGER f;
    QueryPerformanceFrequency(&f);
    s_freq = (double)f.QuadPart;
  }
  QueryPerformanceCounter(&c);
  return (double)c.QuadPart / s_freq;
}

static void sleep_sec(double sec) {
  if (sec <= 0.0) return;
  if (!s_timer_tried) {
    s_timer_tried = true;
    s_timer = CreateWaitableTimerExW(NULL, NULL,
                                     CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
                                     TIMER_ALL_ACCESS);
  }
  if (s_timer) {
    LARGE_INTEGER due;
    due.QuadPart = -(LONGLONG)(sec * 1e7); /* 100 ns units, relative */
    if (SetWaitableTimer(s_timer, &due, 0, NULL, NULL, FALSE)) {
      WaitForSingleObject(s_timer, 100);
      return;
    }
  }
  /* Fallback: plain Sleep (coarse, but still saves CPU). */
  if (sec >= 0.002) Sleep((DWORD)(sec * 1000.0) - 1);
}
#else
#include <time.h>

static double now_sec(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static void sleep_sec(double sec) {
  if (sec <= 0.0) return;
  struct timespec ts;
  ts.tv_sec = (time_t)sec;
  ts.tv_nsec = (long)((sec - (double)ts.tv_sec) * 1e9);
  nanosleep(&ts, NULL);
}
#endif

static double s_next = 0.0;
static double s_prev = 0.0; /* start time of the previous call */
static float s_fps = 0.0f;

float tfps_current(void) { return s_fps; }

void tfps_limit(int fps) {
  double t = now_sec();

  if (s_prev > 0.0) {
    double dt = t - s_prev;
    if (dt > 0.0) {
      float inst = (float)(1.0 / dt);
      s_fps = s_fps == 0.0f ? inst : s_fps + (inst - s_fps) * 0.05f;
    }
  }

  if (fps > 0) {
    double budget = 1.0 / (double)fps;
    if (s_next == 0.0 || t - s_next > budget * 4.0) s_next = t; /* resync */
    s_next += budget;

    double remain = s_next - t;
    if (remain > 0.0006) sleep_sec(remain - 0.0004); /* coarse wait */
    while (now_sec() < s_next) {                      /* fine wait */
    }
  } else {
    s_next = 0.0;
  }

  s_prev = t;
}
