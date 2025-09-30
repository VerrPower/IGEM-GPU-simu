//
// Created by 31255 on 2025/9/2.
//

#ifndef COLLISION_SIMULATION_CP1_CAMERA_H
#define COLLISION_SIMULATION_CP1_CAMERA_H

#include <cglm/cglm.h>

extern vec3 cameraPos, cameraDir;
extern float cameraYaw, cameraPitch;
extern float ortho_l, ortho_r, ortho_b, ortho_t;
extern mat4 projectionMatrix, viewMatrix;

void camera_register(int *winH, int *winW);

void camera_init_matrix();

void camera_update_ortho_projection_matrix();

#endif //COLLISION_SIMULATION_CP1_CAMERA_H