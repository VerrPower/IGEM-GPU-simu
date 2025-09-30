//
// Created by 31255 on 2025/9/2.
//
#include "stdint.h"
#include "stdio.h"
#include "Timer.h"
#include "Engine.h"
#include "BufferManager.h"    // - glad
#include "ShaderManager.h"    // - glad
#include "WindowManager.h"
#include "InputManager.h"
#include "Camera.h"
#include "HostParams.h"
#include <GLFW/glfw3.h>

#include "ParallelRefiller.h"

static bool engine_initialized = false;
static float total_energy;

inline void update();

inline void render();

void engine_init() {
    timer_init();
    input_clear_state();
    window_manager_init();
    shader_manager_init();
    buffer_manager_init();
    camera_register(window_get_winH_ptr(), window_get_winW_ptr());
    camera_init_matrix();
    glfwShowWindow(global_window);
    engine_initialized = true;
    printf("@ Engine Initialized\n\n");fflush(stdout);
}


void engine_run() {
    if (!engine_initialized)
        printf("Engine has not been initialized\n");fflush(stdout);
    printf("\t engine running...\n\n");fflush(stdout);
    uint32_t frameRenderCount = 0, logicUpdateCount = 0;
    double unprocRender = 0.0, unprocLogic = 0.0, unprocInfoUpdate = 0.0,
            smooth_fps = 0.0, last = time_getSec();
    while (!glfwWindowShouldClose(global_window)) {
        double now = time_getSec(); double delta = now - last; last = now;
        unprocRender += delta; unprocLogic += delta; unprocInfoUpdate += delta;
        glfwPollEvents();
        if (unprocLogic >= LOGIC_INTERVAL) {
            update();
            do{unprocLogic -= LOGIC_INTERVAL;}
            while (unprocLogic >= LOGIC_INTERVAL);
            logicUpdateCount ++;
        }
        if (unprocRender >= RENDER_INTERVAL) {
            render();
            do{unprocRender -= RENDER_INTERVAL;}
            while (unprocRender >= RENDER_INTERVAL);
            frameRenderCount ++;
        }
        if (unprocInfoUpdate >= INFO_UPDATE_INTERVAL) {
            do{unprocInfoUpdate -= INFO_UPDATE_INTERVAL;}
            while (unprocInfoUpdate >= INFO_UPDATE_INTERVAL);
            double current_fps = (double) frameRenderCount / INFO_UPDATE_INTERVAL;
            smooth_fps = FRAME_RATE_SMOOTHING * smooth_fps + (1-FRAME_RATE_SMOOTHING) * current_fps;
            char title[128]; snprintf(title, sizeof(title),
                "Simulation template DECAP v1.0 - FPS: %.0f - Total energy： %.3f",smooth_fps, total_energy);
            glfwSetWindowTitle(global_window, title);
            frameRenderCount = 0;
        }
    }
}

