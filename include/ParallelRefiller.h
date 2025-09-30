//
// Created by 31255 on 2025/9/3.
//

#ifndef COLLISION_SIMULATION_CP1_PARAFILL_H
#define COLLISION_SIMULATION_CP1_PARAFILL_H

#include "Windows.h"
#include "stdbool.h"
#include "PogiRandom.h"
#define NUM_THREADS 15

typedef int ARRAY_ID;

typedef struct MainTaskFuncArgs {
    PogiRandom *rnd;
    int   *int_buf;
    float *float_buf;
    int local_st_expo, local_end_expo;
    int local_st_norm, local_end_norm;
    int local_st_twop, local_end_twop;
} MainTaskFuncArgs;

typedef void (*MainTaskFunc) (const MainTaskFuncArgs *);

typedef struct ThreadTaskPacked {
    int tid;
    MainTaskFuncArgs *args;
    MainTaskFunc func;
} ThreadTaskPacked;


void register_host_buffer(int *hostInt, float *hostFloat);
void parafill_set_fill_segment(int _norm_st,int _norm_end,int _expo_st,int _expo_end,int _twop_st,int _twop_end);
void parafill_init_threads();
void parafill_invoke_workers();
void parafill_cleanup();

#endif //COLLISION_SIMULATION_CP1_PARAFILL_H