//
// Created by 31255 on 2025/9/3.
//

#include "stdio.h"
#include "BufferManager.h"
#include "ParallelRefiller.h"

// ============================Multithread Settings===========================

int   *host_int_buffer;
float *host_float_buffer;

volatile bool task_running;
int st_norm, end_norm, st_expo, end_expo, st_2p, end_2p;
PogiRandom generators[NUM_THREADS];
ThreadTaskPacked task_packages[NUM_THREADS];
HANDLE hStart, hContinue;
HANDLE hDone[NUM_THREADS];
HANDLE hThreads[NUM_THREADS];

void register_host_buffer(int *hostInt, float *hostFloat) {
    host_int_buffer   = hostInt;
    host_float_buffer = hostFloat;
}

void parafill_set_fill_segment(int _norm_st,int _norm_end,int _expo_st,int _expo_end,int _twop_st,int _twop_end) {
    st_norm = _norm_st; end_norm = _norm_end;
    st_expo = _expo_st; end_expo = _expo_end;
    st_2p = _twop_st; end_2p = _twop_end;
}

static DWORD WINAPI task_manager_func(LPVOID packed) {
    ThreadTaskPacked *task = packed;
    int tid = task->tid;
    MainTaskFunc task_main_function = task->func;
    MainTaskFuncArgs *args = task->args;
    while (task_running) {
        ResetEvent(hDone[tid]);
        WaitForSingleObject(hStart, INFINITE);
        if (task_running) task_main_function(args);
        SetEvent(hDone[tid]);
        WaitForSingleObject(hContinue, INFINITE);
    }
    return 0;
};

void shared_array_filling_func(const MainTaskFuncArgs *args) {
    PogiRandom *rnd = args->rnd;
    int   *int_buf   = args->int_buf;
    float *float_buf = args->float_buf;
    for (int i = args->local_st_expo; i < args->local_end_expo; ++i) float_buf[i] = (float)rnd_expo(rnd);
    for (int i = args->local_st_norm; i < args->local_end_norm; ++i) float_buf[i] = (float)rnd_norm(rnd);
    for (int i = args->local_st_twop; i < args->local_end_twop; ++i)   int_buf[i] = rnd_2pnt(rnd);
}

void parafill_init_threads() {
    rnd_init_ziggurat();
    int chunk_2p   = (end_2p   - st_2p   + NUM_THREADS - 1) / NUM_THREADS;  // i
    int chunk_norm = (end_norm - st_norm + NUM_THREADS - 1) / NUM_THREADS;  // f
    int chunk_expo = (end_expo - st_expo + NUM_THREADS - 1) / NUM_THREADS;  // f
    for (int tid = 0; tid < NUM_THREADS; tid++) {
        PogiRandom *threadRnd = &generators[tid];
        rnd_update_seed(threadRnd);
        rnd_set_2pnt_param(threadRnd, 0.5);
        int local_st_2p    = st_2p   + chunk_2p   * tid;
        int local_end_2p   = min(end_2p,    local_st_2p   + chunk_2p);
        int local_st_expo  = st_expo + chunk_expo * tid;
        int local_end_expo = min(end_expo, local_st_expo + chunk_expo);
        int local_st_norm  = st_norm + chunk_norm * tid;
        int local_end_norm = min(end_norm, local_st_norm + chunk_norm);
        MainTaskFuncArgs *mainFuncArgs = malloc(sizeof(MainTaskFuncArgs));
        mainFuncArgs->rnd = threadRnd;
        mainFuncArgs->int_buf   = host_int_buffer;
        mainFuncArgs->float_buf = host_float_buffer;
        mainFuncArgs->local_st_twop  = local_st_2p;
        mainFuncArgs->local_st_norm  = local_st_norm;
        mainFuncArgs->local_st_expo  = local_st_expo;
        mainFuncArgs->local_end_norm = local_end_norm;
        mainFuncArgs->local_end_twop = local_end_2p;
        mainFuncArgs->local_end_expo = local_end_expo;
        ThreadTaskPacked *packed = &task_packages[tid];
        packed->args = mainFuncArgs;
        packed->func = shared_array_filling_func;
        packed->tid  = tid;
    }
    hStart    = CreateEvent(NULL, TRUE, FALSE, NULL);
    hContinue = CreateEvent(NULL, TRUE, FALSE, NULL);
    task_running = true;
    for (int tid = 0; tid < NUM_THREADS; tid++) {
        hDone[tid] = CreateEvent(NULL, TRUE, FALSE, NULL);
        hThreads[tid] = CreateThread(NULL, 0, task_manager_func, &task_packages[tid], 0, NULL);
    }
}

void parafill_invoke_workers() {
    ResetEvent(hContinue);
    SetEvent(hStart);
    WaitForMultipleObjects(NUM_THREADS, hDone,TRUE,INFINITE);
    ResetEvent(hStart);
    SetEvent(hContinue);
}

void parafill_cleanup() {
    task_running = false;
    SetEvent(hStart);
    SetEvent(hContinue);
    WaitForMultipleObjects(NUM_THREADS, hThreads, TRUE, INFINITE); //upd
    CloseHandle(hStart);
    CloseHandle(hContinue);
    for (int i = 0; i < NUM_THREADS; i++) CloseHandle(hDone[i]);
    for (int i = 0; i < NUM_THREADS; i++) CloseHandle(hThreads[i]);
    for (int i = 0; i < NUM_THREADS; i++) free(task_packages[i].args);
    printf("@ ParaFill cleanup done\n");
}