inline void update() {
    input_poll_key_events();

    parafill_invoke_workers();
    int sz_DFW = NUM_CAPSULE * 4 * sizeof(float);
    float *mapped_device_float_buffer = map_ssbo_write(device_float_buffer, sz_DFW);
    memcpy(mapped_device_float_buffer, host_float_buffer, sz_DFW);
    unmap_ssbo(device_float_buffer);
    int sz_DIW = NUM_CAPSULE * 1 * sizeof(int);
    int *mapped_device_int_buffer = map_ssbo_write(device_int_buffer, sz_DIW);
    memcpy(mapped_device_int_buffer, host_int_buffer, sz_DIW);
    unmap_ssbo(device_int_buffer);

#if defined(VALIDATE_NORMAL) || defined(VALIDATE_EXPO)
    float *mapped_DFR = map_ssbo_read(device_float_buffer, sz_DFW);
#endif
#ifdef VALIDATE_NORMAL
    float sum_n = 0, EX_n = 0, VarX_n = 0; int N_n = NUM_CAPSULE * 3, base_n = 0;
    for (int i = 0; i < N_n; ++i) sum_n += mapped_DFR[base_n + i];
    EX_n = sum_n / (float)N_n;
    for (int i = 0; i < N_n; ++i) VarX_n += (mapped_DFR[base_n + i] - EX_n) * (mapped_DFR[base_n + i] - EX_n);
    VarX_n /= (float)N_n;
    printf("[normal] \tN = %d\t mean = %.4f\t variance = %.4f\n", N_n, EX_n, VarX_n);
#endif
#ifdef VALIDATE_EXPO
    float sum_e = 0, EX_e = 0, VarX_e = 0; int N_e = NUM_CAPSULE * 1, base_e = NUM_CAPSULE * 3;
    for (int i = 0; i < N_e; ++i) sum_e += mapped_DFR[base_e + i];
    EX_e = sum_e / (float)N_e;
    for (int i = 0; i < N_e; ++i) VarX_e += (mapped_DFR[base_e + i] - EX_e) * (mapped_DFR[base_e + i] - EX_e);
    VarX_e /= (float)N_e;
    printf("[expo]  \tM = %d\t mean = %.4f\t variance = %.4f\n", N_e, EX_e, VarX_e);
#endif
#if defined(VALIDATE_NORMAL) || defined(VALIDATE_EXPO)
    unmap_ssbo(device_float_buffer);
#endif
#ifdef VALIDATE_TWOP
    int sum_2p = 0; int N_2p = NUM_CAPSULE;
    int *mapped_DIR = map_ssbo_read(device_int_buffer, sz_DIW);
    for (int i = 0; i < N_2p; ++i) sum_2p += mapped_DIR[i];
    float ratio = (float)sum_2p / (float)N_2p;
    printf("[2point] \tP = %d\t ratio of digit 1 = %.4f\n",N_2p, ratio);
    unmap_ssbo(device_int_buffer);
#endif
#if defined(VALIDATE_TWOP) || defined(VALIDATE_EXPO) || defined(VALIDATE_NORMAL)
    printf("# Validate ----------------------------------------------\n");
#endif

    // clear
    shader_manager_use_program(comp_clr);
    glDispatchCompute(WG_NUM_CELL, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // insert
    shader_manager_use_program(comp_ins);
    glDispatchCompute(WG_NUM_CAP, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // collide
    shader_manager_use_program(comp_col);
    glDispatchCompute(WG_NUM_CAP, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    /*int * device_int = map_ssbo_read(debug_buffer, sizeof(int));
    //record[*device_int]++;
    printf("int: %d\n", *device_int);
    unmap_ssbo(debug_buffer);*/

#ifdef SAVE_SURVIVAL_SEQUENCE
    static bool write_and_stop = false;
    static int SAMPLE_COUNT = 512;
    static int buffer[1000], _ptr = 0;
    static double t1 = 0;

    if (write_and_stop) {
        FILE *fp = fopen("D:/checkpoint - RAT/output/surv_curve.txt", "w");
        if (fp == NULL)  perror("Failed to open file");
        for (int i = 0; i < SAMPLE_COUNT; i++)  fprintf(fp, "%d\n", buffer[i]); fclose(fp);
        printf("Array written to output.txt\n");
        glfwSetWindowShouldClose(global_window, GLFW_TRUE); return;
    }
    if (time_getSec() - t1 > 0.5) {
        t1 = time_getSec();
        Circle *cmap = map_ssbo_read(circle_struct_buffer, NUM_CIRCLE * sizeof(Circle));
        int alive = 0;for (int i = 0; i < NUM_CIRCLE; ++i) alive += cmap[i].health >= 0? 1 : 0;
        unmap_ssbo(circle_struct_buffer);buffer[_ptr++] = alive;printf("%d\n",alive);
        write_and_stop = (alive <= 1 || _ptr >= SAMPLE_COUNT - 1);
    }
#endif

    // update
    shader_manager_use_program(comp_upd);
    glDispatchCompute(WG_NUM_CAP, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // read back kinetic energy
    float *ptr = map_ssbo_read(output_buffer, NUM_CAPSULE);
    total_energy = 0.0f; for (int i = 0; i < NUM_CAPSULE; ++i) total_energy += ptr[i];
    unmap_ssbo(output_buffer);

    fflush(stdout);
}



inline void render() {
    float _255f_ = 255.0f;
    float color[3] = {247,241,235};
    glClearColor(color[0] / _255f_, color[1]/_255f_, color[2]/_255f_, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    camera_update_ortho_projection_matrix();
    glLineWidth(PARTICLE_LINE_WIDTH);

    //draw Cauris
    shader_manager_use_render_and_set(render_B, loc_P_B, loc_V_B, projectionMatrix, viewMatrix);
    glBindVertexArray(circle_vao);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, VERTEX_COUNT+2, NUM_CIRCLE);

    //draw Ecoli
    shader_manager_use_render_and_set(render_A, loc_P_A, loc_V_A, projectionMatrix, viewMatrix);
    glBindVertexArray(capsule_vao);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, VERTEX_COUNT, NUM_CAPSULE);

    //draw boundary
    shader_manager_use_render_and_set(render_F, loc_P_F, loc_V_F, projectionMatrix, viewMatrix);
    glLineWidth(0.5f);
    glBindVertexArray(bound_vao);
    glDrawArrays(GL_LINE_LOOP, 0, 4);

    glfwSwapBuffers(global_window);
}


void engine_close() {

    buffer_manager_cleanup();
    window_manager_cleanup();
    glfwTerminate();
    printf("@ Engine closed\n");
}
