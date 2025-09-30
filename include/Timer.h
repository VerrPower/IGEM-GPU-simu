// timer.h
#ifndef TIMER_H
#define TIMER_H

#include <windows.h>
#include <stdbool.h>
#include <stdio.h>

static double clock_freq;
static bool timer_initialized = false;

static inline void timer_init(void) {
    LARGE_INTEGER f;
    if (!QueryPerformanceFrequency(&f)) {
        fprintf(stderr, "QueryPerformanceFrequency failed\n");
        return;
    }
    clock_freq = (double)f.QuadPart;
    timer_initialized = true;
}

static inline double time_getSec(void) {
    if (!timer_initialized) {
        printf("forced timer init called\n");
        timer_init();
    }
    LARGE_INTEGER cnt;
    QueryPerformanceCounter(&cnt);
    return (double)cnt.QuadPart / clock_freq;
}

#endif
