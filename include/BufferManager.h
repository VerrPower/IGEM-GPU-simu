//
// Created by 31255 on 2025/9/2.
//

#ifndef COLLISION_SIMULATION_CP1_BUFFERMANAGER_H
#define COLLISION_SIMULATION_CP1_BUFFERMANAGER_H

#include <cglm/cglm.h>
#include <glad/glad.h>

typedef unsigned int BINDING_PORT;

#define __VOID__                0xffff

extern GLuint capsule_vao, circle_vao, bound_vao;
extern GLuint debug_buffer, output_buffer;
extern GLuint circle_struct_buffer;
extern GLuint device_int_buffer, device_float_buffer;
extern int *host_int_buffer;
extern float *host_float_buffer;

typedef struct Capsule {
    vec2 pos;   // 8
    vec2 prev_pos;   // ~16
    vec2 acc;  // 24
    float theta;  //28
    float prev_theta;   //~32
    float angular_acc;  //36
    int overlapped;    // 40
    int motion_state;  // 44
    int cooldown;   // ~48
}Capsule;

typedef struct CapsuleInfoBuf {
    vec2 run_coast_vel;
    float rotate_coast_omega;
    int run_coast_frames;
    int run_decel_frames;
    int rotate_coast_frames;
    int rotate_decel_frames;
    int _pad0;
}CapsuleInfoBuf;

typedef struct Circle {
    vec2 pos;
    float health;
    int _pad0;
}Circle;

typedef struct MotionBuffer{
    vec2 accel;
    float alpha;
    int _pad0;
}AccelBuffer;

void buffer_manager_init();
void buffer_manager_refill_host_buffer();
void buffer_manager_cleanup();

void fill_ssbo(GLuint ssbo, const void *data, GLsizeiptr size);
void fill_ubo(GLuint ubo, const void *data, GLsizeiptr size);
void * map_ssbo_write(GLuint ssbo, GLsizeiptr length);
void * map_ssbo_read(GLuint ssbo, GLsizeiptr length);
void unmap_ssbo(GLuint ssbo);

#endif //COLLISION_SIMULATION_CP1_BUFFERMANAGER_